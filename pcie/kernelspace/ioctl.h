/*
 * ioctl.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#ifndef _ioctl_h_
#define _ioctl_h_


#include <linux/types.h>


#define LSCPCIE_MAGIC 0xE0

typedef struct {
  uint32_t value;
  uint16_t address;
} reg_info_t;

/* camera operations */
#define LSCPCIE_IOCTL_START            _IO(LSCPCIE_MAGIC,  1)
#define LSCPCIE_IOCTL_IDLE_RUN         _IO(LSCPCIE_MAGIC,  2)
#define LSCPCIE_IOCTL_STOP             _IO(LSCPCIE_MAGIC,  3)
#define LSCPCIE_IOCTL_FREE_BYTES       _IOR(LSCPCIE_MAGIC, 4, int *)
#define LSCPCIE_IOCTL_BYTES_AVAILABLE  _IOR(LSCPCIE_MAGIC, 5, int *)
#define LSCPCIE_IOCTL_FIFO_OVERFLOW    _IOR(LSCPCIE_MAGIC, 6, int *)
#define LSCPCIE_IOCTL_CLEAR_FIFO       _IO(LSCPCIE_MAGIC, 7)
#define LSCPCIE_IOCTL_NUM_BOARDS       _IOR(LSCPCIE_MAGIC, 8, int *)

/* register operations */
#define LSCPCIE_IOCTL_GET_REG8         _IOWR(LSCPCIE_MAGIC, 9, reg_info_t *)
#define LSCPCIE_IOCTL_GET_REG16        _IOWR(LSCPCIE_MAGIC, 10, reg_info_t *)
#define LSCPCIE_IOCTL_GET_REG32        _IOWR(LSCPCIE_MAGIC, 11, reg_info_t *)
#define LSCPCIE_IOCTL_SET_REG8         _IOWR(LSCPCIE_MAGIC, 12, reg_info_t *)
#define LSCPCIE_IOCTL_SET_REG16        _IOWR(LSCPCIE_MAGIC, 13, reg_info_t *)
#define LSCPCIE_IOCTL_SET_REG32        _IOWR(LSCPCIE_MAGIC, 14, reg_info_t *)
#define LSCPCIE_IOCTL_GET_CONF         _IOR(LSCPCIE_MAGIC, 15, reg_info_t *)
#define LSCPCIE_IOCTL_SET_CONF         _IOW(LSCPCIE_MAGIC, 16, reg_info_t *)

/* debug functions */
#define LSCPCIE_IOCTL_HARDWARE_PRESENT _IOR(LSCPCIE_MAGIC, 17, int *)
#define LSCPCIE_IOCTL_BUFFER_POINTERS  _IOR(LSCPCIE_MAGIC, 18, uint64_t *)
#define LSCPCIE_IOCTL_SET_DEBUG        _IOW(LSCPCIE_MAGIC, 19, uint32_t)

#define LSCPCIE_IOCTL_MAX 20


struct file;

long lscpcie_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);


#endif // _ioctl_h_
