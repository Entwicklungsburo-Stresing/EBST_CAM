/* plx9056.c
 *
 * Copyright (C) 2010-2016 Bernhard Lang, University of Geneva
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

#include <linux/delay.h>
#include "../userspace/constants.h"        
#include "device.h"
#include "module-main.h"
#include "plx9056.h"

#undef DEV
#define DEV pdev

int probe_plx9056(struct pci_dev *dev, const struct pci_device_id *id) {
  unsigned int base_address, address_size, pciidr, pcicr;
  int result, minor;
  unsigned short pcisvid, pcisid, pcimgr;

  struct dev_struct *pdev = NULL;

  minor = 0;
  while (device_data(minor)->initialised) minor++;
  /* there's a never initialised dummy entry */

  if (!(pdev = device_data(minor))) {
    printk(KERN_WARNING NAME " no free minor number available\n");
    return -ENOBUFS;
  }

  PDEBUG(D_PLX, "taking %d\n", minor);

  if ((result = pci_enable_device(dev)) < 0) return result;

  pci_set_drvdata(dev, pdev);

  PDEBUG(D_PLX, "plx9056 chip found, assigned minor device number %d\n", minor);

  // validate pci configuration registers using access functions
  result = pci_read_config_dword(dev, PLX9056_PCIIDR, &pciidr);
  assert(pciidr == 0x905610B5,
         "wrong PCI9056 card vendor and device id", -1);

  base_address = pci_resource_start(dev, 0);
  address_size = pci_resource_len(dev, 0);

  printk(KERN_WARNING NAME": PLX base address is 0x%x, size %d\n", base_address,
         address_size);

  pdev->pci_config = (char *)ioremap(base_address, address_size);
  assert(pdev->pci_config != 0, "ioremap failed", -1);

  base_address = pci_resource_start(dev, 2);
  address_size = pci_resource_len(dev, 2);

  pdev->pci_camera = (char *)ioremap(base_address, address_size);
  assert(pdev->pci_camera != 0, "ioremap failed", -1);

  printk(KERN_WARNING NAME": camera base address is 0x%x, size %d\n",
         base_address, address_size);

  result = pci_read_config_dword(dev, PLX9056_PCICR, &pcicr);
  assert(!result, "reading PCICR register failed", -1);
  pcicr |= 0x4; // set master mode
  result = pci_write_config_dword(dev, PLX9056_PCICR, pcicr);

  assert(!result, "reading back PCICR register failed", -1);
  assert(pcicr & 0x04, "PLX9056 card not in master mode", -1);

  printk(KERN_WARNING NAME": set master mode\n");

  result = pci_read_config_dword(dev, PLX9056_PCICR, &pcicr);

  result = pci_read_config_word(dev, PLX9056_PCISVID, &pcisvid);
  pcisvid &= 0xFFFF;
  assert(pcisvid == 0x4553, "wrong PLX9056 vendor id", -1);

  printk(KERN_WARNING NAME": PLX9056 vendor id ok\n");

  result = pci_read_config_word(dev, PLX9056_PCISID, &pcisid);
  pcisid &= 0xFFFF;
  assert(pcisid == 0x7401, "wrong PLX9056 subsystem id", -1);

  printk(KERN_WARNING NAME": PLX9056 subsystem id ok\n");

  result = pci_read_config_word(dev, PLX9056_PCIMGR, &pcimgr);
  assert(!result, "failed to read PCIMGR\n", -1);
  pcimgr |= 0xFF;
  result = pci_write_config_word(dev, PLX9056_PCIMGR, pcimgr);
  assert(!result, "failed to write PCIMGR\n", -1);

  writel(0, pdev->pci_config + PLX9056_INTCSR);

  printk(KERN_WARNING NAME": PLX9056 PCIMGR ok, interrupt is %d\n", dev->irq);

/* 16bit bus, local address is 0, no sym, burst */
/* this is obsolete
  writel(DMA0_16BIT | DMA0_INPUT_ENA | DMA0_LOCAL_CONST
         | DMA0_LOCAL_BURST_ENA | DMA0_SG
         | DMA0_DEMAND_MODE | DMA0_TO_PCI | (4<<2), // waits
         pdev->pci_config + PLX9056_DMAMODE0);
*/
  /* abort any DMA pending from buggy drivers :-) */
  writel(DMA0_CLEAR_INTR | DMA0_ABORT,
         pdev->pci_config + PLX9056_DMACSR0);

  pdev->pci_dev = dev;
  pdev->hardware_present = 1;

  return init_board(pdev, minor);
}

