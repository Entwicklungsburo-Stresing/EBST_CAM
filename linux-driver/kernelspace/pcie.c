/*****************************************************************//**
 * @file		pcie.c
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/

#include "pcie.h"
#include "module-main.h"
#include "device.h"
#include "proc.h"
#include "debug.h"
#include <linux/version.h>

/* Workaround for bad completions sent by the card.

   The FPGA occasionally sends a bad completion for a 32-bit register read (seen on e.g. PCIEFLAGS,
   BAR2 + 0xC0). Depending on the root port this is logged as Malformed TLP or as Unexpected Completion.
   If the error is unmasked and fatal, AER or DPC resets the link and the running measurement
   is lost. Logged header: 4a000001 0x000004 000000xx 00000000 (CplD, 1 DW,
   completer = the card).

   Masking both errors on the root port above the card prevents the link reset.
   Only the root port the card is attached to is touched, and the original mask is restored when the driver
   is removed. Disable with the module parameter mask_root_port_errors=0. */

#define LSCPCIE_RP_MASKED_ERRORS (PCI_ERR_UNC_MALF_TLP | PCI_ERR_UNC_UNX_COMP)
static bool mask_root_port_errors = true;
module_param(mask_root_port_errors, bool, 0444);
MODULE_PARM_DESC(mask_root_port_errors,
		 "mask Malformed TLP and Unexpected Completion on the upstream root port (default 1)");

static void lscpcie_mask_root_port_errors(struct dev_struct *dev,
					struct pci_dev *pci_dev)
{
	struct pci_dev *rp;
	u16 aer;
	u32 val;

	if (!mask_root_port_errors)
		return;

	rp = pcie_find_root_port(pci_dev);
	if (!rp) {
		dev_warn(&pci_dev->dev, "no root port found, errors not masked\n");
		return;
	}
	aer = pci_find_ext_capability(rp, PCI_EXT_CAP_ID_ERR);
	if (!aer) {
		dev_warn(&pci_dev->dev, "root port %s has no AER, nothing to mask\n",
			 pci_name(rp));
		return;
	}

	pci_read_config_dword(rp, aer + PCI_ERR_UNCOR_MASK, &val);
	dev->root_port_old_uemask = val;
	pci_write_config_dword(rp, aer + PCI_ERR_UNCOR_MASK,
			       val | LSCPCIE_RP_MASKED_ERRORS);
	dev->root_port = pci_dev_get(rp);
	dev_info(&pci_dev->dev, "masked MalfTLP/UnxCmplt on root port %s (UEMsk 0x%08x -> 0x%08x)\n",
		 pci_name(rp), val, val | LSCPCIE_RP_MASKED_ERRORS);
}

static void lscpcie_restore_root_port_mask(struct dev_struct *dev)
{
	struct pci_dev *rp = dev->root_port;
	u16 aer;
	u32 status;

	if (!rp)
		return;

	aer = pci_find_ext_capability(rp, PCI_EXT_CAP_ID_ERR);
	if (aer) {
		pci_read_config_dword(rp, aer + PCI_ERR_UNCOR_STATUS, &status);
		status &= LSCPCIE_RP_MASKED_ERRORS;
		if (status) {
			dev_info(&rp->dev, "lscpcie: masked error recorded (UESta 0x%08x)\n", status);
			pci_write_config_dword(rp, aer + PCI_ERR_UNCOR_STATUS,
					       status);
		}
		pci_write_config_dword(rp, aer + PCI_ERR_UNCOR_MASK,
				       dev->root_port_old_uemask);
	}
	pci_dev_put(rp);
	dev->root_port = NULL;
}

