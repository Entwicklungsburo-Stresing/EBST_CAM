/* registers.h                                                                *
 *                                                                            *
 * Copyright 2020 Bernhard Lang, University of Geneva
 * Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _registers_h_
#define _registers_h_

#ifdef __user_space__
# include <stdint.h>
#endif
#include <linux/types.h>

//PCIe Addresses
typedef enum {
  PCIeAddr_devCap = 0x5C,
  PCIeAddr_devStatCtrl = 0x60
} pcie_addresses_t;

typedef enum { s0=0, dma=1 } region_t;

// S0 Addresses
typedef enum {
  S0Addr_DBR        = 0x0,
  S0Addr_CTRLA      = 0x4,
  S0Addr_CTRLB      = 0x5,
  S0Addr_CTRLC      = 0x6,
  S0Addr_XCKLL      = 0x8,
  S0Addr_XCKLH      = 0x9,
  S0Addr_XCKHL      = 0xa,
  S0Addr_XCKMSB     = 0xb,
  S0Addr_XCKCNTLL   = 0xc,
  S0Addr_XCKCNTLH   = 0xd,
  S0Addr_XCKCNTHL   = 0xe,
  S0Addr_XCKCNTMSB  = 0xf,
  S0Addr_PIXREGlow  = 0x10,
  S0Addr_PIXREGhigh = 0x11,
  S0Addr_BTRIGREG   = 0x12,
  S0Addr_FF_FLAGS   = 0x13,
  S0Addr_FIFOCNT    = 0x14,
  S0Addr_VCLKCTRL   = 0x18,
  S0Addr_VCLKFREQ   = 0x1b,
  S0Addr_EBST       = 0x1C,
  S0Addr_SDAT       = 0x20,
  S0Addr_SEC        = 0x24,
  S0Addr_TOR        = 0x28,
  S0Addr_ARREG      = 0x2C,
  S0Addr_GIOREG     = 0x30,
  S0Addr_DELAYEC    = 0x34,
  S0Addr_IRQREG     = 0x38,
  S0Addr_PCI        = 0x3C,
  S0Addr_PCIEFLAGS  = 0x40,
  S0Addr_TDCCtrl    = 0x60,
  S0Addr_TDCData    = 0x64,
  S0Addr_BTIMER     = 0x80,
  S0Addr_BDAT       = 0x84,
  S0Addr_BEC        = 0x88,
  S0Addr_BSLOPE     = 0x8C
} s0_addresses_t;

#define S0_REG_SIZE sizeof(s0_t)
#define DMA_REG_SIZE sizeof(dma_reg_t)
#define COPY_BUF_SIZE (DMA_REG_SIZE > S0_REG_SIZE ? DMA_REG_SIZE : S0_REG_SIZE)

/* S0 register bits */

#define CTRLA_TSTART        7
#define CTRLA_DIR_TRIG_IN   6
#define CTRLA_SLOPE         5
#define CTRLA_BOTH_SLOPES   4
#define CTRLA_TRIG_OUT      3
#define CTRLA_XCK           2
#define CTRLA_IFC           1
#define CTRLA_VONOFF        0

#define CTRLB_SHON          0x1<<3
#define CTRLB_BTI_I         0x0<<4
#define CTRLB_BTI_S1        0x1<<4
#define CTRLB_BTI_S2        0x2<<4
#define CTRLB_BTI_S1_S2     0x3<<4
#define CTRLB_BTI_TIMER     0x4<<4
#define CTRLB_STI_I         0x0
#define CTRLB_STI_S1        0x1
#define CTRLB_STI_S2        0x2
#define CTRLB_STI_TIMER     0x4
#define CTRLB_STI_ASL       0x5

#define CTRLC_EOI_CHB       5
#define CTRLC_EOI           4
#define CTRLC_S2            3
#define CTRLC_S1            1
#define CTRLC_STRIG         0

#define XCKMSB_EXT_TRIGGER  7
#define XCKMSB_RS           6
#define XCKMSB_RES_MS       5
#define XCKMSB_RES_NS       4
#define XCKMSB_XCK27        3
#define XCKMSB_XCK26        2
#define XCKMSB_XCK25        1
#define XCKMSB_XCK24        0

