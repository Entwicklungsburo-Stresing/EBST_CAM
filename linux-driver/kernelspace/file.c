/* file.c
 *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include "file.h"
#include "device.h"
#include "registers.h"
#include "debug.h"
#include <linux/fs.h>
#include <linux/uaccess.h>

/* interface for reading from (and writing to) the dma buffer of
   /dev/lscpcie<n>
 */

int lscpcie_open(struct inode *inode, struct file *filp)
{
	/* only one open at a time */
	int minor = iminor(inode);
	struct dev_struct *dev = device_data(minor);

	PDEBUG(D_START_STOP, "opening lscpcie file\n");

	if ((filp->f_mode & FMODE_WRITE)
	    && !atomic_dec_and_test(&dev->write_available)) {
		atomic_inc(&dev->write_available);

		PDEBUG(D_START_STOP, "not opening because no write available\n");
		printk(KERN_ERR NAME
		       ": not opening because no write available\n");

		return -EBUSY;
	}

	if ((filp->f_mode & FMODE_READ)
	    && !atomic_dec_and_test(&dev->read_available)) {
		atomic_inc(&dev->read_available);
		if (filp->f_mode & FMODE_WRITE)
			atomic_inc(&dev->write_available);

		printk(KERN_ERR NAME
		       ": not opening because no read available\n");

		return -EBUSY;
	}

	/* initialise private data field used in file and ioctl callbacks */
	filp->private_data = dev;
	dev->control->write_pos = 0;
	dev->control->read_pos = 0;

	PDEBUG(D_START_STOP, "successfully opened lscpcie\n");

	return 0;
}

int lscpcie_release(struct inode *inode, struct file *filp)
{
	struct dev_struct *dev = filp->private_data;

	PDEBUG(D_START_STOP, "releasing camera\n");

	if (filp->f_mode & FMODE_READ)
		atomic_inc(&dev->read_available);
	if (filp->f_mode & FMODE_WRITE)
		atomic_inc(&dev->write_available);

	PDEBUG(D_START_STOP, "camera released\n");

	return 0;
}

int buffer_free(struct dev_struct *dev)
{
	long int bytes;

	if (down_interruptible(&dev->size_sem))
		return -ERESTARTSYS;

	PDEBUG(D_BUFFERS, "buffer pointers: %d,%d\n",
	       dev->control->read_pos, dev->control->write_pos);

	bytes = ((int) dev->control->read_pos) - ((int) dev->control->write_pos);
	PDEBUG(D_BUFFERS, "diff (free): %ld bytes\n", bytes);
	if (bytes <= 0)
		bytes += dev->control->used_dma_size;

	up(&dev->size_sem);

	PDEBUG(D_BUFFERS, "%ld free bytes\n", bytes);

	return bytes - 1;
}

int bytes_in_buffer(struct dev_struct *dev)
{
	long int bytes;

	if (down_interruptible(&dev->size_sem))
		return -ERESTARTSYS;

	PDEBUG(D_BUFFERS, "buffer pointers: %d,%d\n",
	       dev->control->read_pos, dev->control->write_pos);

	bytes = ((int) dev->control->write_pos) - ((int)dev->control->read_pos);
	PDEBUG(D_BUFFERS, "diff: %ld\n", bytes);
	if (bytes < 0)
		bytes += dev->control->used_dma_size;

	up(&dev->size_sem);

	PDEBUG(D_BUFFERS, "%ld bytes available (%d)\n", bytes,
	       dev->control->used_dma_size);

	return bytes;
}

ssize_t lscpcie_write(struct file *filp, const char __user * buf,
		      size_t len, loff_t * off)
{
	ssize_t free_bytes, copied_bytes;
	struct dev_struct *dev = filp->private_data;

	PDEBUG(D_READOUT, "asked for writing %lu bytes, write pos is %d\n",
	       len, dev->control->write_pos);

	if (device_test_status(dev, DEV_HARDWARE_PRESENT)) {
		PDEBUG(D_READOUT, "have camera, no writing will be performed\n");
		return -EBUSY;
	}

	if (down_interruptible(&dev->write_sem))
		return -ERESTARTSYS;

	/* correct mem size in case user space code should have tinkered with
           it */
	if (dev->control->used_dma_size > dev->control->dma_buf_size)
		dev->control->used_dma_size = dev->control->dma_buf_size;

	PDEBUG(D_READOUT, "waiting for free space in buffer\n");

	/* wait for bytes in ringbuffer */
	while (!(free_bytes = buffer_free(dev))) {
		if (filp->f_flags & O_NONBLOCK) {
			up(&dev->write_sem);
			return -EAGAIN;
		}
		PDEBUG(D_READOUT, "writing: going to sleep\n");
		if (wait_event_interruptible(dev->writeq, buffer_free(dev))) {
			up(&dev->write_sem);
			return -ERESTARTSYS;
		}
		PDEBUG(D_READOUT, "writing: woke up\n");
	}

	if (free_bytes < 0) {	/* error */
		up(&dev->write_sem);
		return free_bytes;
	}

	/* have now at least one free byte in one dma buffer */
	if (len > free_bytes)
		len = free_bytes;

	if (dev->control->write_pos + len > dev->control->used_dma_size) {
		size_t bytes_to_copy =
		    dev->control->used_dma_size - dev->control->write_pos;
		PDEBUG(D_READOUT, "copying %lu bytes to %d (a)\n",
		       bytes_to_copy, dev->control->write_pos);
		if (copy_from_user
		    (dev->dma_virtual_mem + dev->control->write_pos, buf,
		     bytes_to_copy)) {
			printk(KERN_ERR NAME " \"%s\" EFAULT\n",
			       current->comm);
			up(&dev->write_sem);
			return -EFAULT;
		}
		PDEBUG(D_READOUT, "wrote %lu bytes to camera buffer\n",
		       len);
		copied_bytes = bytes_to_copy;
		len -= bytes_to_copy;
		dev->control->write_pos
		    = (dev->control->write_pos + bytes_to_copy)
			% dev->control->used_dma_size;
	} else
		copied_bytes = 0;

	if (len) {
		PDEBUG(D_READOUT, "copying %lu bytes to %d (b)\n", len,
		       dev->control->write_pos);

		if (copy_from_user
		    (dev->dma_virtual_mem + dev->control->write_pos,
		     buf + copied_bytes, len)) {
			printk(KERN_ERR NAME " \"%s\" EFAULT\n",
			       current->comm);
			up(&dev->write_sem);
			return -EFAULT;
		}
		PDEBUG(D_READOUT, "wrote %lu bytes to camera buffer\n",
		       len);
		copied_bytes += len;
		dev->control->write_pos
		    = (dev->control->write_pos + len)
			% dev->control->used_dma_size;
	}

	up(&dev->write_sem);

	if (copied_bytes) {
		wake_up_interruptible(&dev->readq);
		wake_up_interruptible(&dev->proc_readq);
	}

	return copied_bytes;
}

