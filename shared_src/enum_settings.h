#pragma once

/**
 * Scan trigger input mode shows the options for \ref camera_settings.sti_mode
 */
enum sti_mode_t
{
	/**
	 * External trigger on input I of PCIe board
	 */
	sti_I = 0,
	/**
	 * External trigger on input S1 of PCIe board
	 */
	sti_S1 = 1,
	/**
	 * External trigger on input S2 of PCIe board
	 */
	sti_S2 = 2,
	/**
	 * External trigger by I but only when enabled by S2.
	 */
	sti_S2_enable_I = 3,
	/**
	 * Trigger with internal timer. Select the time between two readouts with stime.
	 */
	sti_STimer = 4,
	/**
	 * Automatic internal instant trigger at the end of the last readout.
	 */
	sti_ASL = 5
};

/**
 * Block trigger input mode shows the options for \ref camera_settings.bti_mode
 */
enum bti_mode_t
{
	/**
	 * External trigger on input I of PCIe board
	 */
	bti_I = 0,
	/**
	 * External trigger on input S1 of PCIe board
	 */
	bti_S1 = 1,
	/**
	 * External trigger on input S2 of PCIe board
	 */
	bti_S2 = 2,
	/**
	 * External trigger when inputs S1 and S2 are high
	 */
	bti_S1S2 = 3,
	/**
	 * Trigger with internal timer. Select the time between two blocks of readouts with btimer.
	 */
	bti_BTimer = 4,
	/**
	 * S1 chopper
	 */
	bti_S1chopper = 5,
	/**
	 * S2 chopper
	 */
	bti_S2chopper = 6,
	/**
	 * S1&S2 chopper
	 */
	bti_S1S2chopper = 7
};

/**
 * This enum shows all options for the setting \ref camera_settings.sslope.
 */
enum sslope_t
{
	/**
	 * Use only positive slopes of the trigger input to start a scan.
	 */
	sslope_pos = 0,
	/**
	 * Use only negative slopes of the trigger input to start a scan.
	 */
	sslope_neg = 1,
	/**
	 * Use both slopes of the trigger intput to start a scan.
	 */
	sslope_both = 2,
};

/**
 * This enum shows all options for the setting \ref camera_settings.bslope.
 */
enum bslope_t
{
	/**
	 * Use only negative slopes of the trigger input to start a block.
	 */
	bslope_neg = 0,
	/**
	 * Use only positive slopes of the trigger input to start a block.
	 */
	bslope_pos = 1,
	/**
	 * Use both slopes of the trigger input to start a block.
	 */
	bslope_both = 2,
};

/**
 * Trigger mode shows the options for \ref camera_settings.trigger_mode_integrator.
 */
enum trigger_mode_t
{
	/**
	 * Trigger the integrator with the start signal XCK of the PCIe board. The integrator starts simultaneously with the sensor. 
	 */
	xck = 0,
	/**
	 * Trigger the integrator with the input Ext. Trig. of the camera control. The integrator can be started separately from the sensor
	 */
	exttrig = 1,
	/**
	 * Trigger the integrator with the input Ext. Trig but with an additional delay.
	 */
	dat = 2
};

/**
 * Sensor type shows the options for \ref camera_settings.sensor_type.
 */
enum sensor_type_t
{
	/**
	 * PDA - Photodiode Array.
	 *
	 * These sensors are a PDA:
	 *		* S3901/S3904
	 *		* S3902/S3903
	 *		* S8380/S8381
	 */
	sensor_type_pda = 0,
	/**
	 * IR - Infrared
	 *
	 * These sensors are a IR:
	 *		* G11608
	 *		* G11620
	 *		* G9203-256DA G9204-512DA
	 */
	sensor_type_ir = 1,
	/**
	 * FFT - Full frame transfer.
	 *
	 * These sensors are a FFT:
	 *		* S12600/S12601
	 *		* S7030/S7031
	 *		* S10420-01
	 *		* S16010
	 */
	sensor_type_fft = 2,
	/**
	 * CMOS sensor.
	 *
	 * These sensors are a CMOS:
	 *		* S12198
	 */
	sensor_type_cmos = 3,
	/**
	 * HSVIS - High speed sensor for visible light.
	 *
	 * These sensors are a HSVIS:
	 *		* S14290
	 */
	sensor_type_hsvis = 4,
	/**
	 * HSIR - High speed sensor for infrared light.
	 *
	 * These sensors are a HSIR:
	 *		* G10786
	 */
	sensor_type_hsir = 5,
};

/**
 * Camera system shows the options for \ref camera_settings.camera_system
 */
