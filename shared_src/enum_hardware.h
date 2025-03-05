/*****************************************************************//**
 * @file   enum_hardware.h
 * @brief  All constants and addresses given by the hardware.
 * 
 * @author Florian Hahn
 * @date   23.10.2020
 *********************************************************************/

#pragma once

enum dma_addresses_t
{
	DmaAddr_DCSR = 0x00,
	DmaAddr_DDMACR = 0x04,
	DmaAddr_WDMATLPA = 0x08,
	DmaAddr_WDMATLPS = 0x0C,
	DmaAddr_WDMATLPC = 0x10,
	DmaAddr_WDMATLPP = 0x14,
	DmaAddr_RDMATLPP = 0x18,
	DmaAddr_RDMATLPA = 0x1C,
	DmaAddr_RDMATLPS = 0x20,
	DmaAddr_RDMATLPC = 0x24,
};

enum s0_addresses_t
{
	S0Addr_DBR = 0x00,
	/**
	 * See @ref CTRL_bits_t for details.
	 */
	S0Addr_CTRL = 0x04,
	/**
	 * See @ref XCK_bits_t for details.
	 */
	S0Addr_XCK = 0x08,
	S0Addr_XCKCNT = 0x0c,
	/**
	 * See @ref PIXREG_FFCTRL_FFFLAGS_bits_t for details.
	 */
	S0Addr_PIXREG_FFCTRL_FFFLAGS = 0x10,
	/**
	 * See @ref FIFOCNT_bits_t for details.
	 */
	S0Addr_FIFOCNT = 0x14,
	/**
	 * See @ref VCLKCTRL_VCLKFREQ_bits_t for details.
	 */
	S0Addr_VCLKCTRL_VCLKFREQ = 0x18,
	S0Addr_EBST = 0x1C,
	/**
	 * See @ref SDAT_bits_t for details.
	 */
	S0Addr_SDAT = 0x20,
	S0Addr_SEC = 0x24,
	/**
	 * See @ref TOR_STICNT_TOCNT_bits_t for details.
	 */
	S0Addr_TOR_STICNT_TOCNT = 0x28,
	/**
	 * See @ref ARREG_bits_t for details.
	 */
	S0Addr_ARREG = 0x2C,
	/**
	 * See @ref GIOREG_bits_t for details.
	 */
	S0Addr_GIOREG = 0x30,
	/**
	 * XCK PERIOD is a 32 bit unsigned integer, which shows the period time of the first XCK period (XCK high slope to high slope) of one measurement in a 10 ns resolution. Read only. Introduced in PCIe card version 222_12.
	 *		* min: 0
	 *		* step: 1 => 10 ns
	 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
	 */
	S0Addr_XCK_PERIOD = 0x34,
	/**
	 * See @ref IRQREG_bits_t for details.
	 */
	S0Addr_IRQREG = 0x38,
	/**
	 * See @ref PCI_bits_t for details.
	 */
	S0Addr_PCI = 0x3C,
	/**
	 * See @ref PCIEFLAGS_bits_t for details.
	 */
	S0Addr_PCIEFLAGS = 0x40,
	S0Addr_NOS = 0x44,
	/**
	 * See @ref ScanIndex_bits_t for details.
	 */
	S0Addr_ScanIndex = 0x48,
	/**
	 * See @ref DmaBufSizeInScans_bits_t for details.
	 */
	S0Addr_DmaBufSizeInScans = 0x4C,
	/**
	 * See @ref DMAsPerIntr_bits_t for details.
	 */
	S0Addr_DMAsPerIntr = 0x50,
	S0Addr_NOB = 0x54,
	/**
	 * See @ref BLOCKINDEX_bits_t for details.
	 */
	S0Addr_BLOCKINDEX = 0x58,
	/**
	 * See @ref CAMCNT_bits_t for details.
	 */
	S0Addr_CAMCNT = 0x5C,
	/**
	 * See @ref TDCCtrl_bits_t for details.
	 */
	S0Addr_TDCCtrl = 0x60,
	S0Addr_TDCData = 0x64,
	/**
	 * See @ref ROI0_bits_t for details.
	 */
	S0Addr_ROI0 = 0x68,
	/**
	 * See @ref ROI1_bits_t for details.
	 */
	S0Addr_ROI1 = 0x6C,
	/**
	 * See @ref ROI2_bits_t for details.
	 */
	S0Addr_ROI2 = 0x70,
	/**
	 * See @ref XCKDELAY_bits_t for details.
	 */
	S0Addr_XCKDLY = 0x74,
	S0Addr_S1S2ReadDelay = 0x78,
	/**
	 * See @ref BTICNT_bits_t for details.
	 */
	S0Addr_BTICNT = 0x7c,
	/**
	 * See @ref BTIMER_bits_t for details.
	 */
	S0Addr_BTIMER = 0x80,
	/**
	 * See @ref BDAT_bits_t for details.
	 */
	S0Addr_BDAT = 0x84,
	/**
	 * See @ref BEC_bits_t for details.
	 */
	S0Addr_BEC = 0x88,
	/**
	 * See @ref BSLOPE_bits_t for details.
	 */
	S0Addr_BSLOPE = 0x8C,
	S0Addr_A1DSC = 0x90,
	S0Addr_L1DSC = 0x94,
	S0Addr_A2DSC = 0x98,
	S0Addr_L2DSC = 0x9C,
	S0Addr_ATDC2 = 0xA0,
	S0Addr_LTDC2 = 0xA4,
	/**
	 * See @ref DSCCtrl_bits_t for details.
	 */
	S0Addr_DSCCtrl = 0xA8,
	S0Addr_DAC = 0xAC,
	/**
	 * XCKLEN is 32 bit unsigned integer, which shows the length of the first XCK of one measurement in a 10 ns resolution. Read only. Introduced in PCIe card version 222_12.
	 *		* min: 0
	 *		* step: 1 => 10 ns
	 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
	 */
	S0Addr_XCKLEN = 0xB0,
	/**
	 * BONLEN is a 32 bit unsigned integer, which shows the length of the first BON of one measurement in a 10 ns resolution. Read only. Introduced in PCIe card version 222_12.
	 *		* min: 0
	 *		* step: 1 => 10 ns
	 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
	 */
	S0Addr_BONLEN = 0xB4,
	/**
	 * See @ref camera_type_bits_t for details.
	 */
	S0Addr_CAMERA_TYPE = 0xB8,
	/**
	 * BON PERIOD is a 32 bit unsigned integer, which shows the period time of the first BON period (BON high slope to high slope) of one measurement in a 10 ns resolution. Read only.Introduced in PCIe card version 222_12.
	 *		* min: 0
	 *		* step: 1 => 10 ns
	 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
	 */
	S0Addr_BON_PERIOD = 0xBC,
};

