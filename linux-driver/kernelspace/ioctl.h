/*****************************************************************//**
 * @file		ioctl.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This file is dual licensed. As part of the the userspace library lscpcie and libESLSCLDLL.so is is licensed under the LPGL-3.0. As part of the kernel module lscpcie.ko it is licensed under the GPL-2.0.
 *********************************************************************/


#ifndef _ioctl_h_
#define _ioctl_h_


#include <linux/types.h>


typedef struct {
	uint32_t value;
	uint16_t address;
} reg_info_t;

#define LSCPCIE_MAGIC 0xE0

#define LSCPCIE_IOCTL_NUM_BOARDS _IOR(LSCPCIE_MAGIC, 1, int *)
#define LSCPCIE_IOCTL_GET_CONF	 _IOR(LSCPCIE_MAGIC, 2, reg_info_t *)
#define LSCPCIE_IOCTL_SET_CONF	 _IOW(LSCPCIE_MAGIC, 3, reg_info_t *)
#define LSCPCIE_IOCTL_MAX 4


struct file;

long lscpcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


#endif	/* _ioctl_h_ */
