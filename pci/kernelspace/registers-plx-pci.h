/* registers-plx-pci.h
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

#ifndef _registers_plx_pci_h_
#define _registers_plx_pci_h_

/* camera interface register byte offsets */
#define CAMERA_DBR        0x0
#define CAMERA_CTRLA      0x4
#define CAMERA_CTRLB      0x5
#define CAMERA_CTRLC      0x6
#define CAMERA_MAGIC      0x7
#define CAMERA_XCK32      0x8 /* exposure time generator */
#define CAMERA_XCK0       0x8 /* exposure time generator */
#define CAMERA_XCK1       0x9 /* exposure time generator */
#define CAMERA_XCK2       0xA /* exposure time generator */
#define CAMERA_XCKMSB     0xB
#define CAMERA_XCKCNT     0xC
#define CAMERA_PIXREG     0x10 /* number of pixels */
#define CAMERA_FREQREG    0x12
#define CAMERA_FFFLAGS    0x13
#define CAMERA_FIFO_COUNT 0x14
#define CAMERA_VCLKCTRL   0x18
#define CAMERA_VCLKFREQ   0x1B
#define CAMERA_DAT        0x20
#define CAMERA_EC         0x24
#define CAMERA_TOR32      0x28
#define CAMERA_TOR0       0x28
#define CAMERA_TOR1       0x29
#define CAMERA_TOR2       0x2A
#define CAMERA_TOR3       0x2B
#define CAMERA_DELAYREG   0x34

/* CtrlA */
#define B_AMPLIFIER         (1<<0)
#define B_IFC               (1<<1)
#define B_ACTIVATE          (1<<2)
#define B_TRIGGER_OUT       (1<<3)
#define B_FLIP_FLOP_TRIG_IN (1<<4)
#define B_TRIG_POS_SLOPE    (1<<5)
#define B_DIR_TRIG_IN       (1<<6)
#define B_TRIG_EDGE         (1<<7)

/* CtrlB */
#define B_WAIT0        (1<<0)
#define B_WAIT1        (1<<1)
#define B_WAIT2        (1<<2)
#define B_WAIT3        (1<<3)
#define B_WAIT4        (1<<4)
#define B_DIS_ND       (1<<5)
#define B_DOUBLE_PULSE (1<<6)
#define B_NDSYM        (1<<7)

/* CtrlC */
#define B_STRIG   (1<<0)
#define B_OPT1    (1<<1)
#define B_OPT2    (1<<2)
#define B_BURST   (1<<3)
#define B_EOI     (1<<4)
#define B_EOI_CHB (1<<5)
#define B_CtrlC6  (1<<6)
#define B_CtrlC7  (1<<7)

/* XCK MSB */
#define B_RESOLUTION_NS (1<<4)
#define B_RESOLUTION_MS (1<<5)
#define B_TIMER_RESET   (1<<6)
#define B_TRIG_EXTERN   (1<<7)

/* FREQREG */
#define FREQ_MASK  0x07
#define DELAY_MASK 0x38
#define B_FREQ0      (1<<0)
#define B_FREQ1      (1<<1)
#define B_FREQ2      (1<<2)
#define B_DELAY0     (1<<3)
#define B_DELAY1     (1<<4)
#define B_DELAY2     (1<<5)
#define B_SOFT_TRIG  (1<<6)
#define B_RESET_FIFO (1<<7)

/* FFFLAGS */
#define B_FIFO_OVERFLOW     (1<<3)
#define B_FIFO_WRITE_ACTIVE (1<<4)
#define B_FIFO_FULL         (1<<5)
#define B_FIFO_EMPTY        (1<<6)
#define B_FIFO_HAVE_DATA    (1<<7)

/* exposure control */
#define B_EXPOSURE_ENABLE (1<<31)

/* TOR trigger options */
#define TRIGGER_IN_DIVIDER_MASK  0x000000FF
#define TRIGGER_OUT_DIVIDER_MASK 0x00FF0000
#define B_NO_PDA_RESET             (1<<26)
#define B_DELAY_ACTIVE             (1<<27)
#define B_FIFO_READ_ACTIVE         (1<<28)
#define B_SIGNAL_TRIGGER_IN        (1<<29)
#define B_SENSOR_ACTIVE            (1<<30)
#define B_TO_REG                   (1<<31)
#define OUT_MASK (B_DELAY_ACTIVE | B_FIFO_READ_ACTIVE | B_SIGNAL_TRIGGER_IN \
                  | B_SENSOR_ACTIVE | B_TO_REG)


#endif // _registers_plx_pci_h_

