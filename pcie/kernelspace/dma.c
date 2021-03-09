/*
 * dma.c
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include "dma.h"
#include "module-main.h"
#include "registers.h"
#include "device.h"
#include "debug.h"
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>


int dma_init(struct dev_struct *dev) {
  struct device *pdev = dev->status & HARDWARE_PRESENT ? &dev->pci_dev->dev : 0;
  int num_dma_pages;
  int dev_no = get_device_number(dev);

  PDEBUG(D_BUFFERS, "initialising dma\n");

  if (dev_no < 0) {
    printk(KERN_ERR NAME": invalid device pointer in init_dma\n");
    return -ENODEV;
  }

  /* the size of the dma buffer is taken one page size larger than necessary
     to ensure that the used buffer starts at a page boundary (needed for
     mmap export to userland) */
  dev->control->dma_buf_size
    = dev->control->number_of_cameras * dev->control->number_of_pixels
    * dev->control->dma_num_scans * sizeof(u16);
  num_dma_pages = dev->control->dma_buf_size >> PAGE_SHIFT;
  if (dev->control->dma_buf_size > num_dma_pages << PAGE_SHIFT)
    num_dma_pages++;

  dev->dma_mem_size = num_dma_pages << PAGE_SHIFT;

  PDEBUG(D_BUFFERS, "need %d bytes for dma\n", dev->dma_mem_size);

  if (dev->status & HARDWARE_PRESENT) {
    PDEBUG(D_BUFFERS, "allocating %d bytes of dma memory\n",
           dev->dma_mem_size);
    dev->dma_virtual_mem
      = dma_alloc_coherent(pdev, dev->dma_mem_size, &dev->dma_handle,
			   GFP_KERNEL);
    dev->control->dma_physical_start = (u64) dev->dma_handle;
  } else {/* no dma features needed in debug mode */
    PDEBUG(D_BUFFERS, "allocating %d bytes of kernel memory for debugging\n",
           dev->dma_mem_size);
    dev->dma_virtual_mem = kmalloc(dev->dma_mem_size, GFP_KERNEL);
#warning set pages reserved
  }

  if (!dev->dma_virtual_mem) {
    printk(KERN_ERR NAME": failed to allocate dma memory\n");
    return -ENOMEM;
  }

  dev->control->dma_buf_size = dev->dma_mem_size;
  dev->bytes_per_interrupt = 500;
  dma_start(dev);
  
  PDEBUG(D_BUFFERS, "dma initialised\n");

  return 0;
}

/* release dma buffer */
void dma_finish(struct dev_struct *dev)
{
  dma_end(dev);
  if (dev->dma_virtual_mem) {
    if (dev->status & HARDWARE_PRESENT) {
      PDEBUG(D_BUFFERS, "freeing dma buffer");
      dma_free_coherent(&dev->pci_dev->dev, dev->dma_mem_size,
			dev->dma_virtual_mem, dev->dma_handle);
    } else {
      PDEBUG(D_BUFFERS, "freeing debug buffer");
      kfree(dev->dma_virtual_mem);
    }
    dev->dma_virtual_mem = 0;
  }
}

/* interrupt service routine */
static enum irqreturn isr(int irqn, void *dev_id)
{
  /* >>>>> first check whether the pcie card has issued the interrupt */
  /* if not return IRQ_NONE; */
  /* <<<<< to be implemented */

  struct dev_struct *dev = (struct dev_struct *) dev_id;
  int old_write_pos = dev->control->write_pos;
  u8 fifo_flags = readb(dev->mapped_pci_base + 0x80 + S0Addr_FF_FLAGS);

  set_bits_s0_dword(dev, S0Addr_IRQREG, (1<<IRQ_REG_ISR_active),
                    (1<<IRQ_REG_ISR_active));

  if (fifo_flags & (1<<FF_FLAGS_OVFL)) dev->status |= FIFO_OVERFLOW;

  // advance buffer pointer
  dev->control->write_pos
    = (dev->control->write_pos + dev->bytes_per_interrupt)
    % dev->control->dma_buf_size;

  // check for buffer overflow
  if (old_write_pos < dev->control->write_pos) {
    if ((dev->control->read_pos <= old_write_pos)
        ||
        (dev->control->read_pos > dev->control->write_pos))
      goto end; /* r w0 w1  or w0 w1 r */
  } else
    if ((dev->control->read_pos <= old_write_pos)
        &&
        (dev->control->read_pos > dev->control->write_pos))
      goto end; /* w1 r w0 */

  dev->status |= DMA_OVERFLOW;

 end:
  set_bits_s0_dword(dev, S0Addr_IRQREG, 0, (1<<IRQ_REG_ISR_active));

  wake_up_interruptible(&dev->readq);

  return IRQ_HANDLED;
}

int dma_start(struct dev_struct *dev)
{
  int result;

  unsigned long irqflags = IRQF_SHARED;
  void *dev_id = dev;

  printk(KERN_ERR NAME": requesting irq line %i.",dev->irq_line);
  result = request_irq(dev->irq_line, isr, irqflags, "lscpcie", dev_id);
  if (result) {
    printk(KERN_ERR NAME": requesting interrupt failed with error %d\n", result);
    return result;
  }

  return 0;
}

int dma_end(struct dev_struct *dev)
{
  printk(KERN_ERR NAME": freeing interrupt...");
  free_irq(dev->irq_line, dev);
  /* >>>> clean-up if necessary
     <<<< */

  return 0;
}
