/* dma.c
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
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

#include <linux/types.h>
#include "../userspace/constants.h"
#include "defaults.h"
#include "device.h"
#include "module-main.h"
#include "dma.h"

/*
- dev->descriptor_buffer points to the buffer containing dma.num_buffers DMA
  descriptors ready for the PLX9056. This buffer is 16 bytes longer than
  necessary for the descriptors themselves because the PLX9056 needs them
  aligned to a 64 bit (long) address.
- dev->descriptors16 points to the first u64 aligned descriptor.
*/

/* get memory for buffers and descriptors */
int init_dma(struct dev_struct *dev) {
  int i;

  /* DMA mask to 32 bit */
  if (dev->hardware_present) {
    PDEBUG(D_BUFFERS, "setting DMA mask\n");
    assert(!dma_set_mask(&dev->pci_dev->dev, 0xFFFFFFFF),
           "DMA not supported\n", -1);
  }

  /* get buffer for DMA descriptors */
  PDEBUG(D_BUFFERS, "getting DMA descriptor buffer\n");
  dev->dma.descriptor_buffer
    = kmalloc(dev->dma.n_buffers * sizeof(dma_descriptor_t) + 0x10,
              GFP_KERNEL | __GFP_DMA);

  assert(dev->dma.descriptor_buffer != 0, "no memory for DMA descriptors.\n",
         -1);

  PDEBUG(D_BUFFERS, "setting up descriptor16\n");
  dev->dma.descriptors
    = (dma_descriptor_t *)
    (((pmem_uint)dev->dma.descriptor_buffer) & ~0x0F) + 0x10;
  memset(dev->dma.descriptors, 0, dev->dma.n_buffers * sizeof(dma_descriptor_t));

  /* get DMA buffers */
  PDEBUG(D_BUFFERS, "getting DMA buffer pointer\n");
  dev->dma.buffers
    = kmalloc(dev->dma.n_buffers * sizeof(uint8_t *), GFP_KERNEL);
  assert(dev->dma.buffers != 0, "no memory for DMA buffer pointers.\n", -1);
  memset(dev->dma.buffers, 0, dev->dma.n_buffers * sizeof(u8 *));

  dev->dma.buf_size = dev->n_pixels * 4;
  PDEBUG(D_BUFFERS, "getting DMA buffers\n");
  for (i = 0; i < dev->dma.n_buffers; i++) {
    dev->dma.buffers[i]
      = kmalloc(dev->dma.buf_size, GFP_KERNEL);/* | __GFP_DMA);*/
    assert(dev->dma.buffers[i] != 0, "no memory for DMA buffer.\n", -1);
  }

  PDEBUG(D_BUFFERS, "DMA done\n");

  return 0;
}

/* release buffer and descriptor memory */
void clean_up_dma(struct dev_struct *dev) {
  int i;

  finish_dma(dev);

  if (dev->dma.buffers) {
    for (i = 0; i < dev->dma.n_buffers; i++)
      if (dev->dma.buffers[i]) kfree(dev->dma.buffers[i]);
    kfree(dev->dma.buffers);
    dev->dma.buffers = 0;
  }
  if (dev->dma.descriptor_buffer) {
    kfree(dev->dma.descriptor_buffer);
    dev->dma.descriptor_buffer = 0;
  }
}

/* map buffers and sync descriptors for device */
int prepare_dma(struct dev_struct *dev) {
  struct device *pdev = &dev->pci_dev->dev;
  int i;

  if (!dev->hardware_present) {
    PDEBUG(D_BUFFERS, "debug version desn't need dma");
    return 0;
  }

  PDEBUG(D_BUFFERS, "preparing dma");

  dev->dma.descriptors_pci_address
    = dma_map_single(pdev, dev->dma.descriptors,
                     dev->dma.n_buffers * sizeof(dma_descriptor_t),
                     DMA_TO_DEVICE);

  if (dma_mapping_error(pdev, dev->dma.descriptors_pci_address)) {
    printk(KERN_WARNING NAME " mapping of descriptor buffer failed\n");
    return -ENOBUFS;
  }

  PDEBUG(D_BUFFERS, "mapped desc16 0x%08llx to physical address 0x%08llx\n",
         (pmem_uint) dev->dma.descriptors,
         (pmem_uint) dev->dma.descriptors_pci_address);

  /* map data buffers */
  for (i = 0; i < dev->dma.n_buffers; i++) {
    memset(dev->dma.buffers[i], 0xA5, dev->n_pixels * 4);
    dev->dma.buffers[i][0] = 0xA5;
    dev->dma.descriptors[i].pci_address
      = dma_map_single(pdev, dev->dma.buffers[i], dev->dma.buf_size,
                       DMA_FROM_DEVICE);

    if (dma_mapping_error(pdev, dev->dma.descriptors[i].pci_address)) {
      printk(KERN_WARNING NAME " mapping of buffer failed\n");
      return -ENOBUFS;
    }

    PDEBUG(D_BUFFERS, "mapped buffer 0x%08llx to physical address 0x%08llx\n",
           (pmem_uint) dev->dma.buffers[i],
           (pmem_uint) dev->dma.descriptors[i].pci_address);
  }

  /* write buffer addresses to DMA descriptors */
  dma_sync_single_for_cpu(pdev, dev->dma.descriptors_pci_address,
                          sizeof(dma_descriptor_t) * dev->dma.n_buffers,
                          DMA_TO_DEVICE);

  /* four lsb: - 0x01 descriptor in PCI space - not 0x02 not end of chain
     - 0x04 interrupt after transfer of block completed
     - 0x08 from local to PCI */
  for (i = 0; i < dev->dma.n_buffers; i++) {
    dev->dma.descriptors[i].local_address = 0;    
    dev->dma.descriptors[i].byte_count = dev->dma.buf_size;
    dma_sync_single_for_cpu(pdev, dev->dma.descriptors[i].pci_address,
                          sizeof(dma_descriptor_t) * dev->dma.n_buffers,
                          DMA_TO_DEVICE);
  }

  /* done, reactivate DMA mapping */
  dma_sync_single_for_device(pdev, dev->dma.descriptors_pci_address,
                           sizeof(dma_descriptor_t) * dev->dma.n_buffers,
                             DMA_TO_DEVICE);

  PDEBUG(D_BUFFERS, "mapping of buffers ok, dma prepared\n");

  return 0;
}

int finish_dma(struct dev_struct *dev) {
  int result, i;

  result = plx9056_finish_dma(dev);

  for (i = 0; i < dev->dma.n_buffers; i++)
    if (dev->dma.descriptors[i].pci_address)
      dma_unmap_single(&dev->pci_dev->dev,
                       dev->dma.descriptors[i].pci_address,
                       dev->dma.buf_size, DMA_FROM_DEVICE);

  memset(dev->dma.descriptors, 0,
         dev->dma.n_buffers * sizeof(dma_descriptor_t));

  if (dev->dma.descriptors_pci_address) {
    dma_unmap_single(&dev->pci_dev->dev, dev->dma.descriptors_pci_address,
                     sizeof(dma_descriptor_t) * dev->dma.n_buffers,
                     DMA_TO_DEVICE);
    dev->dma.descriptors_pci_address = 0;
  }

  return result;
}
