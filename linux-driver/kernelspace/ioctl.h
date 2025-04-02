/*****************************************************************//**
 * @file		ioctl.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
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
