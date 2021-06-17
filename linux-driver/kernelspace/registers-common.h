/* registers.h                                                                *
 *                                                                            *
 * Copyright 2020-2021 Bernhard Lang, University of Geneva
 * Copyright 2020-2021 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _registers_h_
#define _registers_h_

#ifdef __user_space__
# include <stdint.h>
#endif

#include "../../shared_src/enum.h"

/* device status bits relevant in user space */
#define DEV_HARDWARE_PRESENT 0x2000
#define DEV_FIFO_OVERFLOW    0x4000
#define DEV_DMA_OVERFLOW     0x8000

/* S0 register bits */

#define CTRLA_TSTART         7
#define CTRLA_DIR_TRIG_IN    6
#define CTRLA_SLOPE          5
#define CTRLA_BOTH_SLOPES    4
#define CTRLA_TRIG_OUT       3
#define CTRLA_XCK            2
#define CTRLA_IFC            1
#define CTRLA_VONOFF         0

#define CTRLB_SHON           (0x1<<3)
#define CTRLB_GTI_Pos        4
#define CTRLB_GTI_Mask       (0x03<<CTRLB_GTI_Pos)
#define CTRLB_GTI_I          (0x00<<CTRLB_GTI_Pos)
#define CTRLB_GTI_S1         (0x01<<CTRLB_GTI_Pos)
#define CTRLB_GTI_S2         (0x02<<CTRLB_GTI_Pos)
#define CTRLB_GTI_STOP       (0x03<<CTRLB_GTI_Pos)
#define CTRLB_STI_Mask       0x07
#define CTRLB_STI_TSTART     0x00
#define CTRLB_STI_AUTO_LINE  0x01
#define CTRLB_STI_I          0x04
#define CTRLB_STI_S1         0x05
#define CTRLB_STI_S2         0x06

#define CTRLC_EOI_CHB        5
#define CTRLC_EOI            4
#define CTRLC_S2             3
#define CTRLC_S1             1
#define CTRLC_STRIG          0

#define XCKMSB_EXT_TRIGGER   7
#define XCKMSB_RS            6
#define XCKMSB_RES_MS        5
#define XCKMSB_RES_NS        4
#define XCKMSB_XCK27         3
#define XCKMSB_XCK26         2
#define XCKMSB_XCK25         1
#define XCKMSB_XCK24         0

#define PCIEFLAG_XCKI        0
#define PCIEFLAG_INTTRIG     1
#define PCIEFLAG_ENRSTIMERHW 2
#define PCIEFLAG_NC          3
#define PCIEFLAG_BLOCKTRIG   4
#define PCIEFLAG_MEASUREON   5
#define PCIEFLAG_BLOCKON     6

#define XCK_EC_MASK 0x0FFFFFFF
#define XCK_EXT_TRIGGER      31
#define XCK_RS               30
#define XCK_RES_MS           29
#define XCK_RES_NS           28

#define FREQ_REG_RESET_FIFO  7	// BTRIGREG
#define FREQ_REG_SW_TRIG     6

#define FF_FLAGS_VALID       7
#define FF_FLAGS_EMPTY       6
#define FF_FLAGS_FULL        5
#define FF_FLAGS_XCKI        4
#define FF_FLAGS_OVFL        3

#define TOR_TO_pos           28
#define TOR_TO_msk           (0x0F<<TOR_TO_pos)
#define TOR_RS_LEVEL         27
#define TOR_NO_RS            26
#define TOR_SENDRS           25
#define TOR_ISFFT            24
#define TOR_TOCNT            16
#define TOR_TICNT            0