#define PCIEFLAG_XCKI       0
#define PCIEFLAG_INTTRIG    1
#define PCIEFLAG_ENRSTIMERHW 2
#define PCIEFLAG_NC         3
#define PCIEFLAG_BLOCKTRIG  4
#define PCIEFLAG_MEASUREON  5
#define PCIEFLAG_BLOCKON    6

#define XCK_EC_MASK 0x0FFFFFFF
#define XCK_EXT_TRIGGER  31
#define XCK_RS           30
#define XCK_RES_MS       29
#define XCK_RES_NS       28

#define FREQ_REG_RESET_FIFO 7 // BTRIGREG
#define FREQ_REG_SW_TRIG    6

#define FF_FLAGS_VALID      7
#define FF_FLAGS_EMPTY      6
#define FF_FLAGS_FULL       5
#define FF_FLAGS_XCKI       4
#define FF_FLAGS_OVFL       3

#define TOR_TO_pos          28
#define TOR_TO_msk          (0x0F<<TOR_TO_pos)
#define TOR_RS_LEVEL        27
#define TOR_NO_RS           26
#define TOR_SENDRS          25
#define TOR_ISFFT           24
#define TOR_TOCNT           16
#define TOR_TICNT           0

#define TOR_OUT_XCK         0x00000000
#define TOR_OUT_REGO        0x10000000
#define TOR_OUT_VON         0x20000000
#define TOR_OUT_DMA_ACT     0x30000000
#define TOR_OUT_ASLS        0x40000000
#define TOR_OUT_STIMER      0x50000000
#define TOR_OUT_BTIMER      0x60000000
#define TOR_OUT_ISR_ACT     0x70000000
#define TOR_OUT_S1          0x80000000
#define TOR_OUT_S2          0x90000000
#define TOR_OUT_BON         0xA0000000
#define TOR_OUT_MEASUREON   0xB0000000
#define TOR_OUT_SDAT        0xC0000000
#define TOR_OUT_BDAT        0xD0000000
#define TOR_OUT_SSHUT       0xE0000000
#define TOR_OUT_BSHUT       0xF0000000

#define EC_ECFON            31
#define EC_NOT              30
#define EC_DIRT             29
#define EC_TS2              28
#define EC_FINE_pos         0
#define EC_FINE_msk         (0xFFFF<<EC_FINE_pos)

#define IRQ_REG_LAT_pos     0
#define IRQ_REG_LAT_msk     (0xFFFF<<IRQ_REG_LAT_pos)
#define IRQ_REG_CNT_pos     16
#define IRQ_REG_CNT_msk     (0x3FFF<<IRQ_REG_CNT_pos)
#define IRQ_REG_HWDREQ_EN   30
#define IRQ_REG_ISR_active  31

#define PCIE_XCKI           0
#define PCIE_IN_TRIG        1
#define PCIE_EN_RS_TIMER_HW 2
#define PCIE_INT_RSR        3
#define PCIE_BLOCK_TRIG     4
#define PCIE_MEASURE_ON     5
#define PCIE_LNK_UP_SPF1    26
#define PCIE_ERROR_SPF1     27
#define PCIE_LNK_UP_SPF2    28
#define PCIE_ERROR_SPF2     29
#define PCIE_LNK_UP_SPF3    30
#define PCIE_ERROR_SPF3     31

#define DMA_COUNTER_RESET   31
#define BLOCK_COUNTER_RESET 31
#define SCAN_INDEX_RESET    31
#define BLOCK_INDEX_RESET   31
#define BTIMER_START        31
#define BFLAG_SW_TRIG       2
#define BFLAG_BOTH_SLOPES   1
#define BFLAG_BSLOPE        0

/* DMA */

