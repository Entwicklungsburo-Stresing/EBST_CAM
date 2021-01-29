/* constants.h
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

#ifndef _lscpci_constants_h_
#define _lscpci_constants_h_

/* state bits */
#define CAMERA_OFF            0x00
#define CAMERA_ON             0x01
#define CAMERA_ACQUIRING      0x02
#define CAMERA_STOP           0x04
#define CAMERA_DMA_ACTIVE     0x08
#define CAMERA_BUFFER_FULL    0x10
#define CAMERA_FIFO_OVERFLOW  0x80

/* values for flags */
#define AMPLIFIER_ON    0x0001
#define TRIG_POS        0x0002
#define TRIG_EDGE       0x0004
#define NDSYM           0x0008
#define DOUBLE_PULSE    0x0010
#define DIVIDER_LOW     0x0020
#define DIVIDER_HIGH    0x0040
#define NO_PDA_RESET    0x0080
#define OPTO1           0x0100
#define OPTO2           0x0200
#define BURST_MODE      0x0400

/* FIFO bits */
#define FIFO_OVERFLOW     0x40

/* debugging */
#define D_START_STOP 0x0001
#define D_BUFFERS    0x0002
#define D_INTERRUPT  0x0004
#define D_READOUT    0x0008
#define D_DATA       0x0010
#define D_IOCTL      0x0020
#define D_PLX        0x0040
#define D_MODULE     0x0080
#define D_BUFSPACE   0x0100
#define D_POLLING    0x0200

#endif /* _lscpci_constants_h_ */
