/* registers-epci.h
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

#ifndef _registers_epci_h_
#define _registers_epci_h_

/* addresses */

#define DBR                   0x00
#define CTRLA                 0x04
#define CTRLB                 0x05
#define CTRLC                 0x06
#define MAGIC                 0x07
#define XCK_LL                0x08
#define XCK_LH                0x09
#define XCK_HL                0x0A
#define XCK_HH                0x0B
#define XCKCNT_LL             0x0C
#define XCKCNT_LH             0x0D
#define XCKCNT_HL             0x0E
#define XCKCNT_HH             0x0F
#define PIXREG_L              0x10
#define PIXREG_H              0x11
#define FREQREG               0x12
#define FF_FLAGS              0x13

#define CTRL                  0x01
#define XCK                   0x02
#define XCKCN                 0x03
#define PIXREG                0x04
#define FIFO                  0x05
#define VCLCK_CTRL            0x06
#define MAGIC2                0x07
#define DAT                   0x08
#define EC                    0x09
#define TOR                   0x0A
#define AR                    0x0B
#define GIO                   0x0C
#define EC_FINE               0x0D
#define IRQ                   0x0E
#define VERS                  0x0F

#define PCIEFLAGS             0x10
#define NOS                   0x20
#define SCAN_INDEX            0x30
#define DMA_BUF_SIZE_IN_SCANS 0x40
#define DMA_SPER_INTERRUPT    0x50
#define BLOCKS                0x60
#define BLOCK_INDEX           0x70
#define CAMCNT                0x80
#define GPX_CONTROL           0x90
#define GPX_DATA              0xA0
#define ROI0                  0xB0
#define ROI1                  0xC0
#define ROI2                  0xD0
#define TRIG_CNT              0xE0
#define XDLY                  0xF0

/* bits */

#define CTRLA_TSTART_pos                7
#define CTRLA_TSTART                    (1<<CTRLA_TSTART_pos)
#define CTRLA_DIR_TRIG_IN_pos           6
#define CTRLA_DIR_TRIG_IN               (1<<CTRLA_DIR_TRIG_IN_pos)
#define CTRLA_SLOPE_pos                 5
#define CTRLA_SLOPE                     (1<<CTRLA_SLOPE_pos)
#define CTRLA_BOTH_SLOPE_pos            4
#define CTRLA_BOTH_SLOPE                (1<<CTRLA_BOTH_SLOPE_pos)
#define CTRLA_TRIG_OUT_pos              3
#define CTRLA_TRIG_OUT                  (1<<CTRLA_TRIG_OUT_pos)
#define CTRLA_XCK_pos                   2
#define CTRLA_XCK                       (1<<CTRLA_XCK_pos)
#define CTRLA_IFC_pos                   1
#define CTRLA_IFC                       (1<<CTRLA_IFC_pos)
#define CTRLA_VONOFF_pos                0
#define CTRLA_VONOFF                    (1<<CTRLA_VONOFF_pos)

#define CTRLB_GTI1_pos                  5
#define CTRLB_GTI1                      (1<<CTRLB_GTI1_pos)
#define CTRLB_GTI0_pos                  4
#define CTRLB_GTI0                      (1<<CTRLB_GTI0_pos)
#define CTRLB_SHON_pos                  3
#define CTRLB_SHON                      (1<<CTRLB_SHON_pos)
#define CTRLB_STI2_pos                  2
#define CTRLB_STI2                      (1<<CTRLB_STI2_pos)
#define CTRLB_STI1_pos                  1
#define CTRLB_STI1                      (1<<CTRLB_STI1_pos)
#define CTRLB_STI0_pos                  0
#define CTRLB_STI0                      (1<<CTRLB_STI0_pos)

#define CTRLC_EOI_CHB_pos               5
#define CTRLC_EOI_CHB                   (1<<CTRLC_EOI_CHB_pos)
#define CTRLC_EOI_pos                   4
#define CTRLC_EOI                       (1<<CTRLC_EOI_pos)
#define CTRLC_S2_pos                    3
#define CTRLC_S2                        (1<<CTRLC_S2_pos)
#define CTRLC_S1_pos                    1
#define CTRLC_S1                        (1<<CTRLC_S1_pos)
#define CTRLC_STRIG_pos                 0
#define CTRLC_STRIG                     (1<<CTRLC_STRIG_pos)