// DMA Addresses
enum dma_addresses {
  DmaAddr_DCSR = 0x000,
  DmaAddr_DDMACR = 0x004,
  DmaAddr_WDMATLPA = 0x008,
  DmaAddr_WDMATLPS = 0x00C,
  DmaAddr_WDMATLPC = 0x010,
  DmaAddr_WDMATLPP = 0x014,
  DmaAddr_RDMATLPP = 0x018,
  DmaAddr_RDMATLPA = 0x01C,
  DmaAddr_RDMATLPS = 0x020,
  DmaAddr_RDMATLPC = 0x024,
  //for extended S0-Space:
  DmaAddr_PCIEFLAGS = 0x40,
  DmaAddr_NOS = 0x44,
  DmaAddr_ScanIndex = 0x48,
  DmaAddr_DmaBufSizeInScans = 0x04C,    // length in scans
  DmaAddr_DMAsPerIntr = 0x050,
  DmaAddr_NOB = 0x054,
  DmaAddr_BLOCKINDEX = 0x058,
  DmaAddr_CAMCNT = 0x05C
};

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


// Cam Addresses könnten später bei unterschiedlichen cam systemen variieren
enum cam_addresses {
  maddr_cam = 0x00,
  maddr_adc = 0x01,
  maddr_ioctrl = 0x02,
  cam_adaddr_gain_led = 0x00,
  cam_adaddr_pixel = 0x01,
  cam_adaddr_trig_in = 0x02,
  cam_adaddr_ch = 0x03,
  cam_adaddr_vclk = 0x04,
  cam_adaddr_LEDoff = 0x05,
  cam_adaddr_coolTemp = 0x06,
  adc_ltc2271_regaddr_reset = 0x00,
  adc_ltc2271_regaddr_outmode = 0x02,
  adc_ltc2271_regaddr_custompattern_msb = 0x03,
  adc_ltc2271_regaddr_custompattern_lsb = 0x04,
  adc_ads5294_regaddr_reset = 0x00,
  adc_ads5294_regaddr_mode = 0x25,
  adc_ads5294_regaddr_custompattern = 0x26,
  adc_ads5294_regaddr_gain_1_to_4 = 0x2A,
  adc_ads5294_regaddr_gain_5_to_8 = 0x2B,
  adc_ads5294_regaddr_2wireMode = 0x46,
  adc_ads5294_regaddr_wordWiseOutput = 0x28,
  adc_ads5294_regaddr_ddrClkAlign = 0x42,
};

enum cam_messages {
  adc_ltc2271_msg_reset = 0x80,
  adc_ltc2271_msg_normal_mode = 0x01,
  adc_ltc2271_msg_custompattern = 0x05,
  adc_ads5294_msg_reset = 0x01,
  adc_ads5294_msg_ramp = 0x40,
  adc_ads5294_msg_custompattern = 0x10,
  adc_ads5294_msg_2wireMode = 0x8401,
  adc_ads5294_msg_wordWiseOutput = 0x80FF,
  adc_ads5294_msg_ddrClkAlign = 0x60,
};

typedef struct {
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
  uint32_t DELAYEC; // EC fine adjust
  union {
    uint32_t IRQREG;
    struct {
      uint16_t IRQ_LAT;
      uint16_t IRQ_CNT;
    } dwords;
  } IRQ;
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
} s0_t;

typedef struct {
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
} dma_reg_t;

typedef struct {
  uconst uint32_t number_of_pixels;
  uconst uint32_t number_of_cameras;
  uconst uint32_t dma_num_scans;
  uconst uint32_t dma_buf_size;
  uconst uint64_t dma_physical_start;
  uconst uint32_t io_size;
  uint32_t bytes_per_interrupt;
  volatile int32_t write_pos;
  int32_t read_pos;
} lscpcie_control_t;

extern const char reg_names[][16];
extern const char reg_names_long[][32];
extern const char dma_reg_names[][16];

#ifndef __user_space__

struct dev_struct;
void set_bits_s0_byte(struct dev_struct *dev, u8 address, u8 bits, u8 mask);
void set_bits_s0_word(struct dev_struct *dev, u8 address, u16 bits, u16 mask);
void set_bits_s0_dword(struct dev_struct *dev, u8 address, u32 bits, u32 mask);

#endif /* __user_space__ */

#endif /* _registers_h_ */
