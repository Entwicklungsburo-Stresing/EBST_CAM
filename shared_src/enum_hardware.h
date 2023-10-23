#pragma once

//LOW LEVEL ENUMS
enum dma_addresses_t
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

enum PCIEFLAGS_bits_t
{
	PCIEFLAGS_bit_XCKI = 0x01,
	PCIEFLAGS_bit_INTTRIG = 0x02,
	PCIEFLAGS_bit_ENRSTIMERHW = 0x04,
	PCIEFLAGS_bit_USE_ENFFW_PROTECT = 0x08,
	PCIEFLAGS_bit_BLOCKTRIG = 0x10,
	PCIEFLAGS_bit_MEASUREON = 0x20,
	PCIEFLAGS_bit_BLOCKON = 0x40,
	PCIEFLAGS_bit_IS_TDC = 0x100,
	PCIEFLAGS_bit_IS_DSC = 0x200,
	PCIEFLAGS_bit_linkup_sfp3 = 0x4000000,
	PCIEFLAGS_bit_error_sfp3 = 0x8000000,
	PCIEFLAGS_bit_linkup_sfp2 = 0x10000000,
	PCIEFLAGS_bit_error_sfp2 = 0x20000000,
	PCIEFLAGS_bit_linkup_sfp1 = 0x40000000,
	PCIEFLAGS_bit_error_sfp1 = 0x80000000,
	PCIEFLAGS_bitindex_XCKI = 0,
	PCIEFLAGS_bitindex_INTTRIG = 1,
	PCIEFLAGS_bitindex_ENRSTIMERHW = 2,
	PCIEFLAGS_bitindex_USE_ENFFW_PROTECT = 3,
	PCIEFLAGS_bitindex_BLOCKTRIG = 4,
	PCIEFLAGS_bitindex_MEASUREON = 5,
	PCIEFLAGS_bitindex_BLOCKON = 6,
	PCIEFLAGS_bitindex_IS_TDC = 8,
	PCIEFLAGS_bitindex_IS_DSC = 9,
	PCIEFLAGS_bitindex_linkup_sfp3 = 26,
	PCIEFLAGS_bitindex_error_sfp3 = 27,
	PCIEFLAGS_bitindex_linkup_sfp2 = 28,
	PCIEFLAGS_bitindex_error_sfp2 = 29,
	PCIEFLAGS_bitindex_linkup_sfp1 = 30,
	PCIEFLAGS_bitindex_error_sfp1 = 31
};

enum IRQFLAGS_bits_t
{
	IRQFLAGS_bitindex_INTRSR = 31
};

enum CTRLB_bits_t
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

enum CTRLA_bits_t
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

enum CTRLC_bits_t
{
	CTRLC_bit_I = 0x01,
	CTRLC_bit_S1 = 0x02,
	CTRLC_bit_S2 = 0x04,
	CTRLC_bit_eoi = 0x10,
	CTRLC_bit_eoi_chb = 0x20,
	CTRLC_bitindex_I = 0,
	CTRLC_bitindex_S1 = 1,
	CTRLC_bitindex_S2 = 2,
	CTRLC_bitindex_eoi = 4,
	CTRLC_bitindex_eoi_chb = 5
};

enum TOR_TICNT_bits_t
{
	TOR_bits_TICNT = 0x7F,
	TOR_bit_TICNT_EN = 0x80,
	TOR_bitindex_TICNT_EN = 7,
};

enum TOR_TOCNT_bits_t
{
	TOR_bits_TOCNT = 0x7F,
	TOR_bit_TOCNT_EN = 0x80,
	TOR_bitindex_TOCNT_EN = 7,
};

/**
 * TOR MSB 8 bits at 0x2B
 */
enum TOR_MSB_bits_t
{
	TOR_MSB_BITS_TO = 0xF8,
	TOR_MSB_bit_TOSEL = 0x08,
	TOR_MSB_bit_TO0 = 0x10,
	TOR_MSB_bit_TO1 = 0x20,
	TOR_MSB_bit_TO2 = 0x40,
	TOR_MSB_bit_TO3 = 0x80,
	TOR_MSB_bitindex_TOSEL = 3,
	TOR_MSB_bitindex_TO0 = 4,
	TOR_MSB_bitindex_TO1 = 5,
	TOR_MSB_bitindex_TO2 = 6,
	TOR_MSB_bitindex_TO3 = 7
};

enum XCKMSB_bits_t
{
	XCKMSB_bit_stimer_on = 0x40,
	XCKMSB_bitindex_stimer_on = 6
};

enum pcie_addresses_t
{
	PCIeAddr_devCap = 0x5C,
	PCIeAddr_devStatCtrl = 0x60
};

