/*
 * pcie.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "pcie.h"
#include "module-main.h"
#include "device.h"
#include "proc.h"
#include "debug.h"
#include "../userspace/constants.h"


int probe_lscpcie(struct pci_dev *pci_dev, const struct pci_device_id *id)
{
  struct dev_struct *dev;
  int result, i;
  u16 word;

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
  pci_set_drvdata(pci_dev, dev);
  if ((result = device_init(dev, i)) < 0)
    goto out_error;

  dev->status |= DEV_PCI_ENABLED;

  /* check for correct vendor / device and subsystem */
  result = pci_read_config_word(pci_dev, PCI_VENDOR_ID, &word);
  assert(!result, "couldn't read pci vendor id", goto out_error, -1);
  assert(word == VENDOR_ID, "wrong vendor id", goto out_error, -1);
  result = pci_read_config_word(pci_dev, PCI_DEVICE_ID, &word);
  assert(!result, "couldn't read pci device id", goto out_error, -1);
  assert(word == DEVICE_ID, "wrong device id", goto out_error, -1);
  result = pci_read_config_word(pci_dev, PCI_SUBSYSTEM_VENDOR_ID, &word);
  assert(!result, "couldn't read pci subsystem vendor id", goto out_error, -1);
  assert(word == SUBSYSTEM_VENDOR_ID, "wrong subsystem vendor id",
	 goto out_error, -1);
  result = pci_read_config_word(pci_dev, PCI_SUBSYSTEM_ID, &word);
  assert(!result, "couldn't read pci subsystem id", goto out_error, -1);
  assert(word == SUBSYSTEM_DEVICE_ID, "wrong subsystem id", goto out_error, -1);

  dev->status |= DEV_HARDWARE_PRESENT;
  dev->pci_dev = pci_dev;

  PDEBUG(D_INTERRUPT, "allocating interrupt vector\n");
  result = pci_alloc_irq_vectors(pci_dev, 1, 1, PCI_IRQ_MSI);
  if (result < 0)
    goto out_error;
  dev->irq_line = pci_dev->irq;
  PDEBUG(D_INTERRUPT, "interrupt line is %d\n", dev->irq_line);
  dev->status |= DEV_IRQ_ALLOCATED;

  dev->physical_pci_base = pci_resource_start(pci_dev, 2);
  dev->control->io_size = pci_resource_len(pci_dev, 2);
  dev->mapped_pci_base
    = ioremap_nocache(dev->physical_pci_base, dev->control->io_size);
  assert(dev->mapped_pci_base != 0, "ioremap of address space failed",
	 goto out_error, -1);
  PDEBUG(D_PCI, "mapped address space 2 to %p\n", dev->mapped_pci_base);

  pci_set_master(pci_dev);

  result = dma_init(dev);
  if (result < 0)
    goto out_error;

  proc_init(dev);

  PDEBUG(D_PCI, "lscpcie board found, assigned minor device number %d\n",
	 dev->minor);

  return 0;

 out_error:
  remove_lscpcie(pci_dev);
  return result;
}

void remove_lscpcie(struct pci_dev *pci_dev)
{
  struct dev_struct *dev = pci_get_drvdata(pci_dev);

  PDEBUG(D_PCI, "removing lscpcie\n");

  if (dev) {
    proc_clean_up(dev);
    dma_finish(dev);
    device_clean_up(dev);
    if (dev->mapped_pci_base)
      iounmap(dev->mapped_pci_base);
    if (dev->status &= DEV_IRQ_ALLOCATED) {
      PDEBUG(D_PCI, "freeing interrupt vector\n");
      pci_free_irq_vectors(pci_dev);
    }
    dev->status &= ~DEV_IRQ_ALLOCATED;
  }

  pci_disable_device(pci_dev);
  dev->status &= ~(DEV_HARDWARE_PRESENT | DEV_PCI_ENABLED);
}
