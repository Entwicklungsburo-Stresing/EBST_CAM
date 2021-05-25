/* device.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include "device.h"
#include "module-main.h"
#include "file.h"
#include "mmap.h"
#include "ioctl.h"
#include "proc.h"
#include "debug.h"
#include <linux/fs.h>

/* device parameters */
struct dev_struct lscpcie_devices[MAX_BOARDS];

/* file operations on /dev/lscpcie<n> */
struct file_operations fops = {
	.owner = THIS_MODULE,
	.llseek = 0,
	.read = lscpcie_read,
	.write = lscpcie_write,
    .unlocked_ioctl = lscpcie_ioctl,
	.open = lscpcie_open,
	.release = lscpcie_release,
	.mmap = mmap_register_remap_mmap
};

/* create device node /dev/lscpcie<n> and initialise instance variables */
int device_init(struct dev_struct *dev, int minor)
{
	struct device *device;
	int result, dev_no;

	dev->init_debug_mode |= debug;

	PDEBUG(D_MODULE, "initialising device %d with major %d\n", minor,
	       major);
	dev->device = MKDEV(major, minor);

	PDEBUG(D_MODULE, "initialising cdev\n");
	cdev_init(&dev->cdev, &fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &fops;

	PDEBUG(D_MODULE, "adding cdev for %d,%d\n", MAJOR(dev->device),
	       MINOR(dev->device));
	result = cdev_add(&dev->cdev, dev->device, 1);
	if (result < 0)
		goto out_error;

	dev->init_status |= DEV_CDEV_CREATED;

	PDEBUG(D_MODULE, "creating device class %d\n", minor);
	device
	    =
	    device_create(lscpcie_class, NULL, dev->device, NULL, "%s%d",
			  NAME, minor);
	if (IS_ERR(device)) {	//-->> no! do proper cleanup!! <<--
		printk(KERN_ERR "creation of device %s%d failed\n", NAME,
		       minor);
		result = PTR_ERR(device);
		goto out_error;
	}

	dev->init_status |= DEV_CLASS_CREATED;
	dev->minor = minor;

	/* semaphores for dma buffer reading (and writing) */
	init_waitqueue_head(&dev->writeq);
	init_waitqueue_head(&dev->readq);
	sema_init(&dev->write_sem, 1);
	sema_init(&dev->read_sem, 1);
	sema_init(&dev->size_sem, 1);

	/* semaphores for proce */
	init_waitqueue_head(&dev->proc_readq);
	sema_init(&dev->proc_read_sem, 1);

	/* get one page of ram for the control structure which is going to be
	   exported to user spcae */
	dev->control = (struct control_struct *) get_zeroed_page(GFP_KERNEL);
	if (!dev->control) {
		printk(KERN_ERR NAME
		       ": failed to allocate memory for control block");
		result = -ENOMEM;
		goto out_error;
	}

	dev_no = get_device_number(dev);
	if (dev_no < 0) {
		printk(KERN_ERR NAME
		       ": invalid device pointer in device_init\n");
		result = -ENODEV;
		goto out_error;
	}

	/* take initial values from module parameters where present */
	dev->control->debug_mode = dev->init_debug_mode;
	dev->control->status = dev->init_status;
	if (num_pixels[dev_no] > 0)
		dev->control->number_of_pixels = num_pixels[dev_no];
	else
		dev->control->number_of_pixels = DEFAULT_NUMBER_OF_PIXELS;

	if (num_cameras[dev_no] > 0)
		dev->control->number_of_cameras = num_cameras[dev_no];
	else
		dev->control->number_of_cameras =
		    DEFAULT_NUMBER_OF_CAMERAS;

	if (dma_num_scans[dev_no] > 0)
		dev->control->dma_num_scans = dma_num_scans[dev_no];
	else
		dev->control->dma_num_scans = DEFAULT_DMA_NUM_SCANS;

	return 0;

      out_error:
	device_clean_up(dev);
	return result;
}

void device_clean_up(struct dev_struct *dev)
{
	if (dev->control) {
		dev->init_debug_mode = dev->control->debug_mode;
		dev->init_status = dev->control->status;
		free_page((unsigned long) dev->control);
		dev->control = 0;
	}
	if (dev->init_status & DEV_CLASS_CREATED) {
		PDEBUG(D_MODULE, "destroying device class\n");
		device_destroy(lscpcie_class, dev->device);
	}
	if (dev->init_status & DEV_CDEV_CREATED) {
		PDEBUG(D_MODULE, "removing cdev\n");
		cdev_del(&dev->cdev);
	}
	PDEBUG(D_MODULE, "done cleaning up device %d\n", dev->minor);

	dev->minor = -1;
}

/* retrieve entry number of device in parameter table */
struct dev_struct *device_data(uint8_t devno)
{
	return (devno < MAX_BOARDS) && (lscpcie_devices[devno].minor >= 0)
	    ? &lscpcie_devices[devno] : NULL;
}

int device_test_status(struct dev_struct *dev, u16 bits)
{
	if (dev->control)
		return dev->control->status & bits;
	return dev->init_status & bits;
}

void device_set_status(struct dev_struct *dev, u16 mask, u16 bits)
{
	if (dev->control)
		dev->control->status = (dev->control->status & ~mask) | bits;
	else
		dev->init_status = (dev->control->status & ~mask) | bits;
}