int probe_lscpcie(struct pci_dev *pci_dev, const struct pci_device_id *id)
{
	struct dev_struct *dev = 0;
	int result, i;
	u16 word;

	if (debug_module)
		printk(KERN_WARNING NAME": enabling pci device\n");

	result = pci_enable_device(pci_dev);
	if (result < 0)
		return result;

	result = pci_request_regions(pci_dev, NAME);
	if (result) {
		dev_err(&pci_dev->dev, "failed to request PCI BARs\n");
		pci_disable_device(pci_dev);
		return result;
	}

	for (i = 0; i < MAX_BOARDS; i++)
		if (lscpcie_devices[i].minor < 0)
			break;

	if (i == MAX_BOARDS) {
		result = -ENOMEM;
		pci_set_drvdata(pci_dev, NULL);
		goto out_error;
	}

	dev = &lscpcie_devices[i];
	PDEBUG(D_PCI, "setting pci device data\n");
	pci_set_drvdata(pci_dev, dev);
	if ((result = device_init(dev, i)) < 0)
		goto out_error;

	device_set_status(dev, DEV_PCI_ENABLED, DEV_PCI_ENABLED);

	/* check for correct vendor / device and subsystem */
	result = pci_read_config_word(pci_dev, PCI_VENDOR_ID, &word);
	assert(!result, "couldn't read pci vendor id", goto out_error, -1);
	assert(word == VENDOR_ID, "wrong vendor id", goto out_error, -1);
	result = pci_read_config_word(pci_dev, PCI_DEVICE_ID, &word);
	assert(!result, "couldn't read pci device id", goto out_error, -1);
	assert(word == DEVICE_ID, "wrong device id", goto out_error, -1);
	result =
	    pci_read_config_word(pci_dev, PCI_SUBSYSTEM_VENDOR_ID, &word);
	assert(!result, "couldn't read pci subsystem vendor id",
	       goto out_error, -1);
	assert(word == SUBSYSTEM_VENDOR_ID, "wrong subsystem vendor id",
	       goto out_error, -1);
	result = pci_read_config_word(pci_dev, PCI_SUBSYSTEM_ID, &word);
	assert(!result, "couldn't read pci subsystem id", goto out_error,
	       -1);
	assert(word == SUBSYSTEM_DEVICE_ID, "wrong subsystem id",
	       goto out_error, -1);

	device_set_status(dev, DEV_HARDWARE_PRESENT, DEV_HARDWARE_PRESENT);
	dev->pci_dev = pci_dev;

	PDEBUG(D_INTERRUPT, "allocating interrupt vector\n");
	result = pci_alloc_irq_vectors(pci_dev, 1, 1, PCI_IRQ_MSI);
	if (result < 0) {
		printk(KERN_ERR NAME": couldn't allocate irq vector\n");
		goto out_error;
	}
	device_set_status(dev, DEV_IRQ_ALLOCATED, DEV_IRQ_ALLOCATED);
	result = pci_irq_vector(pci_dev, 0);
	if (result < 0) {
		printk(KERN_ERR NAME": couldn't get irq vector\n");
		goto out_error;
	}
	dev->irq_line = result;
	PDEBUG(D_INTERRUPT, "interrupt line is %d\n", dev->irq_line);

	PDEBUG(D_PCI, "obtaining pci ressource2\n");
	dev->physical_pci_base = pci_resource_start(pci_dev, 2);
	dev->control->io_size = pci_resource_len(pci_dev, 2);
	PDEBUG(D_PCI, "remapping pci ressource2\n");
	dev->dma_reg = pci_iomap(pci_dev, 2, 0);
	if (!dev->dma_reg) {
		printk(KERN_ERR NAME ": mapping of BAR 2 failed\n");
		result = -ENOMEM;
		goto out_error;
	}
	PDEBUG(D_PCI, "mapped address space 2 to %p\n", dev->dma_reg);
	dev->s0_reg = (struct s0_reg_struct*) (((u8*) dev->dma_reg) + 0x80);
	PDEBUG(D_PCI, "mapped s0 address space 2 to %p\n", dev->s0_reg);

	PDEBUG(D_PCI, "setting PCI device as bus master\n");
	pci_set_master(pci_dev);

	pci_save_state(pci_dev);	/* needed by slot_reset after a link error */

	lscpcie_mask_root_port_errors(dev, pci_dev);

	result = dma_init(dev);
	if (result < 0)
		goto out_error;

	proc_init(dev);

	PDEBUG(D_PCI,
	       "lscpcie board found, assigned minor device number %d\n",
	       dev->minor);

	return 0;

      out_error:
	printk(KERN_ERR NAME": registering pci device failed, error %d\n",
	       result);
	remove_lscpcie(pci_dev);
	pci_set_drvdata(pci_dev, NULL);
	return result;
}

void remove_lscpcie(struct pci_dev *pci_dev)
{
	struct dev_struct *dev = pci_get_drvdata(pci_dev);

	PDEBUG(D_PCI, "removing lscpcie\n");

	if (!dev) {
		pci_release_regions(pci_dev);
		pci_disable_device(pci_dev);
		return;
	}

	if (dev->s0_reg && dev->control && !dev->link_lost &&
	    pci_device_is_present(pci_dev))
		abort_measurement(dev);
	dma_end(dev);
	lscpcie_restore_root_port_mask(dev);

	if (device_test_status(dev, DEV_IRQ_ALLOCATED)) {
		pci_free_irq_vectors(pci_dev);
		PDEBUG(D_INTERRUPT, "interrupt vectors freed\n");
		device_set_status(dev, DEV_IRQ_ALLOCATED, 0);
	}
	if (dev->dma_reg) {
		pci_iounmap(pci_dev, dev->dma_reg);
		dev->dma_reg = NULL;
		dev->s0_reg = NULL;
	}

	PDEBUG(D_PCI, "disabling pci device\n");
	pci_clear_master(pci_dev);
	pci_release_regions(pci_dev);
	pci_disable_device(pci_dev);
	device_set_status(dev, DEV_HARDWARE_PRESENT | DEV_PCI_ENABLED, 0);
	device_clean_up(dev);
	PDEBUG(D_PCI, "done removing pci device\n");
}