enum CTRL_bits_t
{
	// CTRLA
	CTRL_bitindex_VONOFF = 0,
	CTRL_bitindex_IFC = 1,
	CTRL_bitindex_XCK = 2,
	CTRL_bitindex_TRIG_OUT = 3,
	/**
	 * 0: slope selected by SLOPE, 1: both slopes
	 */
	CTRL_bitindex_BOTH_SLOPE = 4,
	/**
	 * 0: negative slope, 1: positive slope.
	 */
	CTRL_bitindex_SLOPE = 5,
	CTRL_bitindex_STRIGIN = 6,
	CTRL_bitindex_BSTART = 7,
	// CTRLB
	CTRL_bitindex_STI0 = 8,
	CTRL_bitindex_STI1 = 9,
	CTRL_bitindex_STI2 = 10,
	CTRL_bitindex_SHON = 11,
	CTRL_bitindex_BTI0 = 12,
	CTRL_bitindex_BTI1 = 13,
	CTRL_bitindex_BTI2 = 14,
	// CTRLC
	CTRL_bitindex_I = 16,
	CTRL_bitindex_S1 = 17,
	CTRL_bitindex_S2 = 18,
	CTRL_bitindex_shift_s = 19,
	CTRL_bitindex_eoi = 20,
	CTRL_bitindex_eoi_chb = 21,
	// CTRLA
	CTRL_bit_VONOFF = 0x01,
	CTRL_bit_IFC = 0x02,
	CTRL_bit_XCK = 0x04,
	CTRL_bit_TRIG_OUT = 0x08,
	CTRL_bit_BOTH_SLOPE = 0x10,
	CTRL_bit_SLOPE = 0x20,
	CTRL_bit_STRIGIN = 0x40,
	CTRL_bit_BSTART = 0x80,
	// CTRLB
	CTRL_bit_STI0 = 0x0100,
	CTRL_bit_STI1 = 0x0200,
	CTRL_bit_STI2 = 0x0400,
	CTRL_bits_STI = 0x0700,
	CTRL_bit_SHON = 0x0800,
	CTRL_bit_BTI0 = 0x1000,
	CTRL_bit_BTI1 = 0x2000,
	CTRL_bit_BTI2 = 0x4000,
	CTRL_bits_BTI = 0x7000,
	// CTRLC
	CTRL_bit_I = 0x010000,
	CTRL_bit_S1 = 0x020000,
	CTRL_bit_S2 = 0x040000,
	CTRL_bit_shift_s = 0x080000,
	CTRL_bit_eoi = 0x100000,
	CTRL_bit_eoi_chb = 0x200000,
};

enum XCK_bits_t
{
	XCK_bitindex_stimer = 0,
	/**
	 * select timer base resolution: 0: 1us, 1: 100ns
	 */
	XCK_bitindex_res_ns = 28,
	/**
	 * divide time base resolution by 1000: 0: 1us, 1: 1ms, or when reset_ns = 1: 0: 100ns, 1: 100us
	 */
	XCK_bitindex_res_ms = 29,
	XCK_bitindex_stimer_on = 30,
	XCK_bits_stimer = 0x0FFFFFFF,
	XCK_bit_res_ns = 0x10000000,
	XCK_bit_res_ms = 0x20000000,
	XCK_bit_stimer_on = 0x40000000,
};

enum PIXREG_FFCTRL_FFFLAGS_bits_t
{
	PIXREG_bitindex_pixel = 0,
	FFCTRL_bitindex_block_reset = 20,
	FFCTRL_bitindex_scan_reset = 21,
	FFCTRL_bitindex_SWTRIG = 22,
	FFCTRL_bitindex_RSFIFO = 23,
	FF_FLAGS_bitindex_block_read = 25,
	FF_FLAGS_bitindex_scan_read = 26,
	FF_FLAGS_bitindex_overflow = 27,
	FF_FLAGS_bitindex_xcki = 28,
	FF_FLAGS_bitindex_full = 29,
	FF_FLAGS_bitindex_empty = 30,
	FF_FLAGS_bitindex_valid = 31,
	PIXREG_bits_pixel = 0x0000FFFF,
	FFCTRL_bit_block_reset = 0x100000,
	FFCTRL_bit_scan_reset = 0x200000,
	FFCTRL_bit_SWTRIG = 0x400000,
	FFCTRL_bit_RSFIFO = 0x800000,
	FF_FLAGS_bit_block_read = 0x02000000,
	FF_FLAGS_bit_scan_read = 0x04000000,
	FF_FLAGS_bit_overflow = 0x08000000,
	FF_FLAGS_bit_xcki = 0x10000000,
	FF_FLAGS_bit_full = 0x20000000,
	FF_FLAGS_bit_empty = 0x40000000,
	FF_FLAGS_bit_valid = 0x80000000,
};

enum FIFOCNT_bits_t
{
	FIFOCNT_bitindex_WRCNT = 0,
	FIFOCNT_bits_WRCNT = 0xFF
};

