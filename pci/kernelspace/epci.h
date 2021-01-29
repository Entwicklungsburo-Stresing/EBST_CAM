/* plx9056.h
 *
 * Copyright (C) 2020 Bernhard Lang, University of Geneva
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

#ifndef _epci_h_
#define _epci_h_

#include <linux/pci.h>

//#define VENDOR_ID  PCI_VENDOR_ID_PLX
//#define DEVICE_ID  PCI_DEVICE_ID_PLX_9056


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


#endif /* _epci_h_ */
