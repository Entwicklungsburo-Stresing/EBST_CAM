/* proc.c
 *
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include "proc.h"
#include "device.h"
#include "registers.h"
#include "debug.h"
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

/* This proc entry is created upon loading the module. It tells the number of
   devices and can be used to add a debugging device. */

static struct proc_dir_entry *proc_entry = NULL;

/* count the number of initialised devices and return it as a two-byte
   number, */

ssize_t lscpcie_read_proc(struct file *filp, char __user * buf,
			  size_t count, loff_t * offp)
{
	int i, count_boards = 0, count_debug = 0;
	char result[3];

	// need at least two bytes space in buffer
	if (count < 3)
		return -EAGAIN;

	if (*offp)
		return 0; // second call returns zero to indicate end of file

	for (i = 0; i < MAX_BOARDS; i++)
		if (lscpcie_devices[i].minor >= 0) {
			if (device_test_status(&lscpcie_devices[i],
						DEV_HARDWARE_PRESENT))
				count_boards++;
			else
				count_debug++;
		}

	result[0] = '0' + count_boards;
	result[1] = '0' + count_debug;
	result[2] = '\n';

	if (copy_to_user((char __user *) buf, result, 3))
		return -EFAULT;

	*offp += 3;

	return 3;
}

/* Add debug device. Two colon-separated numbers are expected, first the number
   of pixels, second the number of cameras. The input has to be terminated
   with a new-line character. */

ssize_t lscpcie_write_proc(struct file *filp, const char __user * buf,
			   size_t count, loff_t * offp)
{
	int i, slot, result;
	static int n_pixels = 0;
	static int n_cameras = -1;
	char c;

	for (i = 0; i < count; i++) {
		get_user(c, buf++);

		/* start with pixels */
		if (n_cameras < 0) {
			if ((c >= '0') && (c <= '9')) {
				n_pixels = n_pixels * 10 + (c - '0');
				continue;
			}
			if (c != ':')
				goto out_done;
			n_cameras = 0;
			continue;
		}

		/* now cameras */
		if ((c >= '0') && (c <= '9')) {
			n_cameras = n_cameras * 10 + (c - '0');
			continue;
		}

		/* it should be a line end once we get here */
		if (c != '\n')
			goto out_done;

		for (slot = 0; slot < MAX_BOARDS; slot++)
			if (lscpcie_devices[slot].minor < 0) {
				result = device_init(&lscpcie_devices[slot],
						slot);
				if (result < 0) {
					/* something unexpected happend, report
					   the error */
					i = result;
					break;
				}
				proc_init(&lscpcie_devices[slot]);
				break;
			}
		if (slot == MAX_BOARDS)
			i = -ENOMEM;	/* no free device left */
		goto out_done;
	}

	return i >= 0 ? i + 1 : i;

      out_done:
	n_pixels = 0;
	n_cameras = -1;

	return i >= 0 ? i + 1 : i;
}

struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_proc,
	.write = lscpcie_write_proc
};

void proc_init_module(void)
{
	if (debug_module)
		printk(KERN_WARNING NAME " creating module proc entry\n");

	proc_entry
	    =
	    proc_create_data(NAME, S_IRUGO | S_IWUGO, NULL, &proc_fops, NULL);
}

void proc_clean_up_module(void)
{
	if (debug_module)
		printk(KERN_WARNING
		       "lscpcie: removing module proc entry\n");
	if (proc_entry) {
		remove_proc_entry(NAME, NULL);
		proc_entry = NULL;
	}
}

/* This proc entry is used to block a 'reading' process until something appears
   in the dma buffer. It returns with the value of the interrupt counter.
   The buffer's content may then directly be read via memory mapping. */

ssize_t lscpcie_read_data_proc(struct file *filp, char __user * buf,
			       size_t count, loff_t * offp)
{
	struct dev_struct *dev = filp->private_data;
	static u32 irq_counter;
	int result, bytes_to_copy;

	if (down_interruptible(&dev->proc_read_sem))
		return -ERESTARTSYS;

	switch (*offp) {
	case 4: /* already all copied */
		result = 0;
		break;

	case 0: /* first call */
		while (dev->control->read_pos == dev->control->write_pos) {
			if (filp->f_flags & O_NONBLOCK) {
				result = -EAGAIN;
				break;
			}
			if (wait_event_interruptible(dev->proc_readq,
			   dev->control->read_pos != dev->control->write_pos)) {
				result = -ERESTARTSYS;
				break;
			}
		}

		irq_counter = dev->control->irq_count;
		/* go on with copying */

	default:
		bytes_to_copy = 4 - *offp;
		if (bytes_to_copy > count) bytes_to_copy = count;
		result = copy_to_user(buf, ((uint8_t*) &irq_counter) + *offp,
				count);
		if (result)
			result = -EFAULT;
		else
			result = bytes_to_copy;
	}

	up(&dev->proc_read_sem);

	return result;
}

struct file_operations proc_data_fops = {
	.owner = THIS_MODULE,
	.read = lscpcie_read_data_proc
};

/* init proc files per device instance */

void proc_init(struct dev_struct *dev)
{
	char name[32];

	PDEBUG(D_PROC, "creating main device proc entry");
	sprintf(name, "%s%d", NAME, dev->minor);
	dev->proc_data_entry =
	    proc_create_data(name, 0, NULL, &proc_data_fops, dev);
}

void proc_clean_up(struct dev_struct *dev)
{
	char name[32];

	PDEBUG(D_PROC, "removing proc entries\n");

	if (dev->proc_data_entry) {
		sprintf(name, "%s%d", NAME, dev->minor);
		remove_proc_entry(name, NULL);
		dev->proc_data_entry = 0;
	}
}
