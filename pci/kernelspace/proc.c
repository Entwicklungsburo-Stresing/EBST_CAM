/* proc.c
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
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

#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/fs.h>
#include "../userspace/constants.h"
#include "device.h"
#include "camera.h"
#include "register-names.h"
#include "proc.h"

#define ADD_LINE(fmt, where, param) do {                              \
   if (len < count - 80) len += sprintf(buf + len, fmt, where->param); \
   } while (0)
#define ADD_VAL(fmt, args...) do {                                    \
   if (len < count - 80) len += sprintf(buf + len, fmt, ## args);     \
   } while (0)
#define ADD_PARAM(fmt, param) ADD_LINE(fmt, (&params), param)

#define ADD_STRING(string) do {                             \
   if (len < count - 80) len += sprintf(buf + len, string); \
   } while (0)

#define ADD_BOOL(strg, where, bit)             \
  do { if ((len < count - 80) && ((where & (bit)) == (bit)))   \
  len += sprintf(buf + len, strg); } while (0)

#define ADD_FLAG(strg, bit) ADD_BOOL(strg, params.flags, bit)
#define ADD_STATUS_BIT(strg, bit) ADD_BOOL(strg, dev->status, bit)
#define ADD_STATE(strg, state) do {                               \
    if ((len < count - 80) && (((dev->status & state) == state))) \
      len += sprintf(buf + len, strg);                  	  \
    } while (0)


#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
static inline void *PDE_DATA(const struct inode *inode) {
  return PDE(inode)->data;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
static inline struct inode *file_inode(struct file *f) {
  return f->f_dentry->d_inode;
}
#endif

ssize_t camera_read_proc(struct file *filp, char __user *buf, size_t count,
                         loff_t *offp) {
  int len = 0, result;
  struct lscpci_param params;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
  struct dev_struct *dev = PDE_DATA(file_inode(filp));
#else
  struct dev_struct *dev = PDE(filp->f_dentry->d_inode)->data;
#endif
  static int finished = 0;

  if (finished) {
    finished = 0;
    return 0;
  }
  finished = 1; /* should ideally check for real end! */

  result = get_params(dev, &params);

  if (result) {
    ADD_STRING("get params error\n");
    return len;
  }

  if (!dev->hardware_present)
    len += sprintf(buf + len, "debugging version without hardware\n");

  ADD_PARAM("ND wait staites: ... %d\n", nd_wait);
  ADD_PARAM("timer resolution: .. %d\n", timer_resolution);
  ADD_PARAM("pclk: .............. %d\n", pclk);
  ADD_PARAM("xck_delay: ......... %d\n", xck_delay);
  ADD_PARAM("vertical control: .. %d\n", vertical_count);
  ADD_PARAM("vertical frequency:  %d\n", vertical_frequency);
  if (params.delay_after_trigger >= 0)
    ADD_PARAM("delay after trigger: %d\n", delay_after_trigger);
  if (params.exposure_time >= 0)
    ADD_PARAM("exposure time: ..... %d\n", exposure_time);
  ADD_PARAM("trigger-in divider:  %d\n", trigger_in_divider);
  ADD_PARAM("trigger-out divider: %d\n", trigger_out_divider);
  ADD_PARAM("FIFO delay: ........ %d\n", fifo_delay);

  ADD_LINE("pixels: ............ %d\n", dev, n_pixels);

  ADD_VAL("interrupts: ........ %d\n", dev->irq_count);
  ADD_VAL("acquisitions: ...... %d\n", dev->acquisition_count);
  ADD_VAL("timer polls: ....... %d\n", dev->timer_count);
  ADD_VAL("read access: ....... %d\n", dev->read_available.counter);
  ADD_VAL("write access: ...... %d\n", dev->write_available.counter);
  ADD_VAL("read semaphore: .... %d\n", dev->write_sem.count);
  ADD_VAL("write semaphore: ... %d\n", dev->read_sem.count);

  ADD_FLAG("amplifier on\n", AMPLIFIER_ON);
  ADD_FLAG("trigger positive\n", TRIG_POS);
  ADD_FLAG("trigger on edge\n", TRIG_EDGE);
  ADD_FLAG("ND SYM\n", NDSYM);
  ADD_FLAG("double pulse\n", DOUBLE_PULSE);
  ADD_FLAG("divider low\n", DIVIDER_LOW);
  ADD_FLAG("divider high\n", DIVIDER_HIGH);
  ADD_FLAG("no PDA reset\n", NO_PDA_RESET);

  switch (params.out_select) {
  case out_xck:    ADD_STRING("out XCK\n"); break;
  case out_dat:    ADD_STRING("out DAT\n"); break;
  case out_ffxck:  ADD_STRING("out FFXCK\n"); break;
  case out_tin:    ADD_STRING("out TIN\n"); break;
  case out_ec:     ADD_STRING("out EC\n"); break;
  case out_to_reg: ADD_STRING("out TO REG\n"); break;
  }

  //ADD_STATUS_BIT("FIFO empty\n", FIFO_EMPTY);
  //ADD_STATUS_BIT("FIFO write active\n", FIFO_WRTIE_ACTIVE);
  ADD_STATUS_BIT("FIFO overflow\n", FIFO_OVERFLOW);

  ADD_STATE("acquiring\n", CAMERA_ACQUIRING);

  ADD_LINE("dma ready to read buffer is %d\n", (&dev->dma), ptrs.write_buf);
  ADD_LINE("dma read buffer is %d\n", (&dev->dma), ptrs.read_buf);
  ADD_LINE("dma read position is %d\n", (&dev->dma), ptrs.read_pos);
  ADD_LINE("dma write buffer is %d\n", (&dev->dma), ptrs.write_buf);
  ADD_LINE("dma write position is %d\n", (&dev->dma), ptrs.write_pos);
  ADD_LINE("debug is 0x%02x\n", dev, debug_mode);

  return len;
}

char buf[] = "0x0000'0000";

static const char *binary(unsigned char n) {
  int pos = 10, i, mask = 0x01;
  for (i = 0; i < 8; i++, mask <<= 1) {
    *(buf + pos--) = (n & mask) ? '1' : '0';
    if (i == 3) *(buf + pos--) = '\'';
  }
  return buf;
}

ssize_t camera_read_registers_proc(struct file *filp, char __user *buf,
                                   size_t count, loff_t *offp) {
  ssize_t len = 0;
  int val, i;
  static int finished = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
  struct dev_struct *dev = PDE_DATA(file_inode(filp));
#else
  struct dev_struct *dev = PDE(filp->f_dentry->d_inode)->data;
#endif

  if (finished) {
    finished = 0;
    return 0;
  }
  finished = 1; /* should ideally check for real end! */

  if (!dev->hardware_present) return -ENODEV;

  for (i = 4; i < 64; i++) {
    val = readb(dev->pci_camera + i);
    len += sprintf(buf + len, "0x%02x (%s): 0x%02x %s\n", i, reg_names[i-4],
                   val, binary(val));
    if (len > count - 80) break;
  }
  for (i = 4; i < NUM_PLX_REGISTERS; i += 4) {
    val = readl(dev->pci_config + i);
    len += sprintf(buf + len, "0x%02x: 0x%02x\n", i, val);
    if (len > count - 80) break;
  }

  return len;
}
