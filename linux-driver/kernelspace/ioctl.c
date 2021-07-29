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


#include <linux/version.h>

/*
 * Kernel 5.0 removed VERIFY_READ and VERIFY_WRITE and removed the first
 * parameter of access_ok() which was set to VERIFY_READ or VERIFY_WRITE.
 *
 * Get rid of the first parameter and always pass VERIFY_WRITE for kernels
 * prior to 5.0.  This will fail for old 386 processors on pre-2.5.70
 * kernels if the memory region is not in fact writeable.
 */
#ifdef VERIFY_WRITE
/* Pre 5.0 kernel. */
static inline int _kcompat_access_ok(unsigned long addr, size_t size)
{
    /* Always use VERIFY_WRITE.  Most architectures ignore it. */
    return access_ok(VERIFY_WRITE, addr, size);
}
/* Redefine access_ok() to remove first parameter. */
#undef access_ok
#define access_ok(addr, size) _kcompat_access_ok((unsigned long)(addr), (size))
#endif


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
	    & !access_ok((void __user *) arg, _IOC_SIZE(cmd)))
		return -EFAULT;

	if (_IOC_DIR(cmd) & _IOC_WRITE
	    & !access_ok((void __user *) arg, _IOC_SIZE(cmd)))
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
	       PDEBUG(D_IOCTL, "register 0x%08x <- 0x%08x\n",
		      reg_info.address, reg_info.value);
	       result
		   =
		   pci_write_config_dword(dev->pci_dev, reg_info.address,
					  reg_info.value);
	       break;
	default:
		PDEBUG(D_IOCTL, ": ioctl unknown\n");
		return -ENOTTY;
	}

	return result;
}
