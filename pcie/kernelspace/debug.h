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


#include "../userspace/constants.h"


#define PDEBUG(flags, ...) do {                   \
  if (dev->debug_mode & (flags))                  \
    printk(KERN_WARNING "lscpcie: " __VA_ARGS__); \
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
