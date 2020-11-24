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

  if ((result = pci_enable_device(pci_dev)) < 0) return result;

  /* check for correct vendor / device and subsystem */
  result = pci_read_config_word(pci_dev, PCI_VENDOR_ID, &word);
  assert(!result, "couldn't read pci vendor id", -1);
  assert(word == VENDOR_ID, "wrong vendor id", -1);
  result = pci_read_config_word(pci_dev, PCI_DEVICE_ID, &word);
  assert(!result, "couldn't read pci device id", -1);
  assert(word == DEVICE_ID, "wrong device id", -1);
  result = pci_read_config_word(pci_dev, PCI_SUBSYSTEM_VENDOR_ID, &word);
  assert(!result, "couldn't read pci subsystem vendor id", -1);
  assert(word == SUBSYSTEM_VENDOR_ID, "wrong subsystem vendor id", -1);
  result = pci_read_config_word(pci_dev, PCI_SUBSYSTEM_ID, &word);
  assert(!result, "couldn't read pci subsystem id", -1);
  assert(word == SUBSYSTEM_DEVICE_ID, "wrong subsystem id", -1);

  for (i = 0; i < MAX_BOARDS; i++)
    if (lscpcie_devices[i].minor < 0) break;
  if (i == MAX_BOARDS) return -ENOMEM;
  dev = &lscpcie_devices[i];

  dev->status |= HARDWARE_PRESENT;
  dev->pci_dev = pci_dev;

  if ((result = device_init(dev, i)) < 0) {
    pci_disable_device(pci_dev);
    return result;
  }

  dev->physical_pci_base = pci_resource_start(pci_dev, 2);
  dev->control->io_size = pci_resource_len(pci_dev, 2);
  dev->mapped_pci_base
    = ioremap_nocache(dev->physical_pci_base, dev->control->io_size);
  assert(dev->mapped_pci_base != 0, "ioremap of address space failed", -1);
  printk(KERN_WARNING NAME": mapped address space 2 to %p\n",
         dev->mapped_pci_base);

  pci_set_drvdata(pci_dev, dev);
  proc_init(dev);

  PDEBUG(D_PCI, "lscpcie board found, assigned minor device number %d\n",
	 dev->minor);

  return 0;
}

void remove_lscpcie(struct pci_dev *pci_dev)
{
  struct dev_struct *dev = pci_get_drvdata(pci_dev);

  PDEBUG(D_PCI, "removing lscpcie\n");

  if (dev->mapped_pci_base) iounmap(dev->mapped_pci_base);

  pci_disable_device(pci_dev);
  device_clean_up(dev);

  dev->status &= ~HARDWARE_PRESENT;
}
