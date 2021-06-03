/* device.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#define DEV_PCI_ENABLED       0x0001
#define DEV_CDEV_CREATED      0x0002
#define DEV_CLASS_CREATED     0x0004
#define DEV_DMA_MEM_ALLOCATED 0x0008
#define DEV_DMA_DEBUG_MEM     0x0010
#define DEV_IRQ_ALLOCATED     0x0020
#define DEV_IRQ_REQUESTED     0x0040
#define DEV_CONTROL_MAPPED    0x0080
#define DEV_BLOCKS_IN_IRQ     0x0100
/* bits 0xE000 in registers-common */

struct dev_struct {
	u16 init_status;
	u16 init_debug_mode;
	u32 physical_pci_base;
	//void __iomem *mapped_pci_base;
	struct dma_reg_struct *dma_reg;
	struct s0_reg_struct *s0_reg;
	dma_addr_t dma_handle;
	void *dma_virtual_mem;
	u32 dma_mem_size;
	struct control_struct *control;
	struct pci_dev *pci_dev;
	struct cdev cdev;
	struct proc_dir_entry *proc_data_entry;
	struct proc_dir_entry *proc_registers_entry;
	struct proc_dir_entry *proc_registers_long_entry;
	struct proc_dir_entry *proc_io_entry;
	wait_queue_head_t readq, writeq;
	struct semaphore write_sem, read_sem, size_sem;
	wait_queue_head_t proc_readq;
	struct semaphore proc_read_sem;
	atomic_t read_available;
	atomic_t write_available;
	int minor;
	dev_t device;
	u8 irq_line;
};


extern struct dev_struct lscpcie_devices[MAX_BOARDS];

int device_init(struct dev_struct *dev, int minor);
void device_clean_up(struct dev_struct *dev);
struct dev_struct *device_data(uint8_t devno);
int device_test_status(struct dev_struct *dev, u16 bits);
void device_set_status(struct dev_struct *dev, u16 mask, u16 bits);

#define VENDOR_ID 0x10EE
#define DEVICE_ID 0x0007
#define SUBSYSTEM_VENDOR_ID 0x4553
#define SUBSYSTEM_DEVICE_ID 0x7401

#define MAGIC_ADDRESS 0x04
#define MAGIC_NUMBER  0x53
#define EBST_ADDRESS  0x1c

#endif /* _device_h_ */
