/* register-names-pci-plx.c
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

#include "register-names.h"

const char reg_names[][16] = {
  "CTRLA",
  "CTRLB",
  "CTRLC",
  "MAGIC",
  "XCK0",
  "XCK1",
  "XCK2",
  "XCKMSB",
  "XCKCNT0",
  "XCKCNT1",
  "XCKCNT2",
  "XCKCNT3",
  "PIXREG0",
  "PIXREG1",
  "FREQREG",
  "FFFLAGS",
  "FIFO_COUNT",
  "unused",
  "unused",
  "unused",
  "VCLKCNT0",
  "VCLKCNT1",
  "unused",
  "VCLKFREQ",
  "unused",
  "unused",
  "unused",
  "unused",
  "DAT0",
  "DAT1",
  "DAT2",
  "DAT3",
  "EC0",
  "EC1",
  "EC2",
  "EC3",
  "TOR0",
  "TOR1",
  "TOR2",
  "TOR3",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "unused",
  "DELAYREG0",
  "DELAYREG1",
  "unused",
  "unused",
  "IRQLAT0",
  "IRQLAT1",
  "IRQCNT0",
  "IRQCNT1",
  "version",
  "unused",
  "unused",
  "unused"
};
