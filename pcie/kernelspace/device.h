/*
 * device.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _device_h_
#define _device_h_


#define NAME "lscpcie"

#include "dma.h"
#include "registers.h"
#include "module-main.h"
#include <linux/cdev.h>
#include <linux/semaphore.h>

#define HARDWARE_PRESENT 0x01
#define PCI_REGISTERED   0x02
#define DEVICE_CREATED   0x04
#define FIFO_OVERFLOW    0x08
#define DMA_OVERFLOW     0x10

struct dev_struct {
  u8 status;
  u32 physical_pci_base;
  void __iomem *mapped_pci_base;
  dma_addr_t dma_handle;
  void *dma_virtual_mem;
  u32 dma_mem_size;
  lscpcie_control_t *control;
  uint32_t scans_per_interrupt;
  uint32_t bytes_per_interrupt;
  struct pci_dev *pci_dev;
  struct cdev cdev;
  struct proc_dir_entry *proc_data_entry;
  struct proc_dir_entry *proc_registers_entry;
  struct proc_dir_entry *proc_io_entry;
  wait_queue_head_t readq, writeq;
  struct semaphore write_sem, read_sem, size_sem;
  atomic_t read_available;
  atomic_t write_available;
  u16 debug_mode;
  int minor;
  dev_t device;
  int proc_actual_register;
  u8 irq_line;
};


extern struct dev_struct lscpcie_devices[MAX_BOARDS];

int device_init(struct dev_struct *dev, int minor);
void device_clean_up(struct dev_struct *dev);
struct dev_struct *device_data(uint8_t devno);

#define VENDOR_ID 0x10EE
#define DEVICE_ID 0x0007
#define SUBSYSTEM_VENDOR_ID 0x4553
#define SUBSYSTEM_DEVICE_ID 0x7401

#define MAGIC_ADDRESS 0x04
#define MAGIC_NUMBER  0x53
#define EBST_ADDRESS  0x1c

#endif /* _device_h_ */
