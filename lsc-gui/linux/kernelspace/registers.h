/* registers.h                                                                *
 *                                                                            *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _kernel_space_registers_h_
#define _kernel_space_registers_h_

#include <linux/types.h>

#include "registers-common.h"

//PCIe Addresses
typedef enum {
	PCIeAddr_devCap = 0x5C,
	PCIeAddr_devStatCtrl = 0x60
} pcie_addresses_t;


#define set_bits_s0_byte(dev, reg, bits, mask) do	      \
		iowrite8((ioread8(&dev->s0_reg->reg) & ~mask) \
			| bits, &dev->s0_reg->reg);	      \
		while (0)

#define set_bits_s0_word(dev, reg, bits, mask) do		\
		iowrite16((ioread16(&dev->s0_reg->reg) & ~mask) \
			| bits, &dev->s0_reg->reg);		\
		while (0)

#define set_bits_s0_dword(dev, reg, bits, mask) do		\
		iowrite32((ioread32(&dev->s0_reg->reg) & ~mask) \
			| bits, &dev->s0_reg->reg);		\
		while (0)

#define read_s0_byte(dev, reg) ioread8(&dev->s0_reg->reg)
#define read_s0_word(dev, reg) ioread16(&dev->s0_reg->reg)
#define read_s0_dword(dev, reg) ioread32(&dev->s0_reg->reg)

#endif  /* _kernel_space_registers_h_ */
