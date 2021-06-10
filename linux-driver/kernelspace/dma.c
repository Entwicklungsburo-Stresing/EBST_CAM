/* dma.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include "dma.h"
#include "module-main.h"
#include "registers.h"
#include "device.h"
#include "debug.h"
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>

#define memory_barrier() asm volatile ("" : : : "memory")

#define pulse_bit(dev, reg, bit_mask) do {				\
		set_bits_s0_dword(dev, reg, bit_mask, bit_mask);	\
		memory_barrier();					\
		set_bits_s0_dword(dev, reg, 0, bit_mask);		\
	} while (0)

static enum irqreturn isr(int irqn, void *dev_id);

int dma_init(struct dev_struct *dev)
{
	struct device *pdev;
	int num_dma_pages, result;
	int dev_no = get_device_number(dev);

	PDEBUG(D_BUFFERS, "initialising dma\n");

	if (dev_no < 0) {
		printk(KERN_ERR NAME
		       ": invalid device pointer in init_dma\n");
		return -ENODEV;
	}

	if (device_test_status(dev, DEV_HARDWARE_PRESENT)) {
		pdev = &dev->pci_dev->dev;
		PDEBUG(D_BUFFERS, "setting dma mask\n");
		if (dma_set_mask_and_coherent(pdev, DMA_BIT_MASK(32))) {
			printk(KERN_ERR NAME ": No suitable DMA available\n");
			return -ENOMEM;
		}
	} else {
		pdev = 0;
	}

	/* the size of the dma buffer is taken one page size larger than
           necessary to ensure that the used buffer starts at a page boundary
           (needed for mmap export to userland)
        */
	PDEBUG(D_BUFFERS, "calculating dma buffer size\n");
	dev->control->dma_buf_size
	    =
	    dev->control->number_of_cameras *
	    dev->control->number_of_pixels * dev->control->dma_num_scans *
	    sizeof(u16);
	num_dma_pages = dev->control->dma_buf_size >> PAGE_SHIFT;
	if (dev->control->dma_buf_size > num_dma_pages << PAGE_SHIFT)
		num_dma_pages++;

	dev->dma_mem_size = num_dma_pages << PAGE_SHIFT;

	PDEBUG(D_BUFFERS, "need %d bytes for dma\n", dev->dma_mem_size);

	if (device_test_status(dev, DEV_HARDWARE_PRESENT)) {
		PDEBUG(D_BUFFERS, "allocating %d bytes of dma memory\n",
		       dev->dma_mem_size);
		dev->dma_virtual_mem
		    =
		    dma_alloc_coherent(pdev, dev->dma_mem_size,
				       &dev->dma_handle, GFP_KERNEL);
		dev->control->dma_physical_start = (u64) dev->dma_handle;
	} else {		/* no dma features needed in debug mode */
		PDEBUG(D_BUFFERS,
		       "allocating %d bytes of kernel memory for debugging\n",
		       dev->dma_mem_size);
		dev->dma_virtual_mem =
		    kmalloc(dev->dma_mem_size, GFP_KERNEL);
	device_set_status(dev, DEV_DMA_DEBUG_MEM, DEV_DMA_DEBUG_MEM);
#warning set pages reserved
	}

	if (!dev->dma_virtual_mem) {
		printk(KERN_ERR NAME ": failed to allocate dma memory\n");
		return -ENOMEM;
	}

	device_set_status(dev, DEV_DMA_MEM_ALLOCATED, DEV_DMA_MEM_ALLOCATED);

	dev->control->dma_buf_size = dev->dma_mem_size;
	if (device_test_status(dev, DEV_HARDWARE_PRESENT)) {
		result = dma_start(dev);
		if (result) {
			dma_finish(dev);
			return result;
		}
	}

	PDEBUG(D_BUFFERS, "dma initialised\n");

	return 0;
}

/* release dma buffer */
void dma_finish(struct dev_struct *dev)
{
	dma_end(dev);
	if (device_test_status(dev, DEV_DMA_MEM_ALLOCATED)) {
		if (device_test_status(dev, DEV_DMA_DEBUG_MEM)) {
			PDEBUG(D_BUFFERS, "freeing debug buffer");
			kfree(dev->dma_virtual_mem);
		} else {
			PDEBUG(D_BUFFERS, "freeing dma buffer");
			dma_free_coherent(&dev->pci_dev->dev,
					  dev->dma_mem_size,
					  dev->dma_virtual_mem,
					  dev->dma_handle);
		}
		dev->dma_virtual_mem = 0;
		device_set_status(dev,
				  DEV_DMA_MEM_ALLOCATED | DEV_DMA_DEBUG_MEM, 0);
	}
}

int dma_start(struct dev_struct *dev)
{
	int result;
	void *dev_id = dev;

	printk(KERN_ERR NAME ": requesting irq line %i.", dev->irq_line);
	result = request_irq(dev->irq_line, isr, 0, "lscpcie", dev_id);
	if (result) {
		printk(KERN_ERR NAME
		       ": requesting interrupt failed with error %d\n",
		       result);
		return result;
	}
	device_set_status(dev, DEV_IRQ_REQUESTED, DEV_IRQ_REQUESTED);

	return 0;
}

int dma_end(struct dev_struct *dev)
{
	if (device_test_status(dev, DEV_IRQ_REQUESTED)) {
		printk(KERN_ERR NAME ": freeing interrupt %d...\n",
		       dev->irq_line);
		free_irq(dev->irq_line, dev);
	}
	device_set_status(dev, DEV_IRQ_REQUESTED, 0);

	return 0;
}

/* interrupt service routine */
static enum irqreturn isr(int irqn, void *dev_id)
{
	int old_write_pos;
	u8 fifo_flags;
	struct dev_struct *dev = (struct dev_struct *) dev_id;
	dev->control->irq_count++;
	PDEBUG(D_INTERRUPT, "got interrupt %d\n", dev->control->irq_count);

	old_write_pos = dev->control->write_pos;
	fifo_flags = read_s0_byte(dev, FF_FLAGS);

	set_bits_s0_dword(dev, IRQREG.REG32, (1 << IRQ_REG_ISR_active),
			  (1 << IRQ_REG_ISR_active));

	if (fifo_flags & (1 << FF_FLAGS_OVFL))
		device_set_status(dev, DEV_FIFO_OVERFLOW, DEV_FIFO_OVERFLOW);

	// advance buffer pointer
	dev->control->write_pos
	    = (dev->control->write_pos + dev->control->bytes_per_interrupt)
	    % dev->control->used_dma_size;


	// check for buffer overflow
	if (old_write_pos < dev->control->write_pos) {
		if ((dev->control->read_pos <= old_write_pos)
		    || (dev->control->read_pos > dev->control->write_pos))
			goto end;	/* r w0 w1  or w0 w1 r */
	} else if ((dev->control->read_pos <= old_write_pos)
		   && (dev->control->read_pos > dev->control->write_pos))
		goto end;	/* w1 r w0 */

	device_set_status(dev, DEV_DMA_OVERFLOW, DEV_DMA_OVERFLOW);

      end:
	set_bits_s0_dword(dev, IRQREG.REG32, 0,
			  (1 << IRQ_REG_ISR_active));
	PDEBUG(D_INTERRUPT, "pointers %d %d\n", dev->control->write_pos,
	       dev->control->read_pos);

	wake_up_interruptible(&dev->readq);
	wake_up_interruptible(&dev->proc_readq);

	return IRQ_HANDLED;
}