void remove_plx9056(struct pci_dev *dev) {
  struct dev_struct *pdev = pci_get_drvdata(dev);

  PDEBUG(D_PLX, "removing plx\n");

  if (pdev->pci_config) {
    iounmap(pdev->pci_config);
    pdev->pci_config = 0;
  }
  if (pdev->pci_camera) {
    iounmap(pdev->pci_camera);
    pdev->pci_camera = 0;
  }
  pdev->hardware_present = 0;
}

#undef DEV
#define DEV dev

/* note: must not be called when DMA is still active */
void plx9056_start_dma(struct dev_struct *dev, int single_buffer) {
  int intcsr;
  long descriptor;

  if (!dev->hardware_present) return;

  /* 16bit bus, local address is 0, no sym, burst */
  writel(DMA0_16BIT | DMA0_INPUT_ENA | DMA0_LOCAL_CONST
         | DMA0_LOCAL_BURST_ENA | DMA0_SG
         | DMA0_DEMAND_MODE | DMA0_INTERRUPT_TO_PCI, //| (4<<2) /* waits */,
         dev->pci_config + PLX9056_DMAMODE0);

  PDEBUG(D_START_STOP, "setting up plx9056 for DMA transfer\n");

  writel(0, dev->pci_config + PLX9056_INTCSR); /* clear interrupt ctrl/status*/

  writel(0, dev->pci_config + PLX9056_DMALADR0); /* local DMA address to 0 */

  /* bytes to transfer */
  writel(2 * dev->n_pixels, dev->pci_config + PLX9056_DMASIZ0);

  /* clear DMA interrupt */
  writel(DMA0_CLEAR_INTR, dev->pci_config + PLX9056_DMACSR0); 

  /* PCI address of first descriptor, descriptor on PCI memory, not end of
     chain, interrupt after terminal count, from local to PCI */
  descriptor
    = (dev->dma.descriptors_pci_address
       + dev->dma.ptrs.write_buf * sizeof(dma_descriptor_t));

  if (single_buffer) descriptor |= 0x0B; /* in pci, end of chain, to pci */
  else descriptor |= 0x09; /* in pci, to pci */

  PDEBUG(D_START_STOP, "writing 0x%08lx to DMADPR0 (0x%08x)\n", descriptor,
         PLX9056_DMADPR0);

  writel(descriptor, dev->pci_config + PLX9056_DMADPR0);

  /* clear interrupt */
  writel(DMA0_CLEAR_INTR, dev->pci_config + PLX9056_DMACSR0);
 
  udelay(10);

  /* enable DMA */
  writel(DMA0_ENA, dev->pci_config + PLX9056_DMACSR0); 
 
  /* enable and start DMA */
  writel(DMA0_ENA | DMA0_START, dev->pci_config + PLX9056_DMACSR0);

  /* enable interrupt */
  udelay(10);
  intcsr = readl(dev->pci_config + PLX9056_INTCSR);
  intcsr |= PCI_INTR_ENA | DMA0_INTR_ENA;
  writel(intcsr, dev->pci_config + PLX9056_INTCSR);

  dev->dma.started = 1;

  PDEBUG(D_START_STOP, "PLX9056 DMA setup done\n");
}

int plx9056_dma_active(struct dev_struct *dev) {
  u32 reg_val;

  if (!dev->hardware_present) return 0;

  reg_val = readl(dev->pci_config + PLX9056_DMACSR0);

  return (reg_val & DMA0_DONE) == 0;
}

void plx9065_abort_dma(struct dev_struct *dev) {
  u32 reg_val;
  reg_val = readl(dev->pci_config + PLX9056_DMACSR0);
  reg_val = (reg_val & ~DMA0_ENA) | DMA0_ABORT;
  writel(reg_val, dev->pci_config + PLX9056_DMACSR0);
}

int plx9056_finish_dma(struct dev_struct *dev) {
  PDEBUG(D_START_STOP, "finishing plx9056 DMA\n");
  writel(DMA0_CLEAR_INTR | DMA0_ABORT, dev->pci_config + PLX9056_DMACSR0);
  udelay(10);
  dev->dma.started = 0;
  writel(0, dev->pci_config + PLX9056_INTCSR);
  writel(0, dev->pci_config + PLX9056_DMAMODE0);  
  writel(0, dev->pci_config + PLX9056_DMADPR0);  
  writel(0, dev->pci_config + PLX9056_DMACSR0);
  return 0;
}