enum camera_system_t
{
	/**
	 * 3001
	 * 
	 * These camera versions are a 3001 system:
	 *		* 205.X
	 *		* 208.X
	 *		* 210.X
	 *		* 211.X
	 *		* 215.X
	 *		* 218.X
	 * 
	 * These sensors are a 3001 system:
	 *		* S3901/S3904
	 *		* S3902/S3903
	 *		* S8380/S8381
	 *		* G11608
	 *		* G11620
	 *		* G9203-256DA G9204-512DA
	 *		* S12600/S12601
	 *		* S7030/S7031
	 *		* S10420-01
	 *		* S16010
	 */
	camera_system_3001 = 0,
	/**
	 * 3010.
	 * 
	 * These camera versions are a 3010 system:
	 *		* 206.X
	 *		* 216.X
	 *		* 230.X
	 * 
	 * These sensors are a 3010 system:
	 *		* S9037/S9038
	 *		* S11071
	 *		* S12198
	 */
	camera_system_3010 = 1,
	/**
	 * 3030.
	 * 
	 * These camera versions are a 3030 system:
	 *		* 209.X
	 *		* 212.X
	 * 
	 * These sensors are a 3030 system:
	 *		* S14290
	 *		* G10786
	 */
	camera_system_3030 = 2
};

/**
 * Shows the options for \ref camera_settings.fft_mode.
 */
enum fft_mode_t
{
	/**
	 * Full binning is the FFT operation mode for summing up all vertical lines for one readout. The sensor is treated as a single line sensor in this mode. This is the default operation mode.
	 */
	full_binning = 0,
	/**
	 * Partial binning is the FFT operation mode for summing up a specific count of lines per readout to get the sum of specific regions of the sensor. The number of regions is determined by the setting \ref camera_settings.number_of_regions. The size of each region is determined by the setting \ref camera_settings.region_size. In this mode the meaning of scans and blocks changes. One "scan" is now one region of the sensor. So \ref measurement_settings.nos should equal \ref camera_settings.number_of_regions and \ref camera_settings.sti_mode should be set to \ref sti_mode_t.sti_ASL. One "block" is one complete readout of all regions. The time between two block triggers is the exposure time of the sensor for one complete image. \ref measurement_settings.nob and \ref camera_settings.bti_mode can be chosen freely.
	 */
	partial_binning = 1,
	/**
	 * Area mode is the FFT operation mode for reading out each vertical line separately. In this mode the meaning of scans and blocks changes. One "scan" is now one line of the sensor. So \ref measurement_settings.nos should equal \ref camera_settings.fft_lines and \ref camera_settings.sti_mode should be set to \ref sti_mode_t.sti_ASL. One "block" is one complete readout of all lines. The time between two block triggers is the exposure time of the sensor for one complete image. \ref measurement_settings.nob and \ref camera_settings.bti_mode can be chosen freely.
	 */
	area_mode = 2
};

/**
 * TOR out shows the options for \ref camera_settings.tor.
 */
