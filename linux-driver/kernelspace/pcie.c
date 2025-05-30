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
	dev->irq_line = pci_dev->irq;
	PDEBUG(D_INTERRUPT, "interrupt line is %d\n", dev->irq_line);

	PDEBUG(D_PCI, "obtaining pci ressource2\n");
	dev->physical_pci_base = pci_resource_start(pci_dev, 2);
	dev->control->io_size = pci_resource_len(pci_dev, 2);
	PDEBUG(D_PCI, "remapping pci ressource2\n");
	dev->dma_reg =
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
		ioremap(dev->physical_pci_base,dev->control->io_size);
#else
		ioremap_nocache(dev->physical_pci_base, dev->control->io_size);
#endif
	assert(dev->dma_reg != 0,
	       "ioremap of address space failed", goto out_error, -1);
	PDEBUG(D_PCI, "mapped address space 2 to %p\n", dev->dma_reg);
	dev->s0_reg = (struct s0_reg_struct*) (((u8*) dev->dma_reg) + 0x80);
	PDEBUG(D_PCI, "mapped s0 address space 2 to %p\n", dev->s0_reg);

	PDEBUG(D_PCI, "setting PCI device as bus master\n");
	pci_set_master(pci_dev);

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
	return result;
}

void remove_lscpcie(struct pci_dev *pci_dev)
{
	struct dev_struct *dev = pci_get_drvdata(pci_dev);
	int dev_no = dev - lscpcie_devices;

	PDEBUG(D_PCI, "removing lscpcie\n");

	if (dev) {
		abort_measurement(dev);
		dma_end(dev);

		if (device_test_status(dev, DEV_IRQ_ALLOCATED)) {
			pci_free_irq_vectors(pci_dev);
			PDEBUG(D_INTERRUPT, "interrupt vectors freed\n");
			device_set_status(dev, DEV_IRQ_ALLOCATED, 0);
		}
		PDEBUG(D_MODULE, "removing device %d\n", dev_no);
		if (dev->dma_reg) {
			iounmap(dev->dma_reg);
			dev->dma_reg = 0;
			dev->s0_reg = 0;
		}
	}

	device_set_status(dev, DEV_HARDWARE_PRESENT | DEV_PCI_ENABLED, 0);
	PDEBUG(D_PCI, "disabling pci device\n");
	pci_clear_master(pci_dev);
	pci_disable_device(pci_dev);
	if (dev) {
		device_set_status(dev, DEV_HARDWARE_PRESENT|DEV_PCI_ENABLED, 0);
		device_clean_up(dev);
	}
	PDEBUG(D_PCI, "done removing pci device\n");
}