enum VCLKCTRL_VCLKFREQ_bits_t
{
	VCLKCTRL_bitindex = 0,
	VCLKFREQ_bitindex = 24,
	VCLKCNT_bit_control = 0x00000FFF,
	VCLKFREQ_bits = 0xFF000000,
	/**
	 * Base frequency for the vertical clocks.
	 */
	VCLKFREQ_base_value = 200,
	/**
	 * Steps on which the VCLK increases based on VCLKFREQ Value.
	 */
	VCLKFREQ_step_value = 400
};

enum SDAT_bits_t
{
	SDAT_bitindex_control = 0,
	SDAT_bitindex_enable = 31,
	SDAT_bit_control = 0x7FFFFFFF,
	SDAT_bit_enable = 0x80000000,
};

enum TOR_STICNT_TOCNT_bits_t
{
	TOR_bitindex_STICNT = 0,
	TOR_bitindex_STICNT_EN = 7,
	TOR_bitindex_TOCNT = 16,
	TOR_bitindex_TOCNT_EN = 23,
	TOR_bitindex_ISFFT_LEGACY = 24,
	TOR_bitindex_TOSEL = 27,
	TOR_bitindex_TO0 = 28,
	TOR_bitindex_TO1 = 29,
	TOR_bitindex_TO2 = 30,
	TOR_bitindex_TO3 = 31,
	TOR_bits_STICNT = 0x7F,
	TOR_bit_STICNT_EN = 0x80,
	TOR_bits_TOCNT = 0x7F0000,
	TOR_bit_TOCNT_EN = 0x800000,
	TOR_BITS_TO = 0xF8000000,
	TOR_bit_ISFFT_LEGACY = 0x01000000,
	TOR_bit_TOSEL = 0x08000000,
	TOR_bit_TO0 = 0x10000000,
	TOR_bit_TO1 = 0x20000000,
	TOR_bit_TO2 = 0x40000000,
	TOR_bit_TO3 = 0x80000000,
	TOR_bit_TO_control = 0xF0000000
};

enum ARREG_bits_t
{
	ARREG_bitindex_pb_control = 0,
	ARREG_bitindex_partial_binning = 15,
	ARREG_bit_pb_control = 0x7FFF,
	ARREG_bit_partial_binning = 0x8000
};

enum GIOREG_bits_t
{
	GIOREG_bitindex_O1 = 0,
	GIOREG_bitindex_O2 = 1,
	GIOREG_bitindex_O3 = 2,
	GIOREG_bitindex_O4 = 3,
	GIOREG_bitindex_O5 = 4,
	GIOREG_bitindex_O6 = 5,
	GIOREG_bitindex_O7 = 6,
	GIOREG_bitindex_O8 = 7,
	GIOREG_bitindex_I1 = 8,
	GIOREG_bitindex_I2 = 9,
	GIOREG_bitindex_I3 = 10,
	GIOREG_bitindex_I4 = 11,
	GIOREG_bitindex_I5 = 12,
	GIOREG_bitindex_I6 = 13,
	GIOREG_bitindex_I7 = 14,
	GIOREG_bitindex_I8 = 15,
	GIOREG_bit_O1 = 0x0001,
	GIOREG_bit_O2 = 0x0002,
	GIOREG_bit_O3 = 0x0004,
	GIOREG_bit_O4 = 0x0008,
	GIOREG_bit_O5 = 0x0010,
	GIOREG_bit_O6 = 0x0020,
	GIOREG_bit_O7 = 0x0040,
	GIOREG_bit_O8 = 0x0080,
	GIOREG_bit_I1 = 0x0100,
	GIOREG_bit_I2 = 0x0200,
	GIOREG_bit_I3 = 0x0400,
	GIOREG_bit_I4 = 0x0800,
	GIOREG_bit_I5 = 0x1000,
	GIOREG_bit_I6 = 0x2000,
	GIOREG_bit_I7 = 0x4000,
	GIOREG_bit_I8 = 0x8000,
};

enum IRQREG_bits_t
{
	IRQREG_bitindex_IRQLAT = 0,
	IRQREG_bitindex_IRQCNT = 16,
	IRQREG_bitindex_HWDREQ_EN = 30,
	IRQREG_bitindex_INTRSR = 31,
	IRQREG_bits_IRQLAT = 0x0000FFFF,
	IRQREG_bits_IRQCNT = 0x3FFF0000,
	IRQREG_bit_HWDREQ_EN = 0x40000000,
	IRQREG_bit_INTRSR = 0x80000000,
};

enum PCI_bits_t
{
	PCI_bitindex_minor_version = 0,
	PCI_bitindex_major_version = 16,
	PCI_bits_minor_version = 0x0000FFFF,
	PCI_bits_major_version = 0xFFFF0000
};

