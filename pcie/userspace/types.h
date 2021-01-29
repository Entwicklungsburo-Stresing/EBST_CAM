/*
 * types.h
 *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _types_h_
#define _types_h_

#include <stdint.h>

typedef enum {
  out_xck=0, out_dat=1, out_ffxck=2, out_tin=3, out_ec=4, out_to_reg=5
} out_select_t;

typedef struct {
  uint8_t  xckdelay;
  uint8_t  pclk;
  uint16_t lines;
  uint16_t vfreq;
  uint16_t delay;
} board_vars_t;

typedef struct {
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
  //-->>enum out_select_enum out_select;
  uint16_t fifo_delay;
} lscpcie_params_t;

#endif /* _types_h_ */
