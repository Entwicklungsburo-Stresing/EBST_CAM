/*
 * debug.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _debug_h_
#define _debug_h_

#define MAX_PCIE_BOARDS 5

/* debugging */
#define D_PCI            0x0001
#define D_START_STOP     0x0002
#define D_READOUT        0x0004
#define D_INTERRUPT      0x0008
#define D_BUFFERS        0x0010
#define D_IOCTL          0x0020
#define D_MODULE         0x0040
#define D_MMAP           0x0080
#define D_PROC           0x0100
#define D_STATUS         0x0200

#define DEBUG_BITS       0x03FF
#define DEBUG_MASK_SHIFT 16

#define PDEBUG(flags, ...) do {						  \
	if ((dev->control && (dev->control->debug_mode & (flags)))	  \
		|| ((!dev->control) && (dev->init_debug_mode & (flags)))) \
		printk(KERN_WARNING "lscpcie: " __VA_ARGS__);		  \
} while (0)

#define PMESSAGE(...) \
  do { printk(KERN_WARNING "lscpcie: " __VA_ARGS__); } while (0)

#define assert(condition, error_message, fail_action, return_code)          \
  do { if (!(condition)) {                                                  \
      printk(NAME ": %s\n", error_message); clean_up_lscpcie_module();      \
      result = return_code;                                                 \
      do { fail_action; } while (0);                                        \
    } } while (0)

#endif /* _debug_h_ */