enum PCIEFLAGS_bits_t
{
	PCIEFLAGS_bitindex_XCKI = 0,
	PCIEFLAGS_bitindex_INTTRIG = 1,
	PCIEFLAGS_bitindex_ENRSTIMERHW = 2,
	PCIEFLAGS_bitindex_USE_ENFFW_PROTECT = 3,
	PCIEFLAGS_bitindex_BLOCKTRIG = 4,
	PCIEFLAGS_bitindex_MEASUREON = 5,
	/**
	 * This bit is a enabling bit for starting a block. Set it to 1 when you want to start a block. The next block trigger after that moment the block will be started. The behavior of this bit changed in P222_14 from a direct control of BLOCK_ON to beeing a enabling bit.
	 */
	PCIEFLAGS_bitindex_BLOCK_EN = 6,
	PCIEFLAGS_bitindex_IS_TDC = 8,
	PCIEFLAGS_bitindex_IS_DSC = 9,
	/**
	 * BLOCK_ON is 1 during one measurement block. The rising edge is synced to the block trigger. It is resetted by setting BLOCK_EN to 0.
	 */
	PCIEFLAGS_bitindex_BLOCK_ON = 10,
	/**
	 * BLOCK_ON_SYNCED is 1 during one measurement block. The rising edge is synced to the next scan trigger after the rising edge of BLOCK_ON. It is resetted by setting BLOCK_EN to 0.
	 */
	PCIEFLAGS_bitindex_BLOCK_ON_SYNCED = 11,
	/**
	 * Scan trigger detected will be set to 1 by the hardware on the slope depending on @ref camera_settings.sslope of the signal @ref camera_settings.sti_mode. It is resetted to 0 by setting @ref PCIEFLAGS_bit_reset_scan_trigger_detected to 1.
	 */
	PCIEFLAGS_bitindex_scan_trigger_detected = 12,
	/**
	 * Block trigger detected will be set to 1 by the hardware on the slope depending on @ref camera_settings.bslope of the signal @ref camera_settings.bti_mode. It is resetted to 0 by setting @ref PCIEFLAGS_bit_reset_block_trigger_detected to 1.
	 */
	PCIEFLAGS_bitindex_block_trigger_detected = 13,
	/**
	 * Setting reset scan trigger detected to 1 resets the bit scan trigger detected to 0.
	 */
	PCIEFLAGS_bitindex_reset_scan_trigger_detected = 14,
	/**
	 * Setting reset block trigger detected to 1 resets the bit block trigger detected to 0.
	 */
	PCIEFLAGS_bitindex_reset_block_trigger_detected = 15,
	PCIEFLAGS_bitindex_linkup_sfp3 = 26,
	PCIEFLAGS_bitindex_error_sfp3 = 27,
	PCIEFLAGS_bitindex_linkup_sfp2 = 28,
	PCIEFLAGS_bitindex_error_sfp2 = 29,
	PCIEFLAGS_bitindex_linkup_sfp1 = 30,
	PCIEFLAGS_bitindex_error_sfp1 = 31,
	PCIEFLAGS_bit_XCKI = 0x0000001,
	PCIEFLAGS_bit_INTTRIG = 0x0000002,
	PCIEFLAGS_bit_ENRSTIMERHW = 0x0000004,
	PCIEFLAGS_bit_USE_ENFFW_PROTECT = 0x0000008,
	PCIEFLAGS_bit_BLOCKTRIG = 0x0000010,
	PCIEFLAGS_bit_MEASUREON = 0x0000020,
	PCIEFLAGS_bit_BLOCK_EN = 0x0000040,
	PCIEFLAGS_bit_IS_TDC = 0x0000100,
	PCIEFLAGS_bit_IS_DSC = 0x0000200,
	PCIEFLAGS_bit_BLOCK_ON = 0x0000400,
	PCIEFLAGS_bit_BLOCK_ON_SYNCED = 0x0000800,
	PCIEFLAGS_bit_scan_trigger_detected = 0x0001000,
	PCIEFLAGS_bit_block_trigger_detected = 0x0002000,
	PCIEFLAGS_bit_reset_scan_trigger_detected = 0x0004000,
	PCIEFLAGS_bit_reset_block_trigger_detected = 0x0008000,
	PCIEFLAGS_bit_linkup_sfp3 = 0x4000000,
	PCIEFLAGS_bit_error_sfp3 = 0x8000000,
	PCIEFLAGS_bit_linkup_sfp2 = 0x10000000,
	PCIEFLAGS_bit_error_sfp2 = 0x20000000,
	PCIEFLAGS_bit_linkup_sfp1 = 0x40000000,
	PCIEFLAGS_bit_error_sfp1 = 0x80000000,
};

enum ScanIndex_bits_t
{
	ScanIndex_bitindex_counter_reset = 31,
	ScanIndex_bits = 0x7FFFFFFF,
	ScanIndex_bit_counter_reset = 0x80000000
};

enum DmaBufSizeInScans_bits_t
{
	DmaBufSizeInScans_bitindex_counter_reset = 31,
	DmaBufSizeInScans_bits = 0x7FFFFFFF,
	DmaBufSizeInScans_bit_counter_reset = 0x80000000
};

enum DMAsPerIntr_bits_t
{
	DMAsPerIntr_bitindex_counter_reset = 31,
	DMAsPerIntrs_bits = 0x7FFFFFFF,
	DMAsPerIntr_bit_counter_reset = 0x80000000
};

enum BLOCKINDEX_bits_t
{
	BLOCKINDEX_bitindex_counter_reset = 31,
	BLOCKINDEX_bit_counter_reset = 0x80
};

enum CAMCNT_bits_t
{
	CAMCNT_bitindex_camcnt = 0,
	CAMCNT_bits = 0x0F
};

enum TDCCtrl_bits_t
{
	TDCCtrl_bitindex_reset = 0,
	TDCCtrl_bitindex_interrupt = 1,
	TDCCtrl_bitindex_load_fifo = 2,
	TDCCtrl_bitindex_empty_fifo = 3,
	TDCCtrl_bitindex_cs = 27,
	TDCCtrl_bitindex_adr0 = 28,
	TDCCtrl_bitindex_adr1 = 29,
	TDCCtrl_bitindex_adr2 = 30,
	TDCCtrl_bitindex_adr3 = 31,
	TDCCtrl_bit_reset = 0x1,
	TDCCtrl_bit_interrupt = 0x2,
	TDCCtrl_bit_load_fifo = 0x4,
	TDCCtrl_bit_empty_fifo = 0x8,
	TDCCtrl_bit_cs = 0x8000000,
	TDCCtrl_bit_adr0 = 0x10000000,
	TDCCtrl_bit_adr1 = 0x20000000,
	TDCCtrl_bit_adr2 = 0x40000000,
	TDCCtrl_bit_adr3 = 0x80000000
};

enum ROI0_bits_t
{
	ROI0_bitindex_range1 = 0,
	ROI0_bitindex_range1_keep = 15,
	ROI0_bitindex_range2 = 16,
	ROI0_bitindex_range2_keep = 31,
	ROI0_bits_range1 = 0x00000FFF,
	ROI0_bit_range1_keep = 0x00008000,
	ROI0_bits_range2 = 0x0FFF0000,
	ROI0_bit_range2_keep = 0x80000000,
};

