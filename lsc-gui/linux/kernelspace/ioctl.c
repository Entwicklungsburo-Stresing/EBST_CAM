/* ioctl.c
 *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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


long lscpcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int result = 0, val, i;
	struct dev_struct *dev = filp->private_data;
	reg_info_t reg_info;
	u32 val32;

	PDEBUG(D_IOCTL, "ioctl called with cmd %d\n", cmd);

	if (_IOC_TYPE(cmd) != LSCPCIE_MAGIC)
		return -ENOTTY;

	/* check read write */
	if (_IOC_DIR(cmd) & _IOC_READ
	    & !access_ok(VERIFY_WRITE, (void __user *) arg,
			 _IOC_SIZE(cmd)))
		return -EFAULT;

	if (_IOC_DIR(cmd) & _IOC_WRITE
	    & !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd)))
		return -EFAULT;

	switch (cmd) {
	case LSCPCIE_IOCTL_NUM_BOARDS:
		PDEBUG(D_IOCTL, "ioctl get number of boards\n");
		val = 0;
		for (i = 0; i < MAX_BOARDS; i++)
			if (lscpcie_devices[i].minor >= 0)
				val++;
		result = put_user(val, (u32 __user *) arg);
		if (result)
			result = -EFAULT;
		break;

       case LSCPCIE_IOCTL_GET_CONF:
	       PDEBUG(D_IOCTL, "ioctl get config register\n");
	       result =
		   get_user(reg_info.address,
			    &((reg_info_t __user *) arg)->address);
	       if (result)
		       result = -EFAULT;
	       result =
		   pci_read_config_dword(dev->pci_dev, reg_info.address,
					 &val32);
	       if (result < 0)
		       return result;
	       result =
		   put_user(val32, &((reg_info_t __user *) arg)->value);
	       if (result)
		       result = -EFAULT;
	       break;

       case LSCPCIE_IOCTL_SET_CONF:
	       PDEBUG(D_IOCTL, "ioctl set register 8\n");
	       result
		   =
		   copy_from_user(&reg_info, (reg_info_t __user *) arg,
				  sizeof(reg_info_t));
	       if (result)
		       result = -EFAULT;
	       result
		   =
		   pci_write_config_dword(dev->pci_dev, reg_info.address,
					  reg_info.value);
	       break;

		case LSCPCIE_IOCTL_GET_REG8:
			PDEBUG(D_IOCTL, "ioctl get register 8\n");
			result =
				get_user(reg_info.address,
					&((reg_info_t __user *) arg)->address);
			if (result)
				result = -EFAULT;
			val32 = readb(dev->dma_reg + reg_info.address);
			result =
				put_user(val32, &((reg_info_t __user *) arg)->value);
			if (result)
				result = -EFAULT;
			break;

		case LSCPCIE_IOCTL_GET_REG16:
			PDEBUG(D_IOCTL, "ioctl get register 16\n");
			result =
				get_user(reg_info.address,
					&((reg_info_t __user *) arg)->address);
			if (result)
				result = -EFAULT;
			val32 = readw(dev->dma_reg + reg_info.address);
			result =
				put_user(val32, &((reg_info_t __user *) arg)->value);
			if (result)
				result = -EFAULT;
			break;

		case LSCPCIE_IOCTL_GET_REG32:
			PDEBUG(D_IOCTL, "ioctl get register 32\n");
			result =
				get_user(reg_info.address,
					&((reg_info_t __user *) arg)->address);
			if (result)
				result = -EFAULT;
			val32 = readl(dev->dma_reg + reg_info.address);
			result =
				put_user(val32, &((reg_info_t __user *) arg)->value);
			if (result)
				result = -EFAULT;
			break;

		case LSCPCIE_IOCTL_SET_REG8:
			PDEBUG(D_IOCTL, "ioctl set register 8\n");
			result
				=
				copy_from_user(&reg_info, (reg_info_t __user *) arg,
					sizeof(reg_info_t));
			if (result)
				result = -EFAULT;
			else
				iowrite8(reg_info.value,
					dev->dma_reg + reg_info.address);
			PDEBUG(D_IOCTL, "at address 0x%p\n",
				dev->dma_reg + reg_info.address);
			break;

		case LSCPCIE_IOCTL_SET_REG16:
			PDEBUG(D_IOCTL, "ioctl set register 16\n");
			result
				=
				copy_from_user(&reg_info, (reg_info_t __user *) arg,
					sizeof(reg_info_t));
			if (result)
				result = -EFAULT;
			else
				writew(reg_info.value,
					dev->dma_reg + reg_info.address);
			break;

		case LSCPCIE_IOCTL_SET_REG32:
			PDEBUG(D_IOCTL, "ioctl set register 32\n");
			result
				=
				copy_from_user(&reg_info, (reg_info_t __user *) arg,
					sizeof(reg_info_t));
			PDEBUG(D_IOCTL, "copy from user done, result: %i, dev->dma_reg: %p, reg_info.value: %u, reg_info.address: %x\n", result, dev->dma_reg, reg_info.value, reg_info.address);
			if (result)
				result = -EFAULT;
			else
				writel(reg_info.value,
					dev->dma_reg + reg_info.address);
			break;

	default:
		PDEBUG(D_IOCTL, ": ioctl unknown\n");
		return -ENOTTY;
	}

	return result;
}
