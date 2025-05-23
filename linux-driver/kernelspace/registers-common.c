/*****************************************************************//**
 * @file		registers-common.c
 * @author		Bernhard Lang
 * @date		05.11.2020
 * @copyright	Copyright 2020-2021 Bernhard Lang, University of Geneva, Entwicklungsbuero Stresing (http://www.stresing.de/). This file is dual licensed. As part of the the userspace library lscpcie and libESLSCLDLL.so is is licensed under the LPGL-3.0. As part of the kernel module lscpcie.ko it is licensed under the GPL-2.0.
 *********************************************************************/


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
