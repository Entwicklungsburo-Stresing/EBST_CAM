/* registers.c
 *
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 * Copyright 2020 Bernhard Lang, University of Geneva
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include "registers.h"


const char reg_names[][16] = {
  "DBR",
  "unused",
  "unused",
  "unused",
  "CTRLA",
  "CTRLB",
  "CTRLC",
  "MAGIC",
  "XCK_LL",
  "XCK_LH",
  "XCK_HL",
  "XCKMSB",
  "XCKCNT_LL",
  "XCKCNT_LH",
  "XCKCNT_HL",
  "XCKCNT_MSB",
  "PIXREG_L",
  "PIXREG_H",
  "FREQREG",
  "FF_FLAGS",
  "FIFO_COUNT_LL",
  "FIFO_COUNT_LH",
  "FIFO_COUNT_HL",
  "FIFO_COUNT_HH",
  "VCLK_CTRL_LL",
  "VCLK_CTRL_LH",
  "VCLK_CTRL_HL",
  "VCLK_CTRL_HH",
  "magic2_LL",
  "magic2_LH",
  "magic2_HL",
  "magic2_HH",
  "DAT_LL",
  "DAT_LH",
  "DAT_HL",
  "DAT_HH",
  "EC_LL",
  "EC_LH",
  "EC_HL",
  "EC_HH",
  "TOR_LL",
  "TOR_LH",
  "TOR_HL",
  "TOR_HH",
  "AR_LL",
  "AR_LH",
  "AR_HL",
  "AR_HH",
  "GIO_LL",
  "GIO_LH",
  "GIO_HL",
  "GIO_HH",
  "EC_FINE_LL",
  "EC_FINE_LH",
  "EC_FINE_HL",
  "EC_FINE_HH",
  "IRQ_LL",
  "IRQ_LH",
  "IRQ_HL",
  "IRQ_HH",
  "VERS_LL",
  "VERS_LH",
  "VERS_HL",
  "VERS_HH",
  ""
};

const char reg_names_long[][16] = {
  "DBR",
  "CTRL",
  "XCK",
  "XCKCNT",
  "PIXREG_FF",
  "FIFO_COUNT",
  "VCLK_CTRL",
  "magic2",
  "DAT",
  "EC",
  "TOR",
  "AR",
  "GIO",
  "EC_FINE",
  "IRQ",
  "VERS",
  ""
};

const char dma_reg_names[][16] = {
  "DCSR",
  "DDMACR",
  "WDMATLPA",
  "WDMATLPS",
  "WDMATLPC",
  "WDMATLPP",
  "RDMATLPP",
  "RDMATLPA",
  "RDMATLPS",
  "RDMATLPC",
  "WDMAPERF",
  "RDMAPERF",
  "RDMASTAT",
  "NRDCOMP",
  "RCOMPDSTZW",
  "DLWSTAT",
  "DLTRSTAT",
  "DMISCOUNT",
  ""
};