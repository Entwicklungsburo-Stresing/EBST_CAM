/* file.c
 *
 * Copyright (C) 2010-2020 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/export.h>
#include <linux/uaccess.h>

#include "../userspace/constants.h"
#include "file.h"
#include "ioctl.h"
#include "module-main.h"
#include "plx9056.h"
#include "camera.h"

int major = 0, minor = -1;

struct file_operations fops = {
  .owner = THIS_MODULE,
  .llseek = 0,
  .read = read_camera,
  .write = write_camera,
  .unlocked_ioctl = ioctl_lscpci,
  .open = driver_open_camera,
  .release = release_camera
};

int driver_open_camera(struct inode *inode, struct file *filp) {
  /* only one open at a time */
  int minor = iminor(inode);
  struct dev_struct *dev = device_data(minor);

  if ((filp->f_mode & FMODE_WRITE)
      &&
      !atomic_dec_and_test(&dev->write_available)) {
    atomic_inc(&dev->write_available);

    printk(KERN_ERR NAME
           " not opening camera because no camera write available\n");

    return -EBUSY;
  }

  if ((filp->f_mode & FMODE_READ)
      &&
      !atomic_dec_and_test(&dev->read_available)) {
    atomic_inc(&dev->read_available);
    if (filp->f_mode & FMODE_WRITE) atomic_inc(&dev->write_available);

    printk(KERN_ERR NAME
           " not opening camera because no camera write available\n");

    return -EBUSY;
  }

  PDEBUG(D_START_STOP, "opening\n");

  if (dev->hardware_present) {
    /* abort any DMA pending from buggy drivers :-) */
    writel(DMA0_CLEAR_INTR | DMA0_ABORT,
           dev->pci_config + PLX9056_DMACSR0);
  }

  filp->private_data = dev;

  dev->dma.ptrs.read_pos = 0;
  dev->dma.ptrs.read_buf = 0;
  dev->dma.ptrs.write_pos = 0;
  dev->dma.ptrs.write_buf = 0;
  dev->read_count = 0;
  dev->write_count = 0;

  return 0;
}

int release_camera(struct inode *inode, struct file *filp) {
  int result, minor = iminor(inode);
  struct dev_struct *dev = device_data(minor);

  PDEBUG(D_START_STOP, "camera releasing\n");

  if (dev->status & CAMERA_ACQUIRING) {
    if (dev->hardware_present && ((result = camera_finish(dev))))
      return result;
    dev->status &= ~(CAMERA_ACQUIRING | CAMERA_STOP | CAMERA_DMA_ACTIVE);
  }

  if (dev->have_irq) {
    free_irq(dev->pci_dev->irq, dev);
    dev->have_irq = 0;
  }

  if (filp->f_mode & FMODE_READ) atomic_inc(&dev->read_available);
  if (filp->f_mode & FMODE_WRITE) atomic_inc(&dev->write_available);

  PDEBUG(D_START_STOP, "camera released\n");

  return 0;
}

ssize_t buffer_free(struct dev_struct *dev) {
  int free_buffers;
  ssize_t bytes;

  if (down_interruptible(&dev->size_sem)) return -ERESTARTSYS;

  PDEBUG(D_BUFSPACE, "bytes available: %d,%d -> %d,%d\n", dev->dma.ptrs.read_buf,
         dev->dma.ptrs.read_pos, dev->dma.ptrs.write_buf,
         dev->dma.ptrs.write_pos);

  free_buffers = dev->dma.ptrs.read_buf - dev->dma.ptrs.write_buf;
  if (!free_buffers) /* all but a single buffer are filled */
    bytes = dev->dma.n_buffers * dev->dma.buf_size
      - (dev->dma.ptrs.write_pos - dev->dma.ptrs.read_pos) - 1;
  else
    if (free_buffers > 0) /* free buffers in the middle */
      bytes
        = dev->dma.ptrs.read_pos + dev->dma.buf_size - dev->dma.ptrs.write_pos
        + (free_buffers - 1) * dev->dma.buf_size - 1;
    else /* free buffers wrap around end */
      bytes
        = dev->dma.ptrs.read_pos + dev->dma.buf_size - dev->dma.ptrs.write_pos
        + (dev->dma.n_buffers + free_buffers - 1) * dev->dma.buf_size - 1;

  up(&dev->size_sem);

  return bytes;
}

