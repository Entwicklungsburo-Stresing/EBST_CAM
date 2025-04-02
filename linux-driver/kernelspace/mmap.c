/*****************************************************************//**
 * @file		mmap.c
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/) This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.
 *********************************************************************/

#include "device.h"
#include "registers.h"
#include "debug.h"
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/io.h>

/* mapping into
    - page 0: io remap of dma/s0 registers
    - page 1: remap of control structure in physical ram
    - page 2: remap of dma buffer in physical ram
*/

static struct vm_operations_struct mmap_register_remap_vm_ops = {
	//.open =  mmap_register_vma_open,
	//.close = mmap_register_vma_close,
	.open = NULL,
	.close = NULL
};

int mmap_register_remap_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct dev_struct *dev = filp->private_data;

	switch (vma->vm_pgoff) {
	case 0:	/* remap dev->base_address to user space, previously mapped
		  to pci io
		*/
		if (!(device_test_status(dev, DEV_HARDWARE_PRESENT))) {
			PDEBUG(D_MMAP, "asked for io mapping without device");
			return -ENODEV;
		}
		PDEBUG(D_MMAP,
		       "register remap at 0x%08lx of size 0x%08lx for io\n",
		       vma->vm_start, vma->vm_end - vma->vm_start);

		if (io_remap_pfn_range(vma, vma->vm_start,
				       ((u64) dev->
					physical_pci_base) >> PAGE_SHIFT,
				       vma->vm_end - vma->vm_start,
				       vma->vm_page_prot))
			return -EAGAIN;
		break;

	case 1:	/* map control struture in kernel memory to user space */
		PDEBUG(D_MMAP, "ram remap with 0x%08lx for status\n",
		       vma->vm_start);

		if (remap_pfn_range(vma, vma->vm_start,
				    virt_to_phys((void *) dev->
						 control) >> PAGE_SHIFT,
				    sizeof(struct control_struct),
				    vma->vm_page_prot))
			return -EAGAIN;
		break;

	case 2:	/* map dma buffer in kernel memory to user space */
		PDEBUG(D_MMAP, "ram remap with 0x%08lx for dma\n",
		       vma->vm_start);

		if (remap_pfn_range(vma, vma->vm_start,
				    virt_to_phys((void *) dev->
						 dma_virtual_mem) >>
				    PAGE_SHIFT, dev->control->dma_buf_size,
				    vma->vm_page_prot))
			return -EAGAIN;
		break;

	default:
		printk(KERN_ERR NAME ": invalid remap offset %ld\n",
		       vma->vm_pgoff);
		return -EINVAL;
	}

	vma->vm_ops = &mmap_register_remap_vm_ops;

	return 0;
}
