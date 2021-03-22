/*
 * module-main.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include "module-main.h"
#include "pcie.h"
#include "proc.h"
#include "file.h"
#include "device.h"
#include "debug.h"
#include "../userspace/constants.h"

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bernhard Lang");
MODULE_DESCRIPTION("line scan camera by Entwicklungsbuero Gerhard Stresing");

int major = 0;
struct class *lscpcie_class = 0;

/* keep track of information what has been initialised so far */
static int module_status = 0;

/* module parameters */
int num_pixels[MAX_BOARDS]    = { -1, -1, -1, -1, -1 };
int num_cameras[MAX_BOARDS]   = { -1, -1, -1, -1, -1 };
int dma_num_scans[MAX_BOARDS] = { -1, -1, -1, -1, -1 };

static int n_num_pixels    = MAX_BOARDS;
static int n_num_cameras   = MAX_BOARDS;
static int n_dma_num_scans = MAX_BOARDS;

module_param_array(num_pixels,    int, &n_num_pixels,    S_IRUGO);
module_param_array(num_cameras,   int, &n_num_cameras,   S_IRUGO);
module_param_array(dma_num_scans, int, &n_dma_num_scans, S_IRUGO);

/* Each driver instance has its own debug flags which can be set individually
   through a ioctl calls. The global debug flag can be set at module load time
   and will apply to each newly created instance. The debug flag D_MODULE is
   stored spearately in the global variable module_debug since it belongs to
   the module as a whole and not to a specific driver instance.
 */
int debug = 0;
int debug_module = 0;

module_param(debug, int, S_IRUGO);

#define PMDEBUG(...) do {                         \
  if (debug_module)                               \
    printk(KERN_WARNING "lscpcie: " __VA_ARGS__); \
} while (0)


/* initial values for newly created device instance */
const struct dev_struct lscpcie_device_init = {
  .status = D_MODULE,
  .physical_pci_base = 0,
  .mapped_pci_base = 0,
  .read_available = ATOMIC_INIT(1),
  .write_available = ATOMIC_INIT(1),
  .minor = -1,
  .dma_virtual_mem = 0,
  .proc_actual_register = 0,
  .proc_actual_register_long = 0,
  .control = 0,
  .proc_registers_entry = 0,
  .proc_registers_long_entry = 0,
  .proc_io_entry = 0
};

/* pci identifier */
static struct pci_device_id ids[] = {
  { PCI_DEVICE(VENDOR_ID, DEVICE_ID), },
  { 0, }
};

MODULE_DEVICE_TABLE(pci, ids);

/* pci initialisation and removal functions */
static struct pci_driver pci_driver = {
  .name = NAME,
  .id_table = ids,
  .probe = probe_lscpcie,
  .remove = remove_lscpcie
};


/* general infrastruture and pci registration, devices are initialised
   through probe_lscpcie upon detection by the pci core */
static int __init lscpcie_module_init(void) {
  dev_t dev;
  int i, result;

  if (debug & D_MODULE) debug_module = 1;

  printk(KERN_WARNING NAME" loading module.\n");

  lscpcie_class = class_create(THIS_MODULE, NAME);
  if (IS_ERR(lscpcie_class)) {
    printk(KERN_ERR "Error creating %s class \n", NAME);
    return PTR_ERR(lscpcie_class);
  }
  PMDEBUG("created device class\n");

  proc_init_module(); /* read number of devices, write adds software devices */

  for (i = 0; i < MAX_BOARDS; i++)
    lscpcie_devices[i] = lscpcie_device_init;

  if (major) { /* major device number given at module load */
    dev = MKDEV(major, 0);
    result = register_chrdev_region(dev, 1, NAME);
  } else { /* let the kernel pick the next available major */
    result = alloc_chrdev_region(&dev, 0, MAX_BOARDS, NAME);
    major = MAJOR(dev);
  }

  if (result < 0) {
    printk(KERN_WARNING NAME": can't obtain major device number %d\n", major);
    return result;
  }

  PMDEBUG("got major %d\n", major);

  if ((result = pci_register_driver(&pci_driver)) != 0) {
    printk(KERN_ERR NAME " registering pci device failed with %d", result);
    goto failed;
  }
  module_status |= MOD_PCI_REGISTERED;
  PMDEBUG("registered pci driver\n");

  printk(KERN_WARNING NAME" ready.\n");

  return 0;

 failed:
  clean_up_lscpcie_module();
  printk(KERN_ERR NAME": loading failed\n");

  return result;
}


static void __exit lscpcie_module_exit(void)
{
  clean_up_lscpcie_module();
  printk(NAME" unloaded\n");
}

/* release all kernel resources allocated at module init in reversed order */
void clean_up_lscpcie_module(void)
{
  int i;

  for (i = 0; i < MAX_BOARDS; i++)
    if (lscpcie_devices[i].minor >= 0) {
      struct dev_struct *dev = &lscpcie_devices[i];
      PMDEBUG("removing device %d\n", i);
      proc_clean_up(dev);
      dma_finish(dev);
      if (dev->mapped_pci_base)
        iounmap(dev->mapped_pci_base);
    }
  
  if (module_status & MOD_PCI_REGISTERED) {
    pci_unregister_driver(&pci_driver);
    module_status &= ~MOD_PCI_REGISTERED;
  }

  for (i = 0; i < MAX_BOARDS; i++)
    if (lscpcie_devices[i].minor >= 0)
      device_clean_up(&lscpcie_devices[i]);
  proc_clean_up_module();

  if (major) {
    PMDEBUG("unregistering major\n");
    unregister_chrdev_region(MKDEV(major, 0), 1);
  }

  if (lscpcie_class && !IS_ERR(lscpcie_class)) {
    PMDEBUG("destroying class\n");
    class_destroy(lscpcie_class);
  }

  PMDEBUG("done cleaning up module\n");
}

int get_device_number(const struct dev_struct *dev)
{
  int i;

  for (i = 0; i < MAX_BOARDS; i++)
    if (dev == lscpcie_devices + i)
      return i;

  return -1;
}

module_init(lscpcie_module_init);
module_exit(lscpcie_module_exit);