#define XCBMSB_F_EXT_TRIG_pos           7
#define XCBMSB_F_EXT_TRIG               (1<<XCBMSB_F_EXT_TRIG_pos)
#define XCBMSB_RS_pos                   6
#define XCBMSB_RS                       (1<<XCBMSB_RS_pos)
#define XCBMSB_RES_MS_pos               5
#define XCBMSB_RES_MS                   (1<<XCBMSB_RES_MS_pos)
#define XCBMSB_RES_NS_pos               4
#define XCBMSB_RES_NS                   (1<<XCBMSB_RES_NS_pos)
#define XCBMSB_XCK27_pos                3
#define XCBMSB_XCK27                    (1<<XCBMSB_XCK27_pos)
#define XCBMSB_XCK26_pos                2
#define XCBMSB_XCK26                    (1<<XCBMSB_XCK26_pos)
#define XCBMSB_XCK25_pos                1
#define XCBMSB_XCK25                    (1<<XCBMSB_XCK25_pos)
#define XCBMSB_XCK24_pos                0
#define XCBMSB_XCK24                    (1<<XCBMSB_XCK24_pos)

#define FREQ_REG_RS_FF_pos              7
#define FREQ_REG_RS_FF                  (1<<FREQ_REG_RS_FF_pos)
#define FREG_REG_SW_TRIG_pos            6
#define FREG_REG_SW_TRIG                (1<<FREG_REG_SW_TRIG_pos)

#define FF_FLAGS_VALID_pos              7
#define FF_FLAGS_VALID                  (1<<FF_FLAGS_VALID_pos)
#define FF_FLAGS_EF_pos                 6
#define FF_FLAGS_EF                     (1<<FF_FLAGS_EF_pos)
#define FF_FLAGS_FF_pos                 5
#define FF_FLAGS_FF                     (1<<FF_FLAGS_FF_pos)
#define FF_FLAGS_XCKI_pos               4
#define FF_FLAGS_XCKI                   (1<<FF_FLAGS_XCKI_pos)
#define FF_FLAGS_OVFL_pos               3
#define FF_FLAGS_OVFL                   (1<<FF_FLAGS_OVFL_pos)

#define TOR_TO3_pos                     31
#define TOR_TO3                         (1<<TOR_TO3_pos)
#define TOR_TO2_pos                     30
#define TOR_TO2                         (1<<TOR_TO2_pos)
#define TOR_TO1_pos                     29
#define TOR_TO1                         (1<<TOR_TO1_pos)
#define TOR_TO0_pos                     28
#define TOR_TO0                         (1<<TOR_TO0_pos)
#define TOR_RS_LEVEL_pos                27
#define TOR_RS_LEVEL                    (1<<TOR_RS_LEVEL_pos)
#define TOR_NO_RS_pos                   26
#define TOR_NO_RS                       (1<<TOR_NO_RS_pos)
#define TOR_SENDRS_pos                  25
#define TOR_SENDRS                      (1<<TOR_SENDRS_pos)
#define TOR_ISFFT_pos                   24
#define TOR_ISFFT                       (1<<TOR_ISFFT_pos)

#define TOR_TOCNT_pos                   16
#define TOR_TOCNT_mask                  (0xFF<<TOR_TOCNT_pos)

#define TOR_TICNT_pos                   0
#define TOR_TICNT_mask                  (0xFF<<TOR_TICNT_pos)

#define TOR_TO_pos                      28
#define TOR_TO_msk                      (0x0F<<TOR_TO_pos)
#define TOR_TO_XCK                      (0x00<<TOR_TO_pos)
#define TOR_TO_REGO                     (0x10<<TOR_TO_pos)
#define TOR_TO_CNTO                     (0x20<<TOR_TO_pos)
#define TOR_TO_RSMON                    (0x30<<TOR_TO_pos)
#define TOR_TO_DMAO                     (0x40<<TOR_TO_pos)
#define TOR_TO_INTTROGO                 (0x50<<TOR_TO_pos)
#define TOR_TO_DATO                     (0x60<<TOR_TO_pos)
#define TOR_TO_BTRIGO                   (0x70<<TOR_TO_pos)
#define TOR_TO_INTSRO                   (0x80<<TOR_TO_pos)
#define TOR_TO_OPT1                     (0x90<<TOR_TO_pos)
#define TOR_TO_OPT2                     (0xA0<<TOR_TO_pos)
#define TOR_TO_BLOCKON                  (0xB0<<TOR_TO_pos)
#define TOR_TO_MEASUREON                (0xC0<<TOR_TO_pos)
#define TOR_TO_XCKDLYON                 (0xD0<<TOR_TO_pos)
#define TOR_TO_VON                      (0xE0<<TOR_TO_pos)
#define TOR_TO_MSHUT                    (0xF0<<TOR_TO_pos)

