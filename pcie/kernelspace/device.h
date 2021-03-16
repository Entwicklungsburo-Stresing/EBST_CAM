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

#define MOD_PCI_REGISTERED   0x01

#define DEV_HARDWARE_PRESENT 0x01
#define DEV_PCI_ENABLED      0x04
#define DEV_CDEV_CREATED     0x08
#define DEV_CLASS_CREATED    0x10
#define DEV_MSI_ENABLED      0x20
#define DEV_IRQ_REQUESTED    0x40

#define DEV_FIFO_OVERFLOW    0x4000
#define DEV_DMA_OVERFLOW     0x8000

struct dev_struct {
  u16 status;
  u32 physical_pci_base;
  void __iomem *mapped_pci_base;
  dma_addr_t dma_handle;
  void *dma_virtual_mem;
  u32 dma_mem_size;
  lscpcie_control_t *control;
  struct pci_dev *pci_dev;
  struct cdev cdev;
  struct proc_dir_entry *proc_data_entry;
  struct proc_dir_entry *proc_registers_entry;
  struct proc_dir_entry *proc_registers_long_entry;
  struct proc_dir_entry *proc_io_entry;
  wait_queue_head_t readq, writeq;
  struct semaphore write_sem, read_sem, size_sem;
  atomic_t read_available;
  atomic_t write_available;
  u16 debug_mode;
  int minor;
  dev_t device;
  int proc_actual_register;
  int proc_actual_register_long;
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