enum ROI1_bits_t
{
	ROI1_bitindex_range3 = 0,
	ROI0_bitindex_range3_keep = 15,
	ROI1_bitindex_range4 = 16,
	ROI0_bitindex_range4_keep = 31,
	ROI1_bits_range3 = 0x00000FFF,
	ROI1_bit_range3_keep = 0x00008000,
	ROI1_bits_range4 = 0x0FFF0000,
	ROI1_bit_range4_keep = 0x80000000,
};

enum ROI2_bits_t
{
	ROI2_bitindex_range5 = 0,
	ROI2_bitindex_range5_keep = 15,
	ROI2_bits_range5 = 0x00000FFF,
	ROI2_bit_range5_keep = 0x00008000,
};

enum XCKDELAY_bits_t
{
	XCKDELAY_bits = 0x7FFFFFFF,
	XCKDELAY_bit_enable = 0x80000000,
	XCKDELAY_bitindex_enable = 31
};

enum BTICNT_bits_t
{
	BTICNT_bitindex_BTICNT = 0,
	BTICNT_bitindex_BTICNT_EN = 7,
	BTICNT_bits_BTICNT = 0x7F,
	BTICNT_bit_BTICNT_EN = 0x80,
};

enum BTIMER_bits_t
{
	BTIMER_bits = 0x0FFFFFFF
};

enum BDAT_bits_t
{
	BDAT_bitindex_enabled = 31,
	BDAT_bits_BDAT = 0x7FFFFFFF,
	BDAT_bit_enable = 0x80000000,
};

enum BEC_bits_t
{
	BEC_bitindex_enabled = 31,
	BEC_bits_BEC = 0x7FFFFFFF,
	BEC_bit_enable = 0x80000000,
};

enum BSLOPE_bits_t
{
	BSLOPE_bitindex_bslope = 0,
	BSLOPE_bitindex_both_slopes = 1,
	BSLOPE_bitindex_bswtrig = 2,
	BSLOPE_bit_bslope = 0x00000001,
	BSLOPE_bit_both_slopes = 0x00000002,
	BSLOPE_bit_both_bswtrig = 0x00000004,
};

enum DSCCtrl_bits_t
{
	DSCCtrl_bitindex_rs1 = 0,
	DSCCtrl_bitindex_dir1 = 1,
	DSCCtrl_bitindex_rs2 = 8,
	DSCCtrl_bitindex_dir2 = 9,
	DSCCtrl_bit_rs1 = 0x00000001,
	DSCCtrl_bit_dir1 = 0x00000002,
	DSCCtrl_bit_rs2 = 0x00000100,
	DSCCtrl_bit_dir2 = 0x00000200,
};

enum camera_type_bits_t
{
	camera_type_sensor_type_bit_index = 0,
	camera_type_camera_system_bit_index = 16,
	camera_type_sensor_type_bits = 0x0000FFFF,
	camera_type_camera_system_bits = 0xFFFF0000,
};

enum master_address_t
{
	maddr_cam = 0x0,
	maddr_adc = 0x1,
	maddr_ioctrl = 0x2,
	maddr_dac = 0x3,
};

/**
 * These registers are addressed when maddr = 0.
 */
enum camera_register_addresses_t
{
	/**
	 * See details in @ref cam_config_register_t.
	 */
	cam_adaddr_config = 0x00,
	cam_adaddr_pixel = 0x01,
	cam_adaddr_trig_in = 0x02,
	cam_adaddr_unused = 0x03,
	cam_adaddr_vclk = 0x04,
	cam_adaddr_LEDoff = 0x05,
	cam_adaddr_coolTemp = 0x06,
	/**
	 * 3030: Currently not in use. Sample mode is setting the ADC clock and the sensor clock.
	 * - 0: ADC clock = sensor clock = 25 MHz with duty cycle 50%
	 * - 1: ADC clock = 50 MHz, duty cycle 50%, sensor clock = 12,5 MHz, duty cycle 20%
	 * - 2: ADC clock = 25 MHz, duty cycle 50%, sensor clock = 12,5 MHz duty cycle 20%
	 */
	cam_adaddr_sample_mode = 0x07,
	/**
	 * Sensor reset length register.
	 *
	 * 3030 HSVIS: This register controls the length of the ARG pulse which is done after the TG pulse.
	 * min: 0ns, max: 0xFFFF * 4ns = 65535 * 4ns = 262140ns = 262,14us, typical value: 200 * 4ns = 800ns
	 * HSIR: min: 134 * 160 ns = 21,440 ns, max: 0xFFFF * 160 ns = 10,485,600 ns, default: 140 * 160 ns = 22,400 ns
	 */
	cam_adaddr_sensor_reset_length = 0x08,
	/**
	 * stores the amount of vclks generated inside the camera.
	 * - cam_adaddr_vclks_amount1: is used for full binning (fft_lines) or the first region of ROI
	 * - cam_adaddr_vclks_amount2..5: are used for ROI mode. Must be set to zero for full binning
		 */
	cam_adaddr_vclks_amount1 = 0x09,
	cam_adaddr_vclks_amount2 = 0x0A,
	cam_adaddr_vclks_amount3 = 0x0B,
	cam_adaddr_vclks_amount4 = 0x0C,
	cam_adaddr_vclks_amount5 = 0x0D,
	/**
	 * Send any data to this address to initialize the camera. This should be done last in the initialisation routine, after reset and after writing all other registers.
	 */
	cam_adaddr_camera_init = 0x10,
	/**
	 * Send any data to this address to do a software reset. This should be done first in the initialisation routine.
	 */
	cam_adaddr_software_reset = 0x7E,
	/**
	 * This is a register for the camera position for multiple cameras in line. The software always sets the first camera to 0 and the cameras are handing their positions one to another.
	 */
	cam_adaddr_camera_position = 0x7F,
};

