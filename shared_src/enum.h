#pragma once

//LOW LEVEL ENUMS
//DMA Addresses
enum dma_addresses
{
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
};

enum PCIEFLAGS_bits
{
	PCIEFLAGS_bit_XCKI = 0x01,
	PCIEFLAGS_bit_INTTRIG = 0x02,
	PCIEFLAGS_bit_ENRSTIMERHW = 0x04,
	PCIEFLAGS_bit_INTRSR = 0x08,
	PCIEFLAGS_bit_BLOCKTRIG = 0x10,
	PCIEFLAGS_bit_MEASUREON = 0x20,
	PCIEFLAGS_bit_BLOCKON = 0x40,
	PCIEFLAGS_bitindex_XCKI = 0,
	PCIEFLAGS_bitindex_INTTRIG = 1,
	PCIEFLAGS_bitindex_ENRSTIMERHW = 2,

	PCIEFLAGS_bitindex_BLOCKTRIG = 4,
	PCIEFLAGS_bitindex_MEASUREON = 5,
	PCIEFLAGS_bitindex_BLOCKON = 6
};

enum IRQFLAGS_bits
{
	IRQFLAGS_bitindex_INTRSR = 31
};

enum CTRLB_bits
{
	CTRLB_bit_STI0 = 0x01,
	CTRLB_bit_STI1 = 0x02,
	CTRLB_bit_STI2 = 0x04,
	CTRLB_bit_SHON = 0x08,
	CTRLB_bit_BTI0 = 0x10,
	CTRLB_bit_BTI1 = 0x20,
	CTRLB_bit_BTI2 = 0x40,
	CTRLB_bitindex_STI0 = 0,
	CTRLB_bitindex_STI1 = 1,
	CTRLB_bitindex_STI2 = 2,
	CTRLB_bitindex_SHON = 3,
	CTRLB_bitindex_BTI0 = 4,
	CTRLB_bitindex_BTI1 = 5,
	CTRLB_bitindex_BTI2 = 6
};

enum CTRLA_bits
{
	CTRLA_bit_VONOFF = 0x01,
	CTRLA_bit_IFC = 0x02,
	CTRLA_bit_XCK = 0x04,
	CTRLA_bit_TRIG_OUT = 0x08,
	CTRLA_bit_BOTH_SLOPE = 0x10,
	CTRLA_bit_SLOPE = 0x20,
	CTRLA_bit_DIR_TRIGIN = 0x40,
	CTRLA_bit_TSTART = 0x80,
	CTRLA_bitindex_VONOFF = 0,
	CTRLA_bitindex_IFC = 1,
	CTRLA_bitindex_XCK = 2,
	CTRLA_bitindex_TRIG_OUT = 3,
	CTRLA_bitindex_BOTH_SLOPE = 4,
	CTRLA_bitindex_SLOPE = 5,
	CTRLA_bitindex_DIR_TRIGIN = 6,
	CTRLA_bitindex_TSTART = 7
};

enum TOR_MSB_bits
{
	TOR_MSB_bit_ISFFT = 0x01,
	TOR_MSB_bit_SENDRS = 0x02,
	TOR_MSB_bit_no_RS = 0x04,
	TOR_MSB_bit_RSLEVEL = 0x08,
	TOR_MSB_bitindex_ISFFT = 0,
	TOR_MSB_bitindex_SENDRS = 1,
	TOR_MSB_bitindex_SHORTRS = 2

};

enum XCKMSB_bits
{
	XCKMSB_bit_stimer_on = 0x40,
	XCKMSB_bitindex_stimer_on = 6
};

//PCIe Addresses
enum pcie_addresses
{
	PCIeAddr_devCap = 0x5C,
	PCIeAddr_devStatCtrl = 0x60
};

enum BTRIGREG_bits
{
	BTRIGREG_bit_SWTRIG = 0x40,
	BTRIGREG_bit_RSFIFO = 0x80,
	BTRIGREG_bitindex_SWTRIG = 6,
	BTRIGREG_bitindex_RSFIFO = 7
};