ssize_t bytes_in_buffer(struct dev_struct *dev) {
  int filled_buffers;
  ssize_t bytes;

  if (down_interruptible(&dev->size_sem)) return -ERESTARTSYS;

  filled_buffers = dev->dma.ptrs.write_buf - dev->dma.ptrs.read_buf;

  PDEBUG(D_BUFSPACE, "bytes available: %d,%d -> %d,%d\n", dev->dma.ptrs.read_buf,
         dev->dma.ptrs.read_pos, dev->dma.ptrs.write_buf,
         dev->dma.ptrs.write_pos);

  if (!filled_buffers) /* there's only a single buffer with stuff to be read */
    bytes = dev->dma.ptrs.write_pos - dev->dma.ptrs.read_pos;
  else
    if (filled_buffers > 0) /* filled buffers in the middle */
      bytes
        = dev->dma.buf_size - dev->dma.ptrs.read_pos + dev->dma.ptrs.write_pos
        + (filled_buffers - 1) * dev->dma.buf_size;
    else /* filled buffer wrap around end */
      bytes
        = dev->dma.buf_size - dev->dma.ptrs.read_pos + dev->dma.ptrs.write_pos
        + (dev->dma.n_buffers + filled_buffers - 1) * dev->dma.buf_size;

  up(&dev->size_sem);

  PDEBUG(D_BUFSPACE, "%lu bytes available\n", bytes);

  return bytes;
}

ssize_t write_camera(struct file *filp, const char __user *buf, size_t len,
                     loff_t *off) {
  size_t bytes_to_copy = len, chunk_size, copied_bytes = 0;
  ssize_t free_bytes;
  struct dev_struct *dev = filp->private_data;
 
  if (dev->hardware_present) {
    PDEBUG(D_READOUT, "have camera, writing nothing\n");
    return -EBUSY;
  }

  if (down_interruptible(&dev->write_sem)) return -ERESTARTSYS;

  PDEBUG(D_READOUT, "asked for writing %lu bytes, write pos is %d\n", len,
         dev->dma.ptrs.write_pos);
  
  while (bytes_to_copy) {
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

    /* do not write beyond end of actual buffer */
    chunk_size = dev->dma.buf_size - dev->dma.ptrs.write_pos;

    if (chunk_size > free_bytes) chunk_size = free_bytes;
    if (chunk_size > bytes_to_copy) chunk_size = bytes_to_copy;

    PDEBUG(D_READOUT, "copying %lu bytes to (%d,%d)\n", chunk_size,
           dev->dma.ptrs.write_buf, dev->dma.ptrs.write_pos);

    if (copy_from_user(dev->dma.buffers[dev->dma.ptrs.write_buf]
                       + dev->dma.ptrs.write_pos,
                       buf + copied_bytes, chunk_size)) {
      printk(KERN_ERR NAME " \"%s\" EFAULT\n", current->comm);
      up(&dev->write_sem);
      return -EFAULT;
    }

    PDEBUG(D_READOUT, "writing: preparing next chunk\n");

    bytes_to_copy -= chunk_size;
    copied_bytes += chunk_size;
    if ((dev->dma.ptrs.write_pos += chunk_size) == dev->dma.buf_size) {
      dev->dma.ptrs.write_pos = 0;
      dev->dma.ptrs.write_buf
        = (dev->dma.ptrs.write_buf + 1) % dev->dma.n_buffers;
    }

    wake_up_interruptible(&dev->readq);

    PDEBUG(D_READOUT, "writing: looping over\n");
  }

  dev->write_count += copied_bytes;

  PDEBUG(D_READOUT, "wrote %lu bytes to camera buffer (%lu,%lu)\n",
         copied_bytes, dev->read_count, dev->write_count);

  up(&dev->write_sem);

  return copied_bytes;
}