enum cam_config_register_t
{
	cam_config_register_bitindex_sensor_gain = 0,
	cam_config_register_bitindex_trigger_mode_cc = 1,
	cam_config_register_bitindex_temp_level = 4,
	cam_config_register_bitindex_led_off = 7,
	cam_config_register_bitindex_monitor = 8,
	/**
	 * Added in P230.6.
	 */
	cam_config_register_bitindex_channel_select_a = 10,
	/**
	 * Added in P230.6.
	 */
	cam_config_register_bitindex_channel_select_b = 11,
	cam_config_register_bitindex_sensor_gain_2 = 12,
	cam_config_register_bits_sensor_gain = 0x0001,
	cam_config_register_bits_trigger_mode_cc = 0x000E,
	cam_config_register_bits_temp_level = 0x0070,
	cam_config_register_bits_led_off = 0x0080,
	cam_config_register_bits_monitor = 0x0300,
	cam_config_register_bit_channel_select_a = 0x0400,
	cam_config_register_bit_channel_select_b = 0x0800,
	cam_config_register_bit_sensor_gain_2 = 0x1000,
};

/**
 * These registers are addressed when maddr = 1 and camera system = 3010.
 */
enum adc_ltc2271_register_adress_t
{
	adc_ltc2271_regaddr_reset = 0x00,
	adc_ltc2271_regaddr_outmode = 0x02,
	adc_ltc2271_regaddr_custompattern_msb = 0x03,
	adc_ltc2271_regaddr_custompattern_lsb = 0x04,
};

enum adc_ltc2271_messages_t
{
	adc_ltc2271_msg_reset = 0x80,
	adc_ltc2271_msg_normal_mode = 0x01,
	adc_ltc2271_msg_custompattern = 0x05,
};

/**
 * These registers are addressed when maddr = 1 and camera system = 3030.
 */
enum adc_ads5294_register_adress_t
{
	adc_ads5294_regaddr_reset = 0x00,
	/**
	 * Low frequency noise suppression mode
	 * - D0...D7: LFNSM for each channel 1...8
	 */
	adc_ads5294_regaddr_LFNSM = 0x14,
	adc_ads5294_regaddr_mode = 0x25,
	adc_ads5294_regaddr_custompattern = 0x26,
	adc_ads5294_regaddr_wordWiseOutput = 0x28,
	/**
	 * - D0: en_channel_avg 1: Enabled channel averaging mode, 0: disable
	 * - D1: global_en_filter 1: enables filter blocks globally, 0: disable
	 */
	adc_ads5294_regaddr_global_en_filter = 0x29,
	adc_ads5294_regaddr_gain_1_to_4 = 0x2A,
	adc_ads5294_regaddr_gain_5_to_8 = 0x2B,
	/**
	 * - D0: use_filter 1: enable filter, 0: disable
	 * - D2: odd_tap 1: Use odd tap filter, 0: disable
	 * - D4...D6: filter_rate: set decimation factor
	 * - D7...D9: filter_coeff_set: select stored coefficient set
	 * - D10...D13 hpf_corner: HPF corner values k from 2 to 10
	 * - D14 hpf_en: 1: hpf enable, 0: disable
	 */
	adc_ads5294_regaddr_filter1 = 0x2E,
	adc_ads5294_regaddr_filter2 = 0x2F,
	adc_ads5294_regaddr_filter3 = 0x30,
	adc_ads5294_regaddr_filter4 = 0x31,
	adc_ads5294_regaddr_filter5 = 0x32,
	adc_ads5294_regaddr_filter6 = 0x33,
	adc_ads5294_regaddr_filter7 = 0x34,
	adc_ads5294_regaddr_filter8 = 0x35,
	/**
	 * D0...D1: data_rate:
	 *		- 0: All converted values at the ADC sampling rate are shown on the digital output
	 *		- 1: 1/2 of ADC sampling rate
	 *		- 2: 1/4 of ADC sampling rate
	 *		- 3: 1/8 of ADC sampling rate
	 */
	adc_ads5294_regaddr_data_rate = 0x38,
	adc_ads5294_regaddr_ddrClkAlign = 0x42,
	adc_ads5294_regaddr_2wireMode = 0x46,
	/**
	 * - D0...D11: 12 bit long coefficient
	 * - D15: en_custom_filt, 1: enable custom coefficient, 0: use preset coefficient, programmed by filter_coeff_set
	 */
	adc_ads5294_regaddr_coeff0_filter1 = 0x5A,
	adc_ads5294_regaddr_coeff1_filter1 = 0x5B,
	adc_ads5294_regaddr_coeff11_filter1 = 0x65,
	adc_ads5294_regaddr_coeff0_filter2 = 0x66,
	adc_ads5294_regaddr_coeff0_filter3 = 0x72,
	// This is the 7 bit border. With the current implementation in the FPGA, only 7 bit addresses are accessible.
	adc_ads5294_regaddr_coeff1_filter4 = 0x7F,
	adc_ads5294_regaddr_coeff11_filter8 = 0xB9,
};

enum adc_ads5294_messages_t
{
	adc_ads5294_msg_reset = 0x01,
	adc_ads5294_msg_ramp = 0x40,
	adc_ads5294_msg_custompattern = 0x10,
	adc_ads5294_msg_2wireMode = 0x8401,
	adc_ads5294_msg_wordWiseOutput = 0x80FF,
	adc_ads5294_msg_ddrClkAlign = 0x60,
};

/**
 * These registers are addressed when maddr = 2.
 */
enum ioctrl_register_address_t
{
	ioctrl_impact_start_pixel = 0x00,
	ioctrl_t0l = 0x01,
	ioctrl_t0h = 0x02,
	ioctrl_t1 = 0x03,
	ioctrl_d1 = 0x04,
	ioctrl_t2 = 0x05,
	ioctrl_d2 = 0x06,
	ioctrl_t3 = 0x07,
	ioctrl_d3 = 0x08,
	ioctrl_t4 = 0x09,
	ioctrl_d4 = 0x0A,
	ioctrl_t5 = 0x0B,
	ioctrl_d5 = 0x0C,
	ioctrl_t6 = 0x0D,
	ioctrl_d6 = 0x0E,
	ioctrl_t7 = 0x0F,
	ioctrl_d7 = 0x10,
};