#define TOR_OUT_XCK          0x00000000
#define TOR_OUT_REGO         0x10000000
#define TOR_OUT_VON          0x20000000
#define TOR_OUT_DMA_ACT      0x30000000
#define TOR_OUT_ASLS         0x40000000
#define TOR_OUT_STIMER       0x50000000
#define TOR_OUT_BTIMER       0x60000000
#define TOR_OUT_ISR_ACT      0x70000000
#define TOR_OUT_S1           0x80000000
#define TOR_OUT_S2           0x90000000
#define TOR_OUT_BON          0xA0000000
#define TOR_OUT_MEASUREON    0xB0000000
#define TOR_OUT_SDAT         0xC0000000
#define TOR_OUT_BDAT         0xD0000000
#define TOR_OUT_SSHUT        0xE0000000
#define TOR_OUT_BSHUT        0xF0000000

#define EC_ECFON             31
#define EC_NOT               30
#define EC_DIRT              29
#define EC_TS2               28
#define EC_FINE_pos          0
#define EC_FINE_msk          (0xFFFF<<EC_FINE_pos)

#define IRQ_REG_LAT_po s     0
#define IRQ_REG_LAT_msk      (0xFFFF<<IRQ_REG_LAT_pos)
#define IRQ_REG_CNT_pos      16
#define IRQ_REG_CNT_msk      (0x3FFF<<IRQ_REG_CNT_pos)
#define IRQ_REG_HWDREQ_EN    30
#define IRQ_REG_ISR_active   31

#define PCIE_XCKI            0
#define PCIE_IN_TRIG         1
#define PCIE_EN_RS_TIMER_HW  2
#define PCIE_INT_RSR         3
#define PCIE_BLOCK_TRIG      4
#define PCIE_MEASURE_ON      5
#define PCIE_LNK_UP_SPF1     26
#define PCIE_ERROR_SPF1      27
#define PCIE_LNK_UP_SPF2     28
#define PCIE_ERROR_SPF2      29
#define PCIE_LNK_UP_SPF3     30
#define PCIE_ERROR_SPF3      31

#define DMA_COUNTER_RESET    31
#define BLOCK_COUNTER_RESET  31
#define SCAN_INDEX_RESET     31
#define BLOCK_INDEX_RESET    31
#define BTIMER_START         31
#define BFLAG_SW_TRIG        2
#define BFLAG_BOTH_SLOPES    1
#define BFLAG_BSLOPE         0

#define DCSR_RESET                     0
#define DCSR_VERS_Pos                  8
#define DCSR_VERS_Msk                  (0xFF<<DCSR_VERS_Pos)
#define DSCR_DATA_WIDTH_Pos            16
#define DSCR_DATA_WIDTH_Msk            (0x07<<DSCR_DATA_WIDTH_Pos)
#define DSCR_XILINX_FAM_Pos            24
#define DSCR_XILINX_FAM_Msk            (0xFF<<DSCR_XILINX_FAM_Pos)

#define DDMACR_START_DMA_WRT           0
#define DDMACR_RELAXED_ORDER_WRT       1
#define DDMACR_NO_SNOOP_WRT            2
#define DDMACR_DMA_DONE_IRQ_DISABLE    3
#define DDMACR_DMA_WRT_DONE            8
#define DDMACR_DMA_START_RD            16
#define DDMACR_RELAXED_ORDER_RD        17
#define DDMACR_NO_SNOOP_RD             18
#define DDMACR_DMA_RD_DONE             24
#define DDMACR_DMA_RD_ERROR            31

#define WDMATLPS_TLP_PAYLOAD_pos       0
#define WDMATLPS_TLP_PAYLOAD_msk       0x00000FFF
#define WDMATLPS_TLP_CLASS_pos         16
#define WDMATLPS_TLP_CLASS_msk         (0x03 << WDMATLPS_TLP_CLASS_pos)
#define WDMATLPS_TLP_64_EN             19
#define WDMATLPS_UPPER_TLP_pos         24
#define WDMATLPS_UPPER_TLP_msk         (0xFF<<WDMATLPS_UPPER_TLP_pos)

#define DLWSTAT_MAX_LINK_CAPACITY_pos  0
#define DLWSTAT_MAX_LINK_CAPACITY_msk  0x1F
#define DLWSTAT_NEG_LINK_WIDTH_pos     8
#define DLWSTAT_NEG_LINK_WIDTH_msk     (0x3F<<DLWSTAT_NEG_LINK_WIDTH_pos)

