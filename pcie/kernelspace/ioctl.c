/*
 * ioctl.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "ioctl.h"
#include "module-main.h"
#include "registers.h"
#include "../userspace/constants.h"
#include "device.h"
#include "file.h"
#include "debug.h"
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/pci.h>


#define ADDRESS(arg) (dev->pci_base + ((reg_info_t __user *)arg)->address)


long lscpcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  int result = 0, val, i;
  struct dev_struct *dev = filp->private_data;
  u32 bits, mask;
  u64 ptrs;
  u32 val32;
  reg_info_t reg_info;

  PDEBUG(D_IOCTL, "ioctl called with cmd %d\n", cmd);

  if (_IOC_TYPE(cmd) != LSCPCIE_MAGIC) return -ENOTTY;

  /* check read write */
  if (_IOC_DIR(cmd) & _IOC_READ
      & !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd)))
    return -EFAULT;

  if (_IOC_DIR(cmd) & _IOC_WRITE
      & !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)))
    return -EFAULT;

  switch (cmd) {
  case LSCPCIE_IOCTL_START:
  case LSCPCIE_IOCTL_IDLE_RUN:
  case LSCPCIE_IOCTL_STOP:
    return -ENOTTY;

  case LSCPCIE_IOCTL_HARDWARE_PRESENT:
    PDEBUG(D_IOCTL, "ioctl get hardware present\n");
    val = dev->status & HARDWARE_PRESENT ? 1 : 0;
    result = put_user(val, (int __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_FREE_BYTES:
    val = buffer_free(dev);
    PDEBUG(D_IOCTL, "ioctl get free bytes (%d)\n", val);
    result = put_user(val, (u32 __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_BYTES_AVAILABLE:
    val = bytes_in_buffer(dev);
    PDEBUG(D_IOCTL, "ioctl get bytes available (%d)\n", val);
    result = put_user(val, (u32 __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_FIFO_OVERFLOW:
    PDEBUG(D_IOCTL, "ioctl get fifo overflow\n");
    val = dev->status & FIFO_OVERFLOW ? 1 : 0;
    result = put_user(val, (int __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_CLEAR_FIFO:
    PDEBUG(D_IOCTL, "ioctl clear fifo\n");
    dev->status &= ~FIFO_OVERFLOW;
#warning make sure that camera is not running
    if (down_interruptible(&dev->read_sem)) return -ERESTARTSYS;
    dev->control->read_pos = 0;
    dev->control->write_pos = 0;
    up(&dev->read_sem);
    break;

  case LSCPCIE_IOCTL_GET_REG8:
    PDEBUG(D_IOCTL, "ioctl get register 8\n");
    result = get_user(reg_info.address, &((reg_info_t __user*)arg)->address);
    val32 = readb(dev->mapped_pci_base + reg_info.address);
    result = put_user(val32, &((reg_info_t __user *)arg)->value);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_GET_REG16:
    PDEBUG(D_IOCTL, "ioctl get register 16\n");
    result = get_user(reg_info.address, &((reg_info_t __user*)arg)->address);
    val32 = readw(dev->mapped_pci_base + reg_info.address);
    result = put_user(val32, &((reg_info_t __user *)arg)->value);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_GET_REG32:
    PDEBUG(D_IOCTL, "ioctl get register 32\n");
    result = get_user(reg_info.address, &((reg_info_t __user*)arg)->address);
    val32 = readl(dev->mapped_pci_base + reg_info.address);
    result = put_user(val32, &((reg_info_t __user *)arg)->value);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_SET_REG8:
    PDEBUG(D_IOCTL, "ioctl set register 8\n");
    result
      = copy_from_user(&reg_info, (reg_info_t __user*)arg, sizeof(reg_info_t));
    if (result) result = -EFAULT;
    else iowrite8(reg_info.value, dev->mapped_pci_base + reg_info.address);
    break;

  case LSCPCIE_IOCTL_SET_REG16:
    PDEBUG(D_IOCTL, "ioctl set register 16\n");
    result
      = copy_from_user(&reg_info, (reg_info_t __user*)arg, sizeof(reg_info_t));
    if (result) result = -EFAULT;
    else writew(reg_info.value, dev->mapped_pci_base + reg_info.address);
    break;

  case LSCPCIE_IOCTL_SET_REG32:
    PDEBUG(D_IOCTL, "ioctl set register 32\n");
    result
      = copy_from_user(&reg_info, (reg_info_t __user*)arg, sizeof(reg_info_t));
    if (result) result = -EFAULT;
    else writel(reg_info.value, dev->mapped_pci_base + reg_info.address);
    break;

  case LSCPCIE_IOCTL_GET_CONF:
    PDEBUG(D_IOCTL, "ioctl get config register\n");
    result = get_user(reg_info.address, &((reg_info_t __user*)arg)->address);
    if (result) result = -EFAULT;
    result = pci_read_config_dword(dev->pci_dev, reg_info.address, &val32);
    if (result < 0) return result;
    result = put_user(val32, &((reg_info_t __user *)arg)->value);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_SET_CONF:
    PDEBUG(D_IOCTL, "ioctl set register 8\n");
    result
      = copy_from_user(&reg_info, (reg_info_t __user*)arg, sizeof(reg_info_t));
    if (result) result = -EFAULT;
    result
      = pci_write_config_dword(dev->pci_dev, reg_info.address, reg_info.value);
    break;

  case LSCPCIE_IOCTL_BUFFER_POINTERS:
    PDEBUG(D_IOCTL, "ioctl get buffer pointers\n");
    ptrs
      = ((u64)dev->control->read_pos) | (((u64)dev->control->write_pos) << 32);
    result = put_user(ptrs, (u64 __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_NUM_BOARDS:
    PDEBUG(D_IOCTL, "ioctl get number of boards\n");
    val = 0;
    for (i = 0; i < MAX_BOARDS; i++)
      if (lscpcie_devices[i].minor >= 0)
        val++;
    result = put_user(val, (u32 __user *)arg);
    if (result) result = -EFAULT;
    break;

  case LSCPCIE_IOCTL_SET_DEBUG:
    PDEBUG(D_IOCTL, "got debug mode data 0x%08lx\n", arg);
    if (result) result = -EFAULT;
    else {
      bits = arg & DEBUG_BITS;
      mask = arg >> DEBUG_MASK_SHIFT;
      PDEBUG(D_IOCTL, "applying debug mode 0x%04x with mask 0x%04x\n", bits,
             mask);
      if (mask & ~D_MODULE) {
        dev->debug_mode = (dev->debug_mode & ~mask) | (bits & mask);
        PDEBUG(D_IOCTL, "device debug mode set to 0x%08x\n", dev->debug_mode);
      }
      if (mask & D_MODULE) {
        debug_module = bits & D_MODULE ? 1 : 0;
        PDEBUG(D_IOCTL, "module debug set to %d\n", debug_module);
      }
    }
    result = dev->debug_mode;
    break;

  default:
    PDEBUG(D_IOCTL, ": ioctl unknown\n");
    return -ENOTTY;
  }

  return result;
}
