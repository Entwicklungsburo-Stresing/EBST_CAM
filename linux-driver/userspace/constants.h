/*****************************************************************//**
 * @file		constants.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/). This program is free software; you can redistribute it and/or modify it under the terms of the LPGL-3.0 as published by the Free Software Foundation.
 *********************************************************************/

#ifndef _constants_h_
#define _constants_h_

#define MAX_PCIE_BOARDS 5

/* debugging */
#define D_PCI            0x0001
#define D_START_STOP     0x0002
#define D_READOUT        0x0004
#define D_INTERRUPT      0x0008
#define D_BUFFERS        0x0010
#define D_IOCTL          0x0020
#define D_MODULE         0x0040
#define D_MMAP           0x0080
#define D_PROC           0x0100
#define D_STATUS         0x0200

#define DEBUG_BITS       0x03FF
#define DEBUG_MASK_SHIFT 16


#endif /* _constants_h_ */
