/*****************************************************************//**
 * @file		registers.h
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/). This file is dual licensed. As part of the the userspace library lscpcie and libESLSCLDLL.so is is licensed under the LPGL-3.0. As part of the kernel module lscpcie.ko it is licensed under the GPL-2.0.
 *********************************************************************/

#ifndef _kernel_space_registers_h_
#define _kernel_space_registers_h_

#include <linux/types.h>

#include "registers-common.h"

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