enum tor_out_t
{
	/**
	 * XCK starts the readout of the camera and is high during the readout.
	 */
	tor_xck = 0,
	/**
	 * The signal REGO is controlled by the bit TRIG_OUT of the register CTRLA. It can be controlled with the functions OutTrigLow(), OutTrigHigh() and OutTrigPulse(). The main purpose of this signal is for creating a hardware signal controlled by software, which can be measured by a oscilloscope or used by other electronics.
	 */
	tor_rego = 1,
	/**
	 * DEPRECATED: VON is the signal for the vertical clocks of the FFT sensor. This signal is only used when \ref camera_settings.is_fft_legacy is on.
	 */
	tor_von = 2,
	/**
	 * DMA active is high during the direct memory access of the PCIe card.
	 */
	tor_dma_act = 3,
	/**
	 * ASLS stands for Auto Scan Line Start and shows when a new readout is started when \ref camera_settings.sti_mode is set to \ref sti_mode_t.sti_ASL.
	 */
	tor_asls = 4,
	/**
	 * stimer is the internal trigger for the start of a readout when \ref camera_settings.sti_mode is set to \ref sti_mode_t.sti_STimer. The repetition rate is determined by \ref camera_settings.stime_in_microsec.
	 */
	tor_stimer = 5,
	/**
	 * btimer is the internal trigger for the start of a block when \ref camera_settings.bti_mode is set to \ref bti_mode_t.bti_BTimer. The repetition rate is determined by \ref camera_settings.btime_in_microsec.
	 */
	tor_btimer = 6,
	/**
	 * ISR_ACT is high when the interrupt service routine is active. The ISR copies the data from the DMA buffer to the user buffer, when \ref camera_settings.use_software_polling is not activated.
	 */
	tor_isr_act = 7,
	/**
	 * Shows the state of the input S1 of the PCIe board.
	 */
	tor_s1 = 8,
	/**
	 * Shows the state of the input S2 of the PCIe board.
	 */
	tor_s2 = 9,
	/**
	 * BLOCK_ON_SYNCED is high during a block readout sequence. The rising edge is synced to the next scan trigger after the rising edge of BLOCK_ON. The signal is resetted by BLOCK_EN = 0.
	 */
	tor_block_on_synced = 10,
	/**
	 * measureon is high during the whole measurement.
	 */
	tor_measureon = 11,
	/**
	 * SDAT is high during the time delay between the scan trigger and the start of a scan. This time depends on \ref camera_settings.sdat_in_10ns.
	 */
	tor_sdat = 12,
	/**
	 * BDAT is high during the time delay between the block trigger and the start of a block. This time depends on \ref camera_settings.bdat_in_10ns.
	 */
	tor_bdat = 13,
	/**
	 * sec_mshut is high during the scan exposure window when the option \ref camera_settings.sec_in_10ns is used or can be manually set to high by OpenShutter().
	 */
	tor_sec_mshut = 14,
	/**
	 * bec_mshut is high during the block exposure window when the option \ref camera_settings.bec_in_10ns is used or can be manually set to high by OpenShutter().
	 */
	tor_bec_mshut = 15,
	/**
	 * IFC is the electronic exposure control signal used by some sensors and is sent to the camera. When \ref camera_settings.sec_in_10ns is used IFC = SEC, otherwise it is always 1.
	 */
	tor_ifc = 16,
	/**
	 * DO_CC_I is a fiber link synchronisation signal.
	 */
	tor_do_cc_i = 17,
	/**
	 * Exposure window shows when it is possible to expose the camera to light. Since the PCIe card doesn't know exactly when the sensor is ready to be exposed this is only an estimated signal. If possible use \ref monitor_t.monitor_win of \ref camera_settings.monitor.
	 */
	tor_exposure_window = 18,
	/**
	 * tor_to_cnt_out shows the signal which is controlled by the setting \ref camera_settings.tocnt. Depending on this setting, only specific XCK high periods are shown on the PCIe card output.
	 */
	tor_to_cnt_out = 19,
	/**
	 * SECON is high during the scan exposure window when the option \ref camera_settings.sec_in_10ns is used.
	 */
	tor_secon = 20,
	/**
	 * Shows the state of the input I of the PCIe board.
	 */
	tor_i = 21,
	/**
	 * Shows the delay between the trigger and the moment, when the states of S1 and S2 are read. This delay is controlled by the settting \ref camera_settings.s1s2_read_delay_in_10ns.
	 */
	tor_S1S2readDelay = 22,
	/**
	 * BLOCK_ON is high during a block readout sequence. The rising edge is synced to the next block trigger after the rising edge of BLOCK_EN. The signal is resetted by BLOCK_EN = 0.
	 */
	tor_block_on = 23,
	/**
	 * Unused.
	 */
	tor_unused_24 = 24,
	/**
	 * Unused.
	 */
	tor_unused_25 = 25,
	/**
	 * Unused.
	 */
	tor_unused_26 = 26,
	/**
	 * Unused.
	 */
	tor_unused_27 = 27,
	/**
	 * Unused.
	 */
	tor_unused_28 = 28,
	/**
	 * Status of enable FIFO read.
	 */
	tor_enffr = 29,
	/**
	 * Status of enable FIFO write.
	 */
	tor_enffw = 30,
	/**
	 *
	 */
	tor_enffwrprot = 31
};

/**
 * ADC mode shows the options for \ref camera_settings.adc_mode.
 */
enum adc_mode_t
{
	/**
	 * Normal operation, default.
	 */
	normal = 0,
	/**
	 * Every 8 channels are performing a ramp signal.
	 */
	ramp = 1,
	/**
	 * With custom pattern all 8 ADC channels deliver a specific constant value given by \ref camera_settings.adc_custom_pattern.
	 */
	custom_pattern = 2
};

/**
 * This enum shows all options for the setting \ref camera_settings.channel_select.
 */
enum channel_select_t
{
	channel_select_A = 0,
	channel_select_B = 1,
	channel_select_A_B = 2,
};

/**
 * This enum shows all options for the setting \ref camera_settings.monitor.
 */
enum monitor_t
{
	/**
	 * XCK is high during the readout of the camera.
	 */
	monitor_xck = 0,
	/**
	 * Exp win stands for exposure window and is high, when the sensor is ready to be exposed to light.
	 */
	monitor_win = 1,
	/**
	 * The option ADC CLK sets the output to the ADC clock.
	 */
	monitor_adc_clk = 2,
	/**
	 * Vin is high when the voltage input is sampled.
	 */
	monitor_Vin = 3
};

/**
 * This enum is describing the possible locations of the IC DAC8568 used in DAC8568_sendData().
 */
enum DAC8568_location_t
{
	/**
	 * High speed camera system 3030, PCB 2189-7.
	 */
	DAC8568_camera = 0,
	/**
	 * PCIe add on board EWS, PCB 2226-3.
	 */
	DAC8568_pcie = 1
};

enum settings_level_t
{
	settings_level_guided = 0,
	settings_level_free = 1
};


enum file_specifications_t
{
	file_path_size = 256,
	file_timestamp_size = 64,
	file_filename_full_size = 256
};