enum FFCTRL_bits_t
{
	FFCTRL_bit_SWTRIG = 0x40,
	FFCTRL_bit_RSFIFO = 0x80,
	FFCTRL_bitindex_SWTRIG = 6,
	FFCTRL_bitindex_RSFIFO = 7,
	FFCTRL_bitindex_scan_reset = 5,
	FFCTRL_bitindex_block_reset = 4
};

enum FF_FLAGS_bits_t
{
	FF_FLAGS_bit_control = 0x08,
	FF_FLAGS_bitindex_valid = 7,
	FF_FLAGS_bitindex_empty = 6,
	FF_FLAGS_bitindex_full = 5,
	FF_FLAGS_bitindex_overflow = 3,
	FF_FLAGS_bitindex_scan_read = 2,
	FF_FLAGS_bitindex_block_read = 1
};

//S0 Addresses
enum s0_addresses_t
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
	S0Addr_FFCTRL = 0x12,
	S0Addr_FF_FLAGS = 0x13,
	S0Addr_FIFOCNT = 0x14, //0x14
	S0Addr_VCLKCTRL = 0x18,
	S0Addr_VCLKFREQ = 0x1b,
	S0Addr_EBST = 0x1C, //0x1c
	S0Addr_SDAT = 0x20, //0x20
	S0Addr_SEC = 0x24, //0x24
	S0Addr_TOR_TICNT = 0x28, //0x28
	S0Addr_TOR_TOCNT = 0x2A,
	S0Addr_TOR_MSB = 0x2B,
	S0Addr_ARREG = 0x2C, //0x2c
	S0Addr_GIOREG = 0x30,
	S0Addr_DELAYEC = 0x34,
	S0Addr_IRQREG = 0x38,
	S0Addr_PCI = 0x3C,
	S0Addr_PCIEFLAGS = 0x40,
	S0Addr_NOS = 0x44,
	S0Addr_ScanIndex = 0x48,
	S0Addr_DmaBufSizeInScans = 0x04C,
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
	S0Addr_DSCCtrl = 0xA8,
	S0Addr_DAC = 0xAC,
	S0Addr_CAMSTATUS12 = 0xB0,
	S0Addr_CAMSTATUS34 = 0xB4,
	S0Addr_CAMERA_TYPE = 0xB8,
};

enum ScanIndex_bits_t
{
	ScanIndex_bitindex_counter_reset = 31,
	ScanIndex_bit_counter_reset = 0x80
};

enum BLOCKINDEX_bits_t
{
	BLOCKINDEX_bitindex_counter_reset = 31,
	BLOCKINDEX_bit_counter_reset = 0x80
};

enum DmaBufSizeInScans_bits_t
{
	DmaBufSizeInScans_bitindex_counter_reset = 31,
	DmaBufSizeInScans_bit_counter_reset = 0x80
};

enum DMAsPerIntr_bits_t
{
	DMAsPerIntr_bitindex_counter_reset = 31,
	DMAsPerIntr_bit_counter_reset = 0x80
};

enum cam_addresses_t
{
	maddr_cam = 0x00,
	maddr_adc = 0x01,
	maddr_ioctrl = 0x02,
	maddr_dac = 0x03,

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

	cam_adaddr_gain = 0x00,
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
	 * 3030: This register controls the length of the ARG pulse which is done after the TG pulse.
	 */
	cam_adaddr_sensor_reset_length_in_8_ns = 0x08,
	/**
	 * stores the amount of vclks generated inside the camera. 
	 * - cam_adaddr_vclks_amount1:    is used for full binning (fft_lines) or the first region of ROI
	 * - cam_adaddr_vclks_amount2..5: are used for ROI mode. Must be set to zero for full binning
	 	 */
	cam_adaddr_vclks_amount1 = 0x9,
	cam_adaddr_vclks_amount2 = 0xA,
	cam_adaddr_vclks_amount3 = 0xB,
	cam_adaddr_vclks_amount4 = 0xC,
	cam_adaddr_vclks_amount5 = 0xD,
	/**
	 * This is a register for the camera position for multiple cameras in line. The software always sets the first camera to 0 and the cameras are handing their positions one to another.
	 */
	cam_adaddr_camera_position = 0x7F,

	adc_ltc2271_regaddr_reset = 0x00,
	adc_ltc2271_regaddr_outmode = 0x02,
	adc_ltc2271_regaddr_custompattern_msb = 0x03,
	adc_ltc2271_regaddr_custompattern_lsb = 0x04,

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

enum cam_messages_t
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
	pixel_fpga_ver = 11
};

/**
 * This enum shows the meaning of the last special pixels. 0: last pixel, 1: last pixel - 1...
 */
enum special_last_pixels_t
{
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

enum camera_type_bits_t
{
	camera_type_sensor_type_bits = 0x0000FFFF,
	camera_type_camera_system_bits = 0xFFFF0000,
	camera_type_sensor_type_bit_index = 0,
	camera_type_camera_system_bit_index = 16,
};