ssize_t read_chunk(struct dev_struct *dev, size_t len, char __user * buf)
{
	PDEBUG(D_READOUT, "reading: copying DMA data\n");
	if (copy_to_user
	    (buf, dev->dma_virtual_mem + dev->control->read_pos, len)) {
		printk(KERN_ERR NAME " \"%s\" EFAULT\n", current->comm);
		return -EFAULT;
	}

	PDEBUG(D_READOUT,
	       "%lu bytes read from camera buffer, waking up writing tasks\n",
	       len);

	return len;
}

ssize_t lscpcie_read(struct file *filp, char __user * buf, size_t len,
		     loff_t * off)
{
	struct dev_struct *dev = filp->private_data;
	ssize_t available_bytes, copied_bytes;

	if (down_interruptible(&dev->read_sem))
		return -ERESTARTSYS;

	PDEBUG(D_READOUT, "asked for reading %lu bytes (r: %d w: %d)\n",
	       len, dev->control->read_pos, dev->control->write_pos);

	/* correct mem size in case user space code should have tinkered with
           it */
	if (dev->control->used_dma_size > dev->control->dma_buf_size)
		dev->control->used_dma_size = dev->control->dma_buf_size;

	/* wait for bytes in ringbuffer */
	PDEBUG(D_READOUT, "reading: waiting for %lu data bytes\n", len);
	while (!(available_bytes = bytes_in_buffer(dev))) {
		if (filp->f_flags & O_NONBLOCK) {
			up(&dev->read_sem);
			return -EAGAIN;
		}
		PDEBUG(D_READOUT, "\"%s\" reading: going to sleep\n",
		       current->comm);
		if (wait_event_interruptible(dev->readq, bytes_in_buffer(dev))) {
			up(&dev->read_sem);
			return -ERESTARTSYS;
		}
		PDEBUG(D_READOUT, "\"%s\" reading: woke up\n",
		       current->comm);
	}

	if (available_bytes < 0) {	/* error */
		up(&dev->read_sem);
		return available_bytes;
	}

	if (len > available_bytes)
		len = available_bytes;

	if (dev->control->read_pos + len > dev->control->used_dma_size) {
		ssize_t bytes_to_copy =
		    dev->control->used_dma_size - dev->control->read_pos;
		copied_bytes = read_chunk(dev, bytes_to_copy, buf);
		if (copied_bytes < 0) {
			up(&dev->read_sem);
			return copied_bytes;
		}
		len -= copied_bytes;
		dev->control->read_pos
		    = (dev->control->read_pos + copied_bytes)
			% dev->control->used_dma_size;
	} else
		copied_bytes = 0;

	if (len) {
		ssize_t n = read_chunk(dev, len, buf + copied_bytes);
		if (n < 0) {
			up(&dev->read_sem);
			return copied_bytes;
		}
		copied_bytes += n;
		dev->control->read_pos
		    = (dev->control->read_pos + n) % dev->control->used_dma_size;
	}

	wake_up_interruptible(&dev->writeq);

	up(&dev->read_sem);

	return copied_bytes;
}

unsigned int lscpcie_poll(struct file *filp, poll_table *wait) {
	struct dev_struct *dev = filp->private_data;
	unsigned int mask = 0;

	down(&dev->write_sem);
	down(&dev->read_sem);

	poll_wait(filp, &dev->readq, wait);
	poll_wait(filp, &dev->writeq, wait);

	if (dev->control->read_pos != dev->control->write_pos)
		mask |= POLLIN | POLLRDNORM;
	if (((dev->control->write_pos + 1) % dev->control->used_dma_size)
		!= dev->control->read_pos)
		mask |= POLLOUT | POLLWRNORM;

	up(&dev->read_sem);
	up(&dev->write_sem);

	return mask;
}
