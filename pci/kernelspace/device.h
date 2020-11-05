/* device.h
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

#ifndef _device_h_
#define _device_h_

#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/semaphore.h>
#include "../userspace/types.h"
#include "dma.h"
#ifdef __plx_pci__
# include "plx9056.h"
# define NAME "lscpci"
#endif
#ifdef __epci__
# define NAME "lscpcie"
#endif

struct dev_struct {
  u8 initialised;
  u8 hardware_present;
  u16 n_pixels;
  u8 lines;
  u8 vfreq;
#ifdef __plx_pci__
  u8 *pci_config;
  u8 *pci_camera;
#endif
  dev_t dev;
  struct pci_dev *pci_dev;
  dma_buffer_t dma;
#ifdef WITH_POLLING
  struct timer_list poll_timer;
#endif
  int status;
  int free_count;
  int have_irq;
  int irq_count;
  wait_queue_head_t readq, writeq;
  struct semaphore write_sem, read_sem, size_sem;
  atomic_t read_available;
  atomic_t write_available;
  struct cdev cdev;
  struct proc_dir_entry *proc_entry;
  struct proc_dir_entry *proc_registers_entry;
  struct board_vars vars;
  u16 debug_mode;
  u32 acquisition_count;
  u32 timer_count;
  size_t write_count, read_count;
};

#define assert(condition, error_message, return_code)                       \
  do { if (!(condition)) {                                                  \
      printk(NAME ": %s\n", error_message); clean_up(); return return_code; \
    } } while (0)


#ifdef PDEBUG
# undef PDEBUG
#endif
#define PDEBUG(flags, fmt, args...) do {                \
    if (DEV->debug_mode & (flags))			\
    printk(KERN_WARNING "lscpci: " fmt, ## args);       \
} while (0)
#define DEV dev

#endif /* _device_h_ */
