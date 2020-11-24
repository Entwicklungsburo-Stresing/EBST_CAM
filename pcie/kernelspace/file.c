/*
 * file.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include "file.h"
#include "device.h"
#include "registers.h"
#include "debug.h"
#include "../userspace/constants.h"
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
      &&
      !atomic_dec_and_test(&dev->write_available)) {
    atomic_inc(&dev->write_available);

    printk(KERN_ERR NAME ": not opening because no write available\n");

    return -EBUSY;
  }

  if ((filp->f_mode & FMODE_READ)
      &&
      !atomic_dec_and_test(&dev->read_available)) {
    atomic_inc(&dev->read_available);
    if (filp->f_mode & FMODE_WRITE) atomic_inc(&dev->write_available);

    printk(KERN_ERR NAME": not opening because no read available\n");

    return -EBUSY;
  }

  /* initialise private data field used in file and ioctl callbacks */
  filp->private_data = dev;

  PDEBUG(D_START_STOP, "successfully opened lscpcie\n");

  return 0;
}


int lscpcie_release(struct inode *inode, struct file *filp)
{
  //int result, minor = iminor(inode);
  struct dev_struct *dev = filp->private_data;

  //PDEBUG(D_START_STOP, "camera releasing\n");

  /*
  if (dev->status & CAMERA_ACQUIRING) {
    if (dev->hardware_present && ((result = camera_finish(dev))))
      return result;
    dev->status &= ~(CAMERA_ACQUIRING | CAMERA_STOP | CAMERA_DMA_ACTIVE);
  }

  if (dev->have_irq) {
    free_irq(dev->pci_dev->irq, dev);
    dev->have_irq = 0;
  }
  */

  if (filp->f_mode & FMODE_READ) atomic_inc(&dev->read_available);
  if (filp->f_mode & FMODE_WRITE) atomic_inc(&dev->write_available);
  
  //PDEBUG(D_START_STOP, "camera released\n");

  return 0;
}


int buffer_free(struct dev_struct *dev)
{
  ssize_t bytes;

  if (down_interruptible(&dev->size_sem)) return -ERESTARTSYS;

  PDEBUG(D_BUFFERS, "buffer pointers: %d,%d\n", dev->control->read_pos,
         dev->control->write_pos);

  bytes = dev->control->write_pos - dev->control->read_pos;
  if (bytes <= 0) bytes += dev->control->buffer_size;

  up(&dev->size_sem);

  PDEBUG(D_BUFFERS, "%lu free bytes\n", bytes);

  return bytes - 1;
}


int bytes_in_buffer(struct dev_struct *dev)
{
  long int bytes;

  if (down_interruptible(&dev->size_sem)) return -ERESTARTSYS;

  PDEBUG(D_BUFFERS, "buffer pointers: %d,%d\n", dev->control->read_pos,
         dev->control->write_pos);

  bytes = dev->control->write_pos - (long int) dev->control->read_pos;
  if (bytes < 0) bytes += dev->control->buffer_size;

  up(&dev->size_sem);

  PDEBUG(D_BUFFERS, "%ld bytes available\n", bytes);

  return bytes;
}