#define EC_ECFON_pos                    31
#define EC_ECFON                        (1<<EC_ECFON_pos)
#define EC_NOT_pos                      30
#define EC_NOT                          (1<<EC_NOT_pos)
#define EC_DIRT_pos                     29
#define EC_DIRT                         (1<<EC_DIRT_pos)
#define EC_TS2_pos                      28
#define EC_TS2                          (1<<EC_TS2_pos)
#define EC_FINE_pos                     0
#define EC_FINE_msk                     (0xFFFF<<EC_FINE_pos)

#define IRQ_REG_LAT_pos                 0
#define IRQ_REG_LAT_msk                 (0xFFFF<<IRQ_REG_LAT_pos)
#define IRQ_REG_CNT_pos                 16
#define IRQ_REG_CNT_msk                 (0x3FFF<<IRQ_REG_CNT_pos)
#define IRQ_REG_HWDREQ_EN_pos           30
#define IRQ_REG_HWDREQ_EN               (1<<IRQ_REG_HWDREQ_EN_pos)

#define PCIE_XCKI_pos                   0
#define PCIE_XCKI                       (1<<PCIE_XCKI_pos)
#define PCIE_IN_TRIG_pos                1
#define PCIE_IN_TRIG                    (1<<PCIE_IN_TRIG_pos)
#define PCIE_EN_RS_TIMER_HW_pos         3
#define PCIE_EN_RS_TIMER_HW             (1<<PCIE_EN_RS_TIMER_HW_pos)
#define PCIE_INT_RSR_pos                4
#define PCIE_INT_RSR                    (1<<PCIE_INT_RSR_pos)
#define PCIE_BLOCK_TRIG_pos             5
#define PCIE_BLOCK_TRIG                 (1<<PCIE_BLOCK_TRIG_pos)
#define PCIE_MEASURE_ON_pos             6
#define PCIE_MEASURE_ON                 (1<<PCIE_MEASURE_ON_pos)
#define PCIE_LNK_UP_SPF1_pos            26
#define PCIE_LNK_UP_SPF1                (1<<PCIE_LNK_UP_SPF1_pos)
#define PCIE_ERROR_SPF1_pos             27
#define PCIE_ERROR_SPF1                 (1<<PCIE_ERROR_SPF1_pos)
#define PCIE_LNK_UP_SPF2_pos            28
#define PCIE_LNK_UP_SPF2                (1<<PCIE_LNK_UP_SPF2_pos)
#define PCIE_ERROR_SPF2_pos             29
#define PCIE_ERROR_SPF2                 (1<<PCIE_ERROR_SPF2_pos)
#define PCIE_LNK_UP_SPF3_pos            30
#define PCIE_LNK_UP_SPF3                (1<<PCIE_LNK_UP_SPF3_pos)
#define PCIE_ERROR_SPF3_pos             31
#define PCIE_ERROR_SPF3                 (1<<PCIE_ERROR_SPF3_pos)

#define SCAN_INDEX_RESET_pos            31
#define SCAN_INDEX_RESET                (1<<SCAN_INDEX_RESET_pos)

/* DMA */

#define DCSR                 0x0000
#define DDMACR               0x0004
#define WDMATLPA             0x0008
#define WDMATLPS             0x000C
#define WDMATLPC             0x0010
#define WDMATLPP             0x0014
#define RDMATLPP             0x0018
#define RDMATLPA             0x001C
#define RDMATLPS             0x0020
#define RDMATLPC             0x0024
#define WDMAPERF             0x0028
#define RDMAPERF             0x002C
#define NRDCOMP              0x0030
#define RCOMPDSTZW           0x0034
#define DLWSTAT              0x0038
#define DLTRSSTAT            0x003C
#define DMISCCONT            0x0040

