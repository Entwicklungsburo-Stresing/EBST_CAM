/* ioctl.c
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

#include <linux/fs.h>
#include <linux/uaccess.h>
#include "../userspace/constants.h"
#include "device.h"
#include "module-main.h"
#include "camera.h"
#include "file.h"
#include "ioctl.h"

long ioctl_lscpci(struct file *filp, unsigned int cmd, unsigned long arg) {
  int result = 0, val;
  u32 reg32[2];
  struct dev_struct *dev = filp->private_data;
  u32 bits, mask;

  PDEBUG(D_IOCTL, "ioctl called with cmd %d\n", cmd);

  if (_IOC_TYPE(cmd) != LSCPCI_MAGIC) return -ENOTTY;

  /* check read write */
  if (_IOC_DIR(cmd) & _IOC_READ
      & !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd)))
    return -EFAULT;
  if (_IOC_DIR(cmd) & _IOC_WRITE
      & !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)))
    return -EFAULT;

  PDEBUG(D_IOCTL, "switching to ioctl\n");

  switch (cmd) {
  case LSCPCI_IOCTL_GET_PIXELS:
    PDEBUG(D_IOCTL, "ioctl get pixels (%d)\n", dev->n_pixels);
    result = put_user(dev->n_pixels, (uint16_t __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCI_IOCTL_INIT:
    PDEBUG(D_IOCTL, "ioctl start\n");
    if (dev->status & (CAMERA_ON | CAMERA_ACQUIRING)) {
      PDEBUG(D_IOCTL, "device busy\n");
      return -EBUSY;
    }

    result = camera_init(dev, &dev->vars);
    if (result) {
      printk(KERN_WARNING NAME " preparing camera failed with result %d\n",
             result);
      camera_finish(dev); /* prepare failed */
      break;
    }

    result = prepare_dma(dev);
    if (result) {
      printk(KERN_WARNING NAME " preparing dma failed with result %d\n",
             result);
      camera_finish(dev); /* dma failed */
      break;
    }

    break;


  case LSCPCI_IOCTL_START:
  case LSCPCI_IOCTL_IDLE_RUN:
    PDEBUG(D_IOCTL, "ioctl start\n");
    if (dev->status & (CAMERA_ON | CAMERA_ACQUIRING)) {
      PDEBUG(D_IOCTL, "device busy\n");
      return -EBUSY;
    }
    result = camera_start(cmd==LSCPCI_IOCTL_START, dev);
    if (result) {
      printk(KERN_WARNING NAME " starting camera failed with result %d\n",
             result);
      camera_stop(dev); /* start failed */
    }
    break;

  case LSCPCI_IOCTL_STOP:
    PDEBUG(D_IOCTL, "ioctl stop\n");
    if (!dev->hardware_present) {
      dev->status &= ~CAMERA_ACQUIRING;
      result = 0;
      break;
    }
    camera_stop(dev);
    break;

  case LSCPCI_IOCTL_SET_REG_BYTE:
    PDEBUG(D_IOCTL, "ioctl setting register byte\n");
    if (!dev->hardware_present) return -ENODEV;
    PDEBUG(D_IOCTL, "setting register 0x%02lx to 0x%02lx\n",
           ((arg>>8) & 0xFF), arg & 0xFF);
    write_register(arg & 0xFF, (arg>>8) & 0xFF, dev);
    break;

  case LSCPCI_IOCTL_SET_REG_WORD:
    PDEBUG(D_IOCTL, "ioctl setting register word\n");
    if (!dev->hardware_present) return -ENODEV;
    PDEBUG(D_IOCTL, "setting register 0x%02lx to 0x%02lx\n",
           ((arg>>16) & 0xFFFF), arg & 0xFFFF);
    write_register_word(arg & 0xFFFF, (arg>>16) & 0xFFFF, dev);
    break;

  case LSCPCI_IOCTL_SET_REG_DWORD:
    PDEBUG(D_IOCTL, "ioctl setting register double word\n");
    if (!dev->hardware_present) return -ENODEV;
    if (copy_from_user(reg32, (u32 __user *) arg, 2*sizeof(u32)))
      return -EFAULT;
    PDEBUG(D_IOCTL, "setting register 0x%02x to 0x%02x\n", reg32[0], reg32[1]);
    write_register_dword(reg32[1], reg32[0], dev);
    break;

  case LSCPCI_IOCTL_SET_PLX_REG:
    PDEBUG(D_IOCTL, "ioctl setting plx register\n");
    if (!dev->hardware_present) return -ENODEV;
    if (copy_from_user(reg32, (u32 __user *) arg, 2*sizeof(u32)))
      return -EFAULT;
    PDEBUG(D_IOCTL, "setting plx register 0x%02x to 0x%02x\n", reg32[0],
           reg32[1]);
    write_plx(reg32[1], reg32[0], dev);
    break;

  case LSCPCI_IOCTL_HARDWARE_PRESENT:
    PDEBUG(D_IOCTL, "ioctl get hardware present\n");
    result = put_user(dev->hardware_present, (int __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCI_IOCTL_FIFO_OVERFLOW:
    PDEBUG(D_IOCTL, "ioctl get fifo overflow\n");
    val = dev->hardware_present ? read_fifo_overflow(dev) : 0;
    result = put_user(val, (int __user *) arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCI_IOCTL_CLEAR_FIFO:
    PDEBUG(D_IOCTL, "ioctl clear fifo\n");
    return camera_clear_fifo(dev);
    break;

  case LSCPCI_IOCTL_FREE_BYTES:
    PDEBUG(D_IOCTL, "ioctl get free bytes\n");
    result = put_user(buffer_free(dev), (u32 __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCI_IOCTL_BYTES_AVAILABLE:
    PDEBUG(D_IOCTL, "ioctl get bytes available\n");
    result = put_user(bytes_in_buffer(dev), (u32 __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCI_IOCTL_BUF_PTR:
    PDEBUG(D_IOCTL, "ioctl get buffer pointers\n");
    result = put_user(get_buffer_pointers(dev), (u64 __user *)arg);
    if (result) result = -EFAULT;
    break;
    
  case LSCPCI_IOCTL_DUMP_REG:
    PDEBUG(D_IOCTL, "ioctl dump reg\n");
    result = dump_registers(dev, (void __user *)arg);
    break;

  case LSCPCI_IOCTL_SET_DEBUG:
    result = get_user(val, (u32 __user *)arg);
    PDEBUG(D_IOCTL, "got debug mode data 0x%08x\n", val);
    if (result) result = -EFAULT;
    else {
      bits = val & 0xFFFF;
      mask = (val >> 16) & ~D_MODULE;
      PDEBUG(D_IOCTL, "applying debug mode 0x%04x with mask 0x%04x\n", bits,
             mask);
      dev->debug_mode = (dev->debug_mode & ~mask) | (bits & mask);
      if (mask & D_MODULE)
	set_debug_module(bits & D_MODULE);
      put_user(dev->debug_mode | get_debug_module(), (u32 __user *)arg);
    }
    break;

  default:
    printk(KERN_WARNING NAME": ioctl unknown\n");
    return -ENOTTY;
  }

  return result;
}