/**
 * These register are addressed when maddr = 3.
 */
enum dac_register_addresses_t
{
	dac_hi_byte_addr = 0x01,
	dac_lo_byte_addr = 0x02,

	/**
	 * The adaddr is structured as following:
	 * - c: camera position
	 * - r: register address
	 * 0b ccc rrrr
	 * The upper 3 bits are describing the camera position and the lower 4 bits are describing the register address. (Only 7 bits are used)
	 */
	campos_bit_index = 4,
};

/**
 * This enum shows the meaning of the first special pixels.
 */
enum special_pixels_enum_t
{
	/**
	 * See enum bits_of_pixel_block_index_high_S1_S2 for details.
	 */
	pixel_block_index_high_s1_s2 = 2,
	/**
	 * Lower 16 bits of block index counter.
	 */
	pixel_block_index_low = 3,
	/**
	 * Higher 16 bits of scan index counter.
	 */
	pixel_scan_index_high = 4,
	/**
	 * Lower 16 bits of scan index counter.
	 */
	pixel_scan_index_low = 5,
	/**
	 * Special pixel for PCIe daughter boards. Higher 16 bits of DSC 1 / TDC 1
	 */
	pixel_impact_signal_1_high = 6,
	/**
	 * Special pixel for PCIe daughter boards. Lower 16 bits of DSC 1 / TDC 1
	 */
	pixel_impact_signal_1_low = 7,
	/**
	 * Special pixel for PCIe daughter boards. Higher 16 bits of DSC 2 / TDC 2
	 */
	pixel_impact_signal_2_high = 8,
	/**
	 * Special pixel for PCIe daughter boards. Lower 16 bits of DSC 2 / TDC 2
	 */
	pixel_impact_signal_2_low = 9,
	/**
	 * See enum pixel_camera_status_bits for details.
	 */
	pixel_camera_status = 10,
	/**
	 * Special pixel for fpga ver number.
	 */
	pixel_fpga_ver = 11,
	pixel_first_sensor_pixel = 12,
	/**
	 * Number of special pixels. 12 in the beginning and 2 at the end of one scan.
	 */
	pixel_number_of_special_pixels = 14,
};

/**
 * This enum shows the meaning of the last special pixels. 0: last pixel, 1: last pixel - 1...
 */
enum special_last_pixels_t
{
	pixel_last_sensor_pixel = 2,
	/**
	 * Higher 16 bits of scan index counter.
	 */
	pixel_scan_index2_high = 1,
	/**
	 * Lower 16 bits of scan index counter.
	 */
	pixel_scan_index2_low = 0,
};

/**
 * This enum shows the encoding of the special pixel 2. The upper two bits are encoding the binary state of S1 and S2. All other bits are representing the upper half of the block index counter.
 */
enum bits_of_pixel_block_index_high_S1_S2_t
{
	/**
	 * The lower 14 bits are representing the bits 29 to 16 from block index.
	 */
	pixel_block_index_high_s1_s2_bits_block_index = 0x3FFF,
	/**
	 * 1: Input S2 is high, 0: S2 is low.
	 */
	pixel_block_index_high_s1_s2_bit_s2 = 0x4000,
	/**
	 * 1: Input S1 is high, 0: S1 is low.
	 */
	pixel_block_index_high_s1_s2_bit_s1 = 0x8000,
	pixel_block_index_high_s1_s2_bitindex_s2 = 14,
	pixel_block_index_high_s1_s2_bitindex_s1 = 15,
};

/**
 * This enum shows the meaning of the bits of the pixel camera status.
 */
enum pixel_camera_status_bits_t
{
	/**
	 * Over temperature. 1: over temperature detected, 0: temperature normal
	 */
	pixel_camera_status_bitindex_over_temp = 0,
	/**
	 * Temperature good. Only for cooled cameras. 1: target cooling temperature reached, 0: target temperature not reached
	 */
	pixel_camera_status_bitindex_temp_good = 1,
	/**
	 * 1: Connected camera is system 3001.
	 */
	pixel_camera_status_bitindex_3001 = 11,
	/**
	 * 1: Connected camera is system 3010.
	 */
	pixel_camera_status_bitindex_3010 = 12,
	/**
	 * 1: Connected camera is system 3030.
	 */
	pixel_camera_status_bitindex_3030 = 13,
	pixel_camera_status_bit_over_temp = 0x1,
	pixel_camera_status_bit_temp_good = 0x2,
	pixel_camera_status_bit_3001 = 0x0800,
	pixel_camera_status_bit_3010 = 0x1000,
	pixel_camera_status_bit_3030 = 0x2000,
};

/**
 * This enum shows the starting number of the major and minor version for the fpga version number.
 */
enum pixel_fpga_ver_t
{
	/**
	 * Starting Bit of Major Version Number.
	 */
	pixel_fpga_ver_major_bit = 0,

	/**
	 * Starting Bit of Minor Version Number.
	 */
	pixel_fpga_ver_minor_bit = 8,

	/**
	 * Used for correcting the pixel to only get the major version of the version number.
	 */
	pixel_fpga_ver_major_and_bit = 0x00FF
};

enum autotune_channel_ranges_hsvis_t
{
	autotune_hsvis_ch1_start = 44,
	autotune_hsvis_ch1_end = 108,
	autotune_hsvis_ch2_start = 172,
	autotune_hsvis_ch2_end = 236,
	autotune_hsvis_ch3_start = 300,
	autotune_hsvis_ch3_end = 364,
	autotune_hsvis_ch4_start = 428,
	autotune_hsvis_ch4_end = 492,
	autotune_hsvis_ch5_start = 556,
	autotune_hsvis_ch5_end = 620,
	autotune_hsvis_ch6_start = 684,
	autotune_hsvis_ch6_end = 748,
	autotune_hsvis_ch7_start = 812,
	autotune_hsvis_ch7_end = 876,
	autotune_hsvis_ch8_start = 940,
	autotune_hsvis_ch8_end = 1004,
};

