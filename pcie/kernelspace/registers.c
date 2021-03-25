/* registers.c
 *
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include "registers.h"
#include "device.h"
#include <linux/io.h>


const char reg_names[][16] = {
	/* 0x00 */
	"DBR",
	"unused",
	"unused",
	"unused",
	/* 0x04 */
	"CTRLA",
	"CTRLB",
	"CTRLC",
	"MAGIC",
	/* 0x0x8 */
	"XCK_LL",
	"XCK_LH",
	"XCK_HL",
	"XCKMSB",
	/* 0xC */
	"XCKCNT_LL",
	"XCKCNT_LH",
	"XCKCNT_HL",
	"XCKCNT_MSB",
	/* 0x10 */
	"PIXREG_L",
	"PIXREG_H",
	"FREQREG",
	"FF_FLAGS",
	/* 0x14 */
	"FIFO_COUNT_LL",
	"FIFO_COUNT_LH",
	"FIFO_COUNT_HL",
	"FIFO_COUNT_HH",
	/* 0x18 */
	"VCLK_CTRL_LL",
	"VCLK_CTRL_LH",
	"VCLK_CTRL_HL",
	"VCLK_CTRL_HH",
	/* 0x1C */
	"magic2_LL",
	"magic2_LH",
	"magic2_HL",
	"magic2_HH",
	/* 0x20 */
	"DAT_LL",
	"DAT_LH",
	"DAT_HL",
	"DAT_HH",
	/* 0x24 */
	"EC_LL",
	"EC_LH",
	"EC_HL",
	"EC_HH",
	/* 0x28 */
	"TOR_LL",
	"TOR_LH",
	"TOR_HL",
	"TOR_HH",
	/* 0x2C */
	"AR_LL",
	"AR_LH",
	"AR_HL",
	"AR_HH",
	/* 0x30 */
	"GIO_LL",
	"GIO_LH",
	"GIO_HL",
	"GIO_HH",
	/* 0x34 */
	"EC_FINE_LL",
	"EC_FINE_LH",
	"EC_FINE_HL",
	"EC_FINE_HH",
	/* 0x38 */
	"IRQ_LL",
	"IRQ_LH",
	"IRQ_HL",
	"IRQ_HH",
	/* 0x3C */
	"VERS_LL",
	"VERS_LH",
	"VERS_HL",
	"VERS_HH",
	""
};

const char reg_names_long[][32] = {
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
	"PCIEFLAGS",
	"NOS",
	"SCANINDEX",
	"DMA_BUF_SIZE_IN_SCANS",
	"DMAS_PER_INTERRUPT",
	"BLOCKS",
	"BLOCK_INDEX",
	"CAMCNT",
	"GPX_CONTROL",
	"GPX_DATA",
	"ROI0",
	"ROI1",
	"ROI2",
	"XCKDLY",
	"ADSC",
	"LDSC",
	"BTIMER",
	"BDAT",
	"BEC",
	"BFLAGS",
	"",
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

void set_bits_s0_byte(struct dev_struct *dev, u8 address, u8 bits, u8 mask)
{
	u8 val = ioread8(dev->mapped_pci_base + 0x80 + address);
	iowrite8((val & ~mask) | (bits & mask),
		 dev->mapped_pci_base + 0x80 + address);
}

void set_bits_s0_word(struct dev_struct *dev, u8 address, u16 bits,
		      u16 mask)
{
	u16 val = ioread16(dev->mapped_pci_base + 0x80 + address);
	iowrite16((val & ~mask) | (bits & mask),
		  dev->mapped_pci_base + 0x80 + address);
}

void set_bits_s0_dword(struct dev_struct *dev, u8 address, u32 bits,
		       u32 mask)
{
	u32 val = ioread32(dev->mapped_pci_base + 0x80 + address);
	iowrite32((val & ~mask) | (bits & mask),
		  dev->mapped_pci_base + 0x80 + address);
}

u8 read_s0_byte(struct dev_struct *dev, u8 address)
{
	return ioread8(dev->mapped_pci_base + 0x80 + address);
}

u16 read_s0_word(struct dev_struct *dev, u8 address)
{
	return ioread16(dev->mapped_pci_base + 0x80 + address);
}

u32 read_s0_dword(struct dev_struct *dev, u8 address)
{
	return ioread32(dev->mapped_pci_base + 0x80 + address);
}