ssize_t lscpcie_write(struct file *filp, const char __user *buf, size_t len,
                      loff_t *off)
{
  ssize_t free_bytes, copied_bytes;
  struct dev_struct *dev = filp->private_data;

  PDEBUG(D_READOUT, "asked for writing %lu bytes, write pos is %d\n", len,
         dev->control->write_pos);

  if (dev->status & HARDWARE_PRESENT) {
    PDEBUG(D_READOUT, "have camera, no writing will be performed\n");
    return -EBUSY;
  }

  if (down_interruptible(&dev->write_sem)) return -ERESTARTSYS;

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

  if (free_bytes < 0) { /* error */
    up(&dev->write_sem);
    return free_bytes;
  }

  /* have now at least one free byte in one dma buffer */
  if (len > free_bytes) len = free_bytes;

  if (dev->control->write_pos + len > dev->control->buffer_size) {
    size_t bytes_to_copy = dev->control->buffer_size - dev->control->write_pos;
    PDEBUG(D_READOUT, "copying %lu bytes to %d (a)\n", bytes_to_copy,
           dev->control->write_pos);
    if (copy_from_user(dev->dma_virtual_mem + dev->control->write_pos, buf,
                       bytes_to_copy)) {
      printk(KERN_ERR NAME " \"%s\" EFAULT\n", current->comm);
      up(&dev->write_sem);
      return -EFAULT;
    }
    PDEBUG(D_READOUT, "wrote %lu bytes to camera buffer\n", len);
    copied_bytes = bytes_to_copy;
    len -= bytes_to_copy;
    dev->control->write_pos
      = (dev->control->write_pos + bytes_to_copy) % dev->control->buffer_size;
  } else
    copied_bytes = 0;

  if (len) {
    PDEBUG(D_READOUT, "copying %lu bytes to %d (b)\n", len,
           dev->control->write_pos);

    if (copy_from_user(dev->dma_virtual_mem + dev->control->write_pos,
                       buf + copied_bytes, len)) {
      printk(KERN_ERR NAME " \"%s\" EFAULT\n", current->comm);
      up(&dev->write_sem);
      return -EFAULT;
    }
    PDEBUG(D_READOUT, "wrote %lu bytes to camera buffer\n", len);
    copied_bytes += len;
    dev->control->write_pos
      = (dev->control->write_pos + len) % dev->control->buffer_size;
  }

  up(&dev->write_sem);

  return copied_bytes;
}


ssize_t read_chunk(struct dev_struct *dev, size_t len, char __user *buf)
{
  PDEBUG(D_READOUT, "reading: copying DMA data\n");
  if (copy_to_user(buf, dev->dma_virtual_mem + dev->control->read_pos, len)) {
    printk(KERN_ERR NAME " \"%s\" EFAULT\n", current->comm);
    return -EFAULT;
  }
 
  PDEBUG(D_READOUT,
         "%lu bytes read from camera buffer, waking up writing tasks\n", len);

  return len;
}


ssize_t lscpcie_read(struct file *filp, char __user *buf, size_t len,
                     loff_t *off)
{
  struct dev_struct *dev = filp->private_data;
  ssize_t available_bytes, copied_bytes;

  if (down_interruptible(&dev->read_sem)) return -ERESTARTSYS;

  PDEBUG(D_READOUT, "asked for reading %lu bytes (r: %d w: %d)\n", len,
         dev->control->read_pos, dev->control->write_pos);

  /* wait for bytes in ringbuffer */
  PDEBUG(D_READOUT, "reading: waiting for %lu data bytes\n", len);
  while (!(available_bytes = bytes_in_buffer(dev))) {
    if (filp->f_flags & O_NONBLOCK) {
      up(&dev->read_sem);
      return -EAGAIN;
    }
    PDEBUG(D_READOUT, "\"%s\" reading: going to sleep\n", current->comm);
    if (wait_event_interruptible(dev->readq,
                                 bytes_in_buffer(dev))) {
      //|| !(dev->status & CAMERA_ACQUIRING))) {
      up(&dev->read_sem);
      return -ERESTARTSYS;
    }
    PDEBUG(D_READOUT, "\"%s\" reading: woke up\n", current->comm);
    //if (!(dev->status & CAMERA_ACQUIRING)) break;
  }

  if (available_bytes < 0) { /* error */
    up(&dev->read_sem);
    return available_bytes;
  }

  if (len > available_bytes) len = available_bytes;

  if (dev->control->read_pos + len > dev->control->buffer_size) {
    ssize_t bytes_to_copy = dev->control->buffer_size - dev->control->read_pos;
    copied_bytes = read_chunk(dev, bytes_to_copy, buf);
    if (copied_bytes < 0) {
      up(&dev->read_sem);
      return copied_bytes;
    }
    len -= copied_bytes;
    dev->control->read_pos += copied_bytes;
    if (dev->control->read_pos == dev->control->buffer_size)
      dev->control->read_pos = 0;
  } else
    copied_bytes = 0;

  if (len) {
    ssize_t n = read_chunk(dev, len, buf + copied_bytes);
    if (n < 0) {
      up(&dev->read_sem);
      return copied_bytes;
    }
    copied_bytes += len;
    dev->control->read_pos += copied_bytes;
    if (dev->control->read_pos == dev->control->buffer_size)
      dev->control->read_pos = 0;
  }

  wake_up_interruptible(&dev->writeq);

  up(&dev->read_sem);

  return copied_bytes;
}
