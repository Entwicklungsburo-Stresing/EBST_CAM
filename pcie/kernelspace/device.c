/*
 * device.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include "device.h"
#include "module-main.h"
#include "file.h"
#include "ioctl.h"
#include "mmap.h"
#include "proc.h"
#include "debug.h"
#include <linux/fs.h>

/* device parameters */
struct dev_struct lscpcie_devices[MAX_BOARDS];

/* file operations on /dev/lscpcie<n> */
struct file_operations fops = {
  .owner = THIS_MODULE,
  .llseek         = 0,
  .read           = lscpcie_read,
  .write          = lscpcie_write,
  .unlocked_ioctl = lscpcie_ioctl,
  .open           = lscpcie_open,
  .release        = lscpcie_release,
  .mmap           = mmap_register_remap_mmap
};

/* create device node /dev/lscpcie<n> and initialise instance variables */
int device_init(struct dev_struct *dev, int minor)
{
  struct device *device;
  int result;
  int dev_no = get_device_number(dev);

  PDEBUG(D_BUFFERS, "initialising dma\n");

  if (dev_no < 0) {
    printk(KERN_ERR NAME": invalid device pointer in init_dma\n");
    return -ENODEV;
  }

  dev->debug_mode |= debug;// | D_MODULE | D_MMAP | D_IOCTL;

  PDEBUG(D_MODULE, "initialising device %d with major %d\n", minor, major);
  dev->device = MKDEV(major, minor);

  PDEBUG(D_MODULE, "initialising cdev\n");
  cdev_init(&dev->cdev, &fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &fops;

  PDEBUG(D_MODULE, "adding cdev for %d,%d\n", MAJOR(dev->device),
	 MINOR(dev->device));
  result = cdev_add(&dev->cdev, dev->device, 1);
  if (result < 0) device_clean_up(dev);
  dev->status |= DEVICE_CREATED;

  PDEBUG(D_MODULE, "creating device %d\n", minor);
  device
    = device_create(lscpcie_class, NULL, dev->device, NULL, "%s%d", NAME, minor);
  if (IS_ERR(device)) { //-->> no! do proper cleanup!! <<--
    printk(KERN_ERR "creation of device %s%d failed\n", NAME, minor);
    result = PTR_ERR(device);
    goto error;
  }

  dev->minor = minor;

  /* semaphores for dma buffer reading (and writing) */
  init_waitqueue_head(&dev->writeq);
  init_waitqueue_head(&dev->readq);
  sema_init(&dev->write_sem, 1);
  sema_init(&dev->read_sem, 1);
  sema_init(&dev->size_sem, 1);

  /* get one page of ram for the control structure which is going to be
     exported to user spcae */
  dev->control = (lscpcie_control_t *) get_zeroed_page(GFP_KERNEL);
  if (!dev->control) {
    printk(KERN_ERR NAME": failed to allocate memory for control block");
    return -ENOMEM;
  }

  /* take initial values from module parameters where present */
  if (num_pixels[dev_no] > 0)
    dev->control->number_of_pixels = num_pixels[dev_no];
  else
    dev->control->number_of_pixels = DEFAULT_NUMBER_OF_PIXELS;

  if (num_cameras[dev_no] > 0)
    dev->control->number_of_cameras = num_cameras[dev_no];
  else
    dev->control->number_of_cameras = DEFAULT_NUMBER_OF_CAMERAS;

  if (num_scans[dev_no] > 0) dev->control->number_of_scans = num_scans[dev_no];
  else dev->control->number_of_scans = DEFAULT_NUM_SCANS;

  if (num_blocks[dev_no] > 0) dev->control->number_of_blocks = num_blocks[dev_no];
  else dev->control->number_of_blocks = DEFAULT_NUM_BLOCKS;

  result = dma_init(dev);
  if (result < 0) goto error;

  return result;

 error:
  device_clean_up(dev);
  return result;
}


void device_clean_up(struct dev_struct *dev) {
  PDEBUG(D_MODULE, "cleaning up proc\n");
  proc_clean_up(dev);
  PDEBUG(D_MODULE, "cleaning up dma\n");
  dma_finish(dev);
  if (dev->status & DEVICE_CREATED) {
    PDEBUG(D_MODULE, "removing cdev\n");
    cdev_del(&dev->cdev);
    PDEBUG(D_MODULE, "destroying device\n");
    device_destroy(lscpcie_class, dev->device);
  }
  PDEBUG(D_MODULE, "done cleaning up device %d\n", dev->minor);

  dev->minor = -1;
}

/* retrieve entry number of device in parameter table */
struct dev_struct *device_data(uint8_t devno) {
  return (devno < MAX_BOARDS) && (lscpcie_devices[devno].minor >= 0)
    ? &lscpcie_devices[devno] : NULL;
}