enum autotune_channel_ranges_hsir_t
{
	autotune_hsir_ch1_start = 12,
	autotune_hsir_ch1_end = 264,
	autotune_hsir_ch2_start = 13,
	autotune_hsir_ch2_end = 265,
	autotune_hsir_ch3_start = 266,
	autotune_hsir_ch3_end = 520,
	autotune_hsir_ch4_start = 267,
	autotune_hsir_ch4_end = 521,
	autotune_hsir_ch5_start = 522,
	autotune_hsir_ch5_end = 766,
	autotune_hsir_ch6_start = 523,
	autotune_hsir_ch6_end = 767,
	autotune_hsir_ch7_start = 778,
	autotune_hsir_ch7_end = 1032,
	autotune_hsir_ch8_start = 779,
	autotune_hsir_ch8_end = 1031,
};

/**
 * Addresses of PCIe configuration space. See documentation of Spartan-6 FPGA Integrated Endpoint Block for details. Table 2-2.
 * https://docs.xilinx.com/v/u/en-US/s6_pcie_ug654
 */
enum pcie_configuration_space_t
{
	PCIeAddr_VendorID = 0x00,
	PCIeAddr_DeviceID = 0x02,
	PCIeAddr_Command = 0x04,
	PCIeAddr_Status = 0x06,
	PCIeAddr_RevID = 0x08,
	PCIeAddr_ClassCode = 0x09,
	PCIeAddr_CacheLn = 0x0C,
	PCIeAddr_LatTimer = 0x0D,
	PCIeAddr_Header = 0x0E,
	PCIeAddr_BIST = 0x0F,
	PCIeAddr_BaseAddressRegister0 = 0x10,
	PCIeAddr_BaseAddressRegister1 = 0x14,
	PCIeAddr_BaseAddressRegister2 = 0x18,
	PCIeAddr_BaseAddressRegister3 = 0x1C,
	PCIeAddr_BaseAddressRegister4 = 0x20,
	PCIeAddr_BaseAddressRegister5 = 0x24,
	PCIeAddr_CardbusCisPointer = 0x28,
	PCIeAddr_SubsystemVendorID = 0x2C,
	PCIeAddr_SubsystemID = 0x2E,
	PCIeAddr_ExpansionRomBaseAddress = 0x30,
	PCIeAddr_CapPTr = 0x34,
	PCIeAddr_Reserved1 = 0x35,
	PCIeAddr_Reserved2 = 0x38,
	PCIeAddr_IntrLine = 0x3C,
	PCIeAddr_IntrPin = 0x3D,
	PCIeAddr_MinGnt = 0x3E,
	PCIeAddr_Maxlat = 0x3F,
	PCIeAddr_PMCap = 0x40,
	PCIeAddr_NxtCap1 = 0x41,
	PCIeAddr_PMCapability = 0x42,
	PCIeAddr_PMCSR = 0x44,
	PCIeAddr_BSE = 0x46,
	PCIeAddr_Data = 0x47,
	PCIeAddr_MSICap = 0x48,
	PCIeAddr_NxtCap2 = 0x49,
	PCIeAddr_MsiControl = 0x4A,
	PCIeAddr_MessageAddressLower = 0x4C,
	PCIeAddr_MessageAddressUpper = 0x50,
	PCIeAddr_MessageData = 0x54,
	PCIeAddr_Reserved3 = 0x56,
	PCIeAddr_PeCap = 0x58,
	PCIeAddr_NxtCap3 = 0x59,
	PCIeAddr_PeCapabilty = 0x5A,
	PCIeAddr_PCIExpressDeviceCapabilities = 0x5C,
	PCIeAddr_DeviceControl = 0x60,
	PCIeAddr_DeviceStatus = 0x62,
	PCIeAddr_PCIExpressLinkCapabilities = 0x64,
	PCIeAddr_LinkControl = 0x68,
	PCIeAddr_LinkStatus = 0x6A,
	PCIeAddr_ReservedLegacyConfigurationSpace = 0x6C
};

/**
 * See section 7.8.3, figure 7-13 of the PCI Express Base Specification.
 * https://astralvx.com/storage/2020/11/PCI_Express_Base_4.0_Rev0.3_February19-2014.pdf
 */
enum PciExpressDeviceCapabilities_bits_t
{
	PciExpressDeviceCapabilities_MaxPayloadSizeSupported_bits = 0x7
};

/**
 * See section 7.8.3, table 7-13 of the PCI Express Base Specification.
 * https://astralvx.com/storage/2020/11/PCI_Express_Base_4.0_Rev0.3_February19-2014.pdf
 */
enum MaxPayloadSizeSupported_encoding_t
{
	maxPaxloadSize_128bytes = 0,
	maxPaxloadSize_256bytes = 1,
	maxPaxloadSize_512bytes = 2,
	maxPaxloadSize_1024bytes = 3,
	maxPaxloadSize_2048bytes = 4,
	maxPaxloadSize_4096bytes = 5,
	maxPaxloadSize_reserved1 = 6,
	maxPaxloadSize_reserved2 = 7,
};

/**
 * See section 7.8.4, figure 7-14 of the PCI Express Base Specification.
 * https://astralvx.com/storage/2020/11/PCI_Express_Base_4.0_Rev0.3_February19-2014.pdf
 */
enum DeviceControl_bits_t
{
	deviceControl_maxPayloadSize_bits = 0xE0,
	deviceControl_maxReadRequestSize_bits = 0x7000,
	deviceControl_maxPayloadSize_bitindex = 5,
	deviceControl_maxReadRequestSize_bitindex = 12,
};
