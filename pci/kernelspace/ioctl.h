/* ioctl.h
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

#ifndef _ioctl_h_
#define _ioctl_h_

#include <linux/types.h>

#define LSCPCI_MAGIC 0xE0

/* camera operation */
#define LSCPCI_IOCTL_GET_PIXELS       _IOR(LSCPCI_MAGIC, 1, int)
#define LSCPCI_IOCTL_INIT             _IO(LSCPCI_MAGIC,  2)
#define LSCPCI_IOCTL_START            _IO(LSCPCI_MAGIC,  3)
#define LSCPCI_IOCTL_IDLE_RUN         _IO(LSCPCI_MAGIC,  4)
#define LSCPCI_IOCTL_STOP             _IO(LSCPCI_MAGIC,  5)
#define LSCPCI_IOCTL_STATE            _IOR(LSCPCI_MAGIC, 6, int)
#define LSCPCI_IOCTL_CLEAR_FIFO       _IO(LSCPCI_MAGIC,  7)
#define LSCPCI_IOCTL_FIFO_OVERFLOW    _IOR(LSCPCI_MAGIC, 8, int)
#define LSCPCI_IOCTL_BYTES_AVAILABLE  _IOR(LSCPCI_MAGIC, 9, uint32_t)

/* register accesss */
#define LSCPCI_IOCTL_SET_REG_BYTE     _IOW(LSCPCI_MAGIC, 10, int)
#define LSCPCI_IOCTL_SET_REG_WORD     _IOW(LSCPCI_MAGIC, 11, int)
#define LSCPCI_IOCTL_SET_REG_DWORD    _IOW(LSCPCI_MAGIC, 12, uint32_t*)
#define LSCPCI_IOCTL_SET_PLX_REG      _IOW(LSCPCI_MAGIC, 13, uint32_t*)

/* debug functions */
#define LSCPCI_IOCTL_HARDWARE_PRESENT _IOR(LSCPCI_MAGIC, 14, int)
#define LSCPCI_IOCTL_FREE_BYTES       _IOR(LSCPCI_MAGIC, 15, uint32_t)
#define LSCPCI_IOCTL_BUF_PTR          _IOR(LSCPCI_MAGIC, 16, uint64_t)
#define LSCPCI_IOCTL_DUMP_REG         _IOR(LSCPCI_MAGIC, 17, void*)
#define LSCPCI_IOCTL_SET_DEBUG        _IOW(LSCPCI_MAGIC, 18, uint32_t*)

#define LSCPCI_IOCTL_MAX 19

#ifdef __KERNEL__
long ioctl_lscpci(struct file *, unsigned int, unsigned long);
#endif

#endif /* _ioctl_h_ */