#define DLTRSSTAT_MAX_PAYLOAD_pos      0
#define DLTRSSTAT_MAX_PAYLOAD          0x07
#define DLTRSSTAT_PROG_MAX_PAYLOAD_pos 8
#define DLTRSSTAT_PROG_MAX_PAYLOAD_msk (0x07<<DLTRSSTAT_PROG_MAX_PAYLOAD_pos)
#define DLTRSSTAT_MAX_RD_REQ_pos       16
#define DLTRSSTAT_MAX_RD_REQ_msk       (0x07<<DLTRSSTAT_MAX_RD_REQ_pos)

struct s0_reg_struct {
	uint32_t DBR;
	uint8_t CTRLA;
	uint8_t CTRLB;
	uint8_t CTRLC;
	uint8_t MAGIC;
	union {
		uint32_t dword;
		struct {
			uint8_t LL;
			uint8_t LH;
			uint8_t HL;
			uint8_t MSB;
		} bytes;
	} XCK;
	uint32_t XCKCNT;
	uint16_t PIXREG;
	uint8_t BTRIGREG;
	uint8_t FF_FLAGS;
	uint32_t FIFOCNT;
	uint32_t VCLKCTRL;
	uint32_t EBST;
	uint32_t DAT;
	uint32_t EC;
	uint32_t TOR;
	uint32_t ARREG;
	uint32_t GIOREG;
	uint32_t DELAYEC;	// EC fine adjust
	union {
		uint32_t REG32;
		struct {
			uint16_t LAT;
			uint16_t CNT;
		} dwords;
	} IRQREG;
	uint32_t PCIE_VERS;
	uint32_t PCIEFLAGS;
	uint32_t NUMBER_OF_SCANS;
	uint32_t SCAN_INDEX;
	uint32_t DMA_BUF_SIZE_IN_SCANS;
	uint32_t DMAS_PER_INTERRUPT;
	uint32_t NUMBER_OF_BLOCKS;
	uint32_t BLOCK_INDEX;
	uint32_t CAM_CNT;
	uint32_t GPX_Control;
	uint32_t GPX_Data;
	uint32_t ROI0;
	uint32_t ROI1;
	uint32_t ROI2;
	uint32_t XCKDLY;
	uint32_t ADSC;
	uint32_t LDSC;
	uint32_t BTIMER;
	uint32_t BDAT;
	uint32_t BEC;
	uint32_t BFLAGS;
};

struct dma_reg_struct {
	uint32_t DCSR;
	uint32_t DDMACR;
	uint32_t WDMATLPA;
	uint32_t WDMATLPS;
	uint16_t WDMATLPC;
	uint16_t reserved1;
	uint32_t WDMATLPP;
	uint32_t RDMATLPP;
	uint32_t RDMATLPA;
	uint16_t RDMATLPC;
	uint16_t reserved2;
	uint32_t WDMAPERF;
	uint32_t RDMAPERF;
	uint32_t NRDCOMP;
	uint32_t RCOMPDSTZW;
	uint32_t DLWSTAT;
	uint32_t DLTRSSTAT;
	uint32_t DMISCOUNT;
};

struct control_struct {
	uconst uint32_t number_of_pixels;
	uconst uint32_t number_of_cameras;
	uconst uint32_t dma_num_scans;
	uconst uint32_t dma_buf_size;
	uconst uint64_t dma_physical_start;
	uconst uint32_t io_size;
	uint32_t bytes_per_interrupt;
	uint32_t used_dma_size;
	volatile int32_t write_pos;
	int32_t read_pos;
	uint32_t irq_count;
	int32_t stimer_val;
	uint16_t debug_mode;
	uint16_t status;
};

extern const char reg_names[][16];
extern const char reg_names_long[][32];
extern const char dma_reg_names[][16];


#endif  /* _registers_h_ */
