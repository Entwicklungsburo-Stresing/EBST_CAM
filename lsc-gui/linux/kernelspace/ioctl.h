/* ioctl.h
 *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


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
#define LSCPCIE_IOCTL_GET_REG8   _IOWR(LSCPCIE_MAGIC, 4, reg_info_t *)
#define LSCPCIE_IOCTL_GET_REG16  _IOWR(LSCPCIE_MAGIC, 5, reg_info_t *)
#define LSCPCIE_IOCTL_GET_REG32  _IOWR(LSCPCIE_MAGIC, 6, reg_info_t *)
#define LSCPCIE_IOCTL_SET_REG8   _IOWR(LSCPCIE_MAGIC, 7, reg_info_t *)
#define LSCPCIE_IOCTL_SET_REG16  _IOWR(LSCPCIE_MAGIC, 8, reg_info_t *)
#define LSCPCIE_IOCTL_SET_REG32  _IOWR(LSCPCIE_MAGIC, 9, reg_info_t *)
#define LSCPCIE_IOCTL_MAX 6


struct file;

long lscpcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


#endif	/* _ioctl_h_ */
