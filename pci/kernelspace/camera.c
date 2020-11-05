/* camera.c
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

/* this file contains code related to the FPGA implementation */

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include "../userspace/constants.h"
#include "device.h"
#include "file.h"
#include "camera.h"

#define SET_RUN_STATE(run_state) \
  do dev->status = (dev->status & ~CCD_RUN_STATE_MASK) | run_state; while (0)

/* set camera registers and initialise DMA buffer pointers to zero */
int camera_init(struct dev_struct *dev, struct board_vars *vars) {
  if (dev->status & CAMERA_ACQUIRING) return -EBUSY;

  if (!dev->hardware_present) return 0;

  PDEBUG(D_START_STOP, "initialising camera\n");

  init_hardware(dev, vars);

  PDEBUG(D_START_STOP, "initialising done\n");

  dev->dma.ptrs.read_buf = 0;
  dev->dma.ptrs.read_pos = 0;
  dev->dma.ptrs.write_buf = 0;
  dev->dma.ptrs.write_pos = 0;
  dev->free_count = dev->dma.n_buffers;

  return 0;
}

/* start camera read-out */
int camera_start(int with_readout, struct dev_struct *dev) {
  int result;

  if (dev->status & CAMERA_ACQUIRING) return -EBUSY;

  if (!dev->hardware_present) {
    dev->status |= CAMERA_ACQUIRING;
    return 0;
  }

  if (with_readout && ((result = clear_fifo(dev)) != 0)) return result;

  if ((result = enable_trigger(dev)) != 0) return result;

  if (with_readout) {
    dev->acquisition_count = 0;
    dev->status
      = (dev->status & ~CAMERA_FIFO_OVERFLOW) | CAMERA_ON | CAMERA_ACQUIRING;
  } else
    dev->status |= CAMERA_ON;

  PDEBUG(D_START_STOP, "camera running\n");

  return 0;
}

/* stop camra trigger */
int camera_stop(struct dev_struct *dev) {
  if (!dev->hardware_present) {
    dev->status &= ~(CAMERA_ACQUIRING | CAMERA_STOP);
    return 0;
  }

  if (!(dev->status & CAMERA_ON)) return 0;

  PDEBUG(D_START_STOP,
         "signalling stop and wait the irq handler having acted upon\n");

  if (dev->status | CAMERA_ACQUIRING) {
    dev->status |= CAMERA_STOP;

    if (wait_event_interruptible(dev->readq, !(dev->status & CAMERA_STOP)))
      return -ERESTARTSYS;
  }

  disable_trigger(dev);

  dev->status &= ~(CAMERA_ON | CAMERA_ACQUIRING);

  return 0;
}

/* clean-up before unloadingthe driver */
int camera_finish(struct dev_struct *dev) {
  int result;

  if (!dev->hardware_present) return 0;

  if (dev->status & CAMERA_ACQUIRING) {
    if ((result = camera_stop(dev)))
      printk(KERN_WARNING NAME " stopping camera returned with error %d\n",
             result);
  }

  result = finish_dma(dev);

  PDEBUG(D_START_STOP, "finished camera\n");

  return result;
}

/* clear the camera's fifo register and reset dma buffer pointer */
int camera_clear_fifo(struct dev_struct *dev) {
  if (dev->status & CAMERA_ACQUIRING) {
    printk(KERN_ERR NAME " attempt to clear running camera\n");
    return -EBUSY;
  }

  PDEBUG(D_START_STOP, "clearing camera fifo\n");

  if (dev->hardware_present) clear_fifo(dev);

  // clear ring buffer structure

  if (down_interruptible(&dev->write_sem)) return -ERESTARTSYS;

  if (down_interruptible(&dev->read_sem)) {
    up(&dev->write_sem);
    return -ERESTARTSYS;
  }

  if (down_interruptible(&dev->size_sem)) {
    up(&dev->write_sem);
    up(&dev->read_sem);
    return -ERESTARTSYS;
  }

  dev->dma.ptrs.read_buf = 0;
  dev->dma.ptrs.read_pos = 0;
  dev->dma.ptrs.write_pos = 0;
  dev->dma.ptrs.write_buf = 0;

  up(&dev->write_sem);
  up(&dev->read_sem);
  up(&dev->size_sem);

  return 0;
}
