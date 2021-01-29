/* plx9056.h
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

#ifndef _plx9056_h_
#define _plx9056_h_

#include <linux/pci.h>

#define VENDOR_ID  PCI_VENDOR_ID_PLX
#define DEVICE_ID  PCI_DEVICE_ID_PLX_9056

#define NUM_PLX_REGISTERS 128
#define NUM_CAMERA_REGISTERS 64

/* PCI9056 pci configuration register byte offsets */
#define PLX9056_PCIIDR     0x0
#define PLX9056_PCICR      0x4
#define PLX9056_PCISVID   0x2C
#define PLX9056_PCISID    0x2E
#define PLX9056_PCIMGR    0x3E

/* PCI9056 local configuration register byte offsets */
#define PLX9056_INTCSR    0x68
#define PLX9056_DMAMODE0  0x80
#define PLX9056_DMAPADR0  0x84
#define PLX9056_DMALADR0  0x88
#define PLX9056_DMASIZ0   0x8C
#define PLX9056_DMADPR0   0x90
#define PLX9056_DMACSR0   0xA8

/* INTCSR */
#define PCI_INTR_ENA      (1<<8)
#define LOCAL_INTR_ENA    (1<<11)
#define LOCAL_INTR_ACTIVE (1<<15)
#define DMA0_INTR_ENA     (1<<18)
#define DMA0_INTR_ACTIVE  (1<<21)

/* DMAMODE0 */
#define DMA0_8BIT             0
#define DMA0_16BIT            1
#define DMA0_32BIT            3
#define DMA0_INPUT_ENA        (1<<6)
#define DMA0_CONTINUOUS_BURST (1<<7)
#define DMA0_LOCAL_BURST_ENA  (1<<8)
#define DMA0_SG               (1<<9)
#define INT_ON_DMA0_DONE      (1<<10)
#define DMA0_LOCAL_CONST      (1<<11)
#define DMA0_DEMAND_MODE      (1<<12)
#define DMA0_INTERRUPT_TO_PCI (1<<17)

/* DMACSR0 */
#define DMA0_ENA         (1<<0)
#define DMA0_START       (1<<1)
#define DMA0_ABORT       (1<<2)
#define DMA0_CLEAR_INTR  (1<<3)
#define DMA0_DONE        (1<<4)

typedef struct {
  u32 pci_address;
  u32 local_address;
  u32 byte_count;
  u32 next_descriptor;
} dma_descriptor_t;

struct dev_struct;

int probe_plx9056(struct pci_dev *dev, const struct pci_device_id *id);
void remove_plx9056(struct pci_dev *dev);
void plx9056_start_dma(struct dev_struct *dev, int single_buffer);
int plx9056_dma_active(struct dev_struct *dev);
void plx9065_abort_dma(struct dev_struct *dev);
int plx9056_finish_dma(struct dev_struct *dev);


#endif /* _plx9056_h_ */
