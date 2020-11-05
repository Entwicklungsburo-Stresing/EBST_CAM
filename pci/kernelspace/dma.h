/* dma.h
 *
 * Copyright (C) 2010-2020 Bernhard Lang, University of Geneva
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

#ifndef _dma_h_
#define _dma_h_

#ifdef __plx_pci__
# include "plx9056.h"
#endif
#ifdef __epci__
//# include ".h"
#endif

#include <linux/types.h>

typedef struct {
  uint8_t read_buf;
  uint8_t write_buf;
  uint16_t read_pos;
  uint16_t write_pos;
} buffer_ptr_t;

typedef struct {
  u8 n_buffers;
  u16 buf_size;
  u8 **buffers;
  buffer_ptr_t ptrs;
  u8 *descriptor_buffer;
  dma_descriptor_t *descriptors;
  u32 descriptors_pci_address;
  int started;
} dma_buffer_t;

struct dev_struct;
struct device;

int init_dma(struct dev_struct *dev);
int prepare_dma(struct dev_struct *dev);
int finish_dma(struct dev_struct *dev);
void clean_up_dma(struct dev_struct *dev);

#endif /* _dma_h_ */