ssize_t read_camera(struct file *filp, char __user *buf, size_t len,
                    loff_t *off) {
  struct dev_struct *dev = filp->private_data;
  size_t bytes_to_copy = len, copied_bytes = 0, chunk_size;
  ssize_t available_bytes;

  if (down_interruptible(&dev->read_sem)) return -ERESTARTSYS;

  PDEBUG(D_READOUT, "asked for reading %lu bytes (r: %d,%d w: %d,%d)\n", len,
         dev->dma.ptrs.read_buf, dev->dma.ptrs.read_pos, dev->dma.ptrs.write_buf,
         dev->dma.ptrs.write_pos);

  while (bytes_to_copy) {
    /* wait for bytes in ringbuffer */
    PDEBUG(D_READOUT, "reading: waiting for %lu data bytes\n", bytes_to_copy);
    while (!(available_bytes = bytes_in_buffer(dev))) {
      if (filp->f_flags & O_NONBLOCK) {
        up(&dev->read_sem);
        return -EAGAIN;
      }
      PDEBUG(D_READOUT, "\"%s\" reading: going to sleep\n", current->comm);
      if (wait_event_interruptible(dev->readq,
                                   bytes_in_buffer(dev)
                                   || !(dev->status & CAMERA_ACQUIRING))) {
        up(&dev->read_sem);
        return -ERESTARTSYS;
      }
      PDEBUG(D_READOUT, "\"%s\" reading: woke up\n", current->comm);
      if (!(dev->status & CAMERA_ACQUIRING)) break;
    }

    if (available_bytes < 0) { /* error */
      up(&dev->read_sem);
      return available_bytes;
    }

    PDEBUG(D_READOUT, "reading: have data, copied %ld, left %ld\n",
           copied_bytes, bytes_to_copy);

    chunk_size = dev->dma.buf_size - dev->dma.ptrs.read_pos;
    if (chunk_size > available_bytes) chunk_size = available_bytes;
    if (chunk_size > bytes_to_copy) chunk_size = bytes_to_copy;

    PDEBUG(D_READOUT, "copying %lu bytes from (%d,%d %d,%d)\n", chunk_size,
           dev->dma.ptrs.read_buf, dev->dma.ptrs.read_pos,
           dev->dma.ptrs.write_buf, dev->dma.ptrs.read_pos);

    if (dev->hardware_present) {
      int i;
      u8 temp, *p1, *p2;
      PDEBUG(D_READOUT, "reading: swapping DMA data\n");
      p1 = dev->dma.buffers[dev->dma.ptrs.read_buf];
      p2 = p1 + 1;
      for (i = 0; i < dev->n_pixels * 2; i++, p1 += 2, p2 += 2) {
	temp = *p1;
	*p1 = *p2;
	*p2 = temp;
      }
    }

    PDEBUG(D_READOUT, "reading: copying DMA data\n");
    if (copy_to_user(&buf[copied_bytes],
                     dev->dma.buffers[dev->dma.ptrs.read_buf]
                     + dev->dma.ptrs.read_pos,
                     chunk_size)) {
      up(&dev->read_sem);
      printk(KERN_ERR NAME " \"%s\" EFAULT\n", current->comm);
      return -EFAULT;
    }
 
    dev->dma.ptrs.read_pos += chunk_size;
    if (dev->dma.ptrs.read_pos == dev->dma.buf_size) {
      dev->dma.ptrs.read_pos = 0;
      dev->dma.ptrs.read_buf = (dev->dma.ptrs.read_buf + 1) % dev->dma.n_buffers;
    }

    wake_up_interruptible(&dev->writeq);

    bytes_to_copy -= chunk_size;
    copied_bytes += chunk_size;
    PDEBUG(D_READOUT, "reading: next data chunk\n");
  }

  PDEBUG(D_READOUT,
         "%lu bytes read from camera buffer, waking up writing tasks\n",
         copied_bytes);

  dev->read_count += copied_bytes;

  PDEBUG(D_READOUT,
         "done reading, pointers %d,%d %d,%d (having still %lu, %lu, %lu)\n",
         dev->dma.ptrs.read_buf, dev->dma.ptrs.read_pos, dev->dma.ptrs.write_buf,
         dev->dma.ptrs.write_pos, bytes_in_buffer(dev), dev->read_count,
         dev->write_count);

  up(&dev->read_sem);

  return copied_bytes;
}

u64 get_buffer_pointers(struct dev_struct *dev) {
  u64 data = 0;

  memcpy(&data, &dev->dma.ptrs, sizeof(buffer_ptr_t));
  return data;
}
