/* types.h
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

#ifndef _types_h_
#define _types_h_

enum out_select_enum {
  out_xck=0, out_dat=1, out_ffxck=2, out_tin=3, out_ec=4, out_to_reg=5
};

struct board_vars {
  uint8_t  xckdelay;
  uint8_t  pclk;
  uint16_t lines;
  uint16_t vfreq;
  uint16_t delay;
};

struct lscpci_param {
  uint16_t flags;
  uint8_t nd_wait;
  uint32_t timer_resolution;
  uint8_t pclk;
  uint8_t xck_delay;
  uint16_t vertical_count;
  uint16_t vertical_frequency;
  int32_t delay_after_trigger;
  int32_t exposure_time;
  uint8_t trigger_in_divider;
  uint8_t trigger_out_divider;
  enum out_select_enum out_select;
  uint16_t fifo_delay;
};

typedef struct {
  uint8_t read_buf;
  uint8_t write_buf;
  uint16_t read_pos;
  uint16_t write_pos;
} lscpci_buffer_ptr_t;

#endif /* _types_h_ */