//S0 Addresses
enum s0_addresses
{
	S0Addr_DBR = 0x0, //0x00
	S0Addr_CTRLA = 0x4, //0x04
	S0Addr_CTRLB = 0x5,
	S0Addr_CTRLC = 0x6,
	S0Addr_XCKLL = 0x8, //0x08
	S0Addr_XCKLH = 0x9,
	S0Addr_XCKHL = 0xa,
	S0Addr_XCKMSB = 0xb,
	S0Addr_XCKCNTLL = 0xc, //0x0c
	S0Addr_XCKCNTLH = 0xd,
	S0Addr_XCKCNTHL = 0xe,
	S0Addr_XCKCNTMSB = 0xf,
	S0Addr_PIXREGlow = 0x10, //0x10
	S0Addr_PIXREGhigh = 0x11,
	S0Addr_BTRIGREG = 0x12,
	S0Addr_FF_FLAGS = 0x13,
	S0Addr_FIFOCNT = 0x14, //0x14
	S0Addr_VCLKCTRL = 0x18,
	S0Addr_VCLKFREQ = 0x1b,
	S0Addr_EBST = 0x1C, //0x1c
	S0Addr_SDAT = 0x20, //0x20
	S0Addr_SEC = 0x24, //0x24
	S0Addr_TOR = 0x28, //0x28
	S0Addr_TOR_MSB = 0x2B,
	S0Addr_ARREG = 0x2C, //0x2c
	S0Addr_GIOREG = 0x30,
	S0Addr_DELAYEC = 0x34,
	S0Addr_IRQREG = 0x38,
	S0Addr_PCI = 0x3C,
	S0Addr_PCIEFLAGS = 0x40,
	S0Addr_NOS = 0x44,
	S0Addr_ScanIndex = 0x48,
	S0Addr_DmaBufSizeInScans = 0x04C,		// length in scans
	S0Addr_DMAsPerIntr = 0x050,
	S0Addr_NOB = 0x054,
	S0Addr_BLOCKINDEX = 0x058,
	S0Addr_CAMCNT = 0x05C,
	S0Addr_TDCCtrl = 0x60,
	S0Addr_TDCData = 0x64,
	S0Addr_ROI0 = 0x68,
	S0Addr_ROI1 = 0x6C,
	S0Addr_ROI2 = 0x70,
	S0Addr_XCKDLY = 0x74,
	S0Addr_BTIMER = 0x80,
	S0Addr_BDAT = 0x84,
	S0Addr_BEC = 0x88,
	S0Addr_BSLOPE = 0x8C,
	S0Addr_A1DSC = 0x90,
	S0Addr_L1DSC = 0x94,
	S0Addr_A2DSC = 0x98,
	S0Addr_L2DSC = 0x9C,
	S0Addr_A3DSC = 0xA0,
	S0Addr_L3DSC = 0xA4,
	S0Addr_DSCCtrl = 0xA8
};

enum ScanIndex_bits
{
	ScanIndex_bitindex_counter_reset = 31,
	ScanIndex_bit_counter_reset = 0x80
};

enum BLOCKINDEX_bits
{
	BLOCKINDEX_bitindex_counter_reset = 31,
	BLOCKINDEX_bit_counter_reset = 0x80
};

enum DmaBufSizeInScans_bits
{
	DmaBufSizeInScans_bitindex_counter_reset = 31,
	DmaBufSizeInScans_bit_counter_reset = 0x80
};

enum DMAsPerIntr_bits
{
	DMAsPerIntr_bitindex_counter_reset = 31,
	DMAsPerIntr_bit_counter_reset = 0x80
};

//Cam Addresses könnten später bei unterschiedlichen cam systemen vaariieren
enum cam_addresses
{
	maddr_cam = 0x00,
	maddr_adc = 0x01,
	maddr_ioctrl = 0x02,

	cam_adaddr_gain = 0x00,
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

enum cam_messages
{
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

enum sti_mode
{
	sti_I = 0,
	sti_S1 = 1,
	sti_S2 = 2,
	sti_unused = 3,
	sti_STimer = 4,
	sti_ASL = 5
};

enum bti_mode
{
	bti_I = 0,
	bti_S1 = 1,
	bti_S2 = 2,
	bti_S1S2 = 3,
	bti_BTimer = 4,
};


enum sensor_type
{
	PDAsensor = 0,		//set RS after read; for HA S39xx
	FFTsensor = 1,		//set vclk generator; for HA S703x
};

//camera system select
enum camera_system
{
	camera_system_3001 = 0,
	camera_system_3010 = 1,
	camera_system_3030 = 2
};

enum trigger_mode
{
	xck = 0,
	exttrig = 1,
	dat = 2
};

enum adc_mode
{
	normal = 0,
	ramp = 1,
	custom_pattern = 2
};

enum tor_out
{
	xck_tor = 0,
	rego = 1,
	von = 2,
	dma_act = 3,
	asls = 4,
	stimer = 5,
	btimer = 6,
	isr_act = 7,
	s1 = 8,
	s2 = 9,
	bon = 10,
	measureon = 11,
	sdat = 12,
	bdat = 13,
	sshut = 14,
	bshut = 15
};

enum theme
{
	lighttheme = 0,
	darktheme = 1,
};

enum slope
{
	slope_pos = 0,
	slope_neg = 1,
	slope_both = 2,
};

enum fft_mode
{
	full_binning = 0,
	partial_binning = 1,
	area_mode = 2
};

enum TOR_fkt
{
	TOR_XCK = 0,
	TOR_REG = 1,
	TOR_VON = 2,
	TOR_DMA_ACT = 3,
	TOR_ASLS = 4,
	TOR_STIMER = 5,
	TOR_BTIMER = 6,
	TOR_ISR_ACT = 7,
	TOR_S1 = 8,
	TOR_S2 = 9,
	TOR_BON = 10,
	TOR_MEASUREON = 11,
	TOR_SDAT = 12,
	TOR_BDAT = 13,
	TOR_SSHUT = 14,
	TOR_BSHUT = 15
};