#define DCSR_RESET                      0x01
#define DCSR_VERS_Pos                   8
#define DCSR_VERS_Msk                   (0xFF<<DCSR_VERS_Pos)
#define DSCR_DATA_WIDTH_Pos             16
#define DSCR_DATA_WIDTH_Msk             (0x07<<DSCR_DATA_WIDTH_Pos)
#define DSCR_XILINX_FAM_Pos             24
#define DSCR_XILINX_FAM_Msk             (0xFF<<DSCR_XILINX_FAM_Pos)

#define DDMACR_START_DMA_WRT_pos        0
#define DDMACR_START_DMA_WRT            (1<<DDMACR_START_DMA_WRT_pos)
#define DDMACR_RELAXED_ORDER_WRT_pos    1
#define DDMACR_RELAXED_ORDER_WRT        (1<<DDMACR_RELAXED_ORDER_WRT_pos)
#define DDMACR_NO_SNOOP_WRT_pos         2
#define DDMACR_NO_SNOOP_WRT             (1<<DDMACR_NO_SNOOP_WRT_pos)
#define DDMACR_DMA_DONE_IRQ_DISABLE_pos 3
#define DDMACR_DMA_DONE_IRQ_DISABLE     (1<<DDMACR_DMA_DONE_IRQ_DISABLE_pos)
#define DDMACR_DMA_WRT_DONE_pos         8
#define DDMACR_DMA_WRT_DONE             (1<<DDMACR_DMA_WRT_DONE_pos)
#define DDMACR_DMA_START_RD_pos         16
#define DDMACR_DMA_START_RD             (1<<DDMACR_DMA_START_RD_pos)
#define DDMACR_RELAXED_ORDER_RD_pos     17
#define DDMACR_RELAXED_ORDER_RD         (1<<DDMACR_RELAXED_ORDER_RD_pos)
#define DDMACR_NO_SNOOP_RD_pos          18
#define DDMACR_NO_SNOOP_RD              (1<<DDMACR_NO_SNOOP_RD_pos)
#define DDMACR_DMA_WRT_DONE_pos         24
#define DDMACR_DMA_WRT_DONE             (1<<DDMACR_DMA_WRT_DONE_pos)
#define DDMACR_DMA_RD_ERROR_pos         31
#define DDMACR_DMA_RD_ERROR             (1<<DDMACR_DMA_RD_ERROR_pos)

#define WDMATLPS_TLP_PAYLOAD_pos        0
#define WDMATLPS_TLP_PAYLOAD_msk        0x00000FFF
#define WDMATLPS_TLP_CLASS_pos          16
#define WDMATLPS_TLP_CLASS_msk          (0x03 << WDMATLPS_TLP_CLASS_pos)
#define WDMATLPS_TLP_64_EN_pos          19
#define WDMATLPS_TLP_64_EN_msk          (1<<WDMATLPS_TLP_64_EN_pos)
#define WDMATLPS_UPPER_TLP_pos          24
#define WDMATLPS_UPPER_TLP_msk          (0xFF<<WDMATLPS_UPPER_TLP_pos)

#define DLWSTAT_MAX_LINK_CAPACITY_pos   0
#define DLWSTAT_MAX_LINK_CAPACITY_msk   0x1F
#define DLWSTAT_NEG_LINK_WIDTH_pos      8
#define DLWSTAT_NEG_LINK_WIDTH_msk      (0x3F<<DLWSTAT_NEG_LINK_WIDTH_pos)

#define DLTRSSTAT_MAX_PAYLOAD_pos       0
#define DLTRSSTAT_MAX_PAYLOAD           0x07
#define DLTRSSTAT_PROG_MAX_PAYLOAD_pos  8
#define DLTRSSTAT_PROG_MAX_PAYLOAD_msk  (0x07<<DLTRSSTAT_PROG_MAX_PAYLOAD_pos)
#define DLTRSSTAT_MAX_RD_REQ_pos        16
#define DLTRSSTAT_MAX_RD_REQ_msk        (0x07<<DLTRSSTAT_MAX_RD_REQ_pos)


#endif /* _registers_epci_h_ */
