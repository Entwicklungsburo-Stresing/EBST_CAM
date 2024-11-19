#include "Camera.h"
#include "../shared_src/enum_hardware.h"
#include "Board.h"

/**
 * \brief Initialize camera registers.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_Init(uint32_t drvno)
{
	ES_LOG("\nInit camera %u\n", drvno);
	es_status_codes status = FindCam(drvno);
	if (status != es_no_error) return status;
	status = Cam_DoSoftReset(drvno);
	if (status != es_no_error) return status;
	status = Cam_SetPosition(drvno);
	if (status != es_no_error) return status;
	// when cooled camera legacy mode: disable PCIe FIFO when cool cam transmits cool status
	if (settings_struct.camera_settings[drvno].is_cooled_camera_legacy_mode)
		status = Use_ENFFW_protection(drvno, true);
	else
		status = Use_ENFFW_protection(drvno, false);
	status = Cam_SetPixelRegister(drvno);
	if (status != es_no_error) return status;
	status = Cam_SetTriggerInput(drvno);
	if (status != es_no_error) return status;
	//set led off
	status = Cam_SetLedOff(drvno, (uint8_t)settings_struct.camera_settings[drvno].led_off);
	if (status != es_no_error) return status;
	//set gain switch (mostly for IR sensors)
	status = Cam_SetConfigRegister(drvno);
	if (status != es_no_error) return status;

	if (settings_struct.camera_settings[drvno].sensor_type == sensor_type_fft)
	{
		status = Cam_SetupFFT(drvno);
		if (status != es_no_error) return status;
	}

	if (status != es_no_error) return status;
	switch (settings_struct.camera_settings[drvno].camera_system)
	{
	case camera_system_3001:
		status = Cam3001_Init(drvno);
		break;
	case camera_system_3010:
		status = Cam3010_Init(drvno, (uint8_t)settings_struct.camera_settings[drvno].adc_mode, (uint16_t)settings_struct.camera_settings[drvno].adc_custom_pattern);
		break;
	case camera_system_3030:
		status = Cam3030_Init(drvno);
		break;
	default:
		return es_parameter_out_of_range;
	}
	if (status != es_no_error) return status;
	//for cooled Cam
	status = Cam_SetTemp(drvno, (uint8_t)settings_struct.camera_settings[drvno].temp_level);
	if (status != es_no_error) return status;
	status = CamIOCtrl_setImpactStartPixel(drvno, (uint16_t)settings_struct.camera_settings[drvno].ioctrl_impact_start_pixel);
	if (status != es_no_error) return status;
	for (uint8_t i = 1; i <= IOCTRL_OUTPUT_COUNT - 1; i++)
	{
		status = CamIOCtrl_setOutput(drvno, i, (uint16_t)settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[i - 1], (uint16_t)settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[i - 1]);
		if (status != es_no_error) return status;
	}
	status = CamIOCtrl_setT0(drvno, settings_struct.camera_settings[drvno].ioctrl_T0_period_in_10ns);
	if (status != es_no_error) return status;
	status = Cam_SetSensorResetOrHsirEc(drvno, settings_struct.camera_settings[drvno].sensor_reset_or_hsir_ec);
	if (status != es_no_error) return status;
	status = Cam_Initialize(drvno);
	return status;
}

/**
 * \brief Do a soft reset of the camera.
 *
 * Do this first in the camera initialisation routine.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 */
es_status_codes Cam_DoSoftReset(uint32_t drvno)
{
	ES_LOG("Do camera soft reset\n");
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_software_reset, 0);
}

/**
 * \brief Trigger initialisation in the camera.
 *
 * Do this last in the camera initialisation routine.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 */
es_status_codes Cam_Initialize(uint32_t drvno)
{
	ES_LOG("Do init in camera\n");
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_camera_init, 0);
}

/**
 * \brief Init routine for Camera System 3001.
 *
 * 	Sets register in camera.
 * \param drvno selects PCIe board
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes Cam3001_Init(uint32_t drvno)
{
	(void)drvno;
	ES_LOG("Init camera 3001\n");
	return es_no_error;
}

/**
 * \brief Init routine for Camera System 3010.
 *
 * 	Sets registers in camera and ADC LTC2271.
 * 	FL3010 is intended for sensor S12198 !
 * 	with frame rate 8kHz = min. 125µs exp time
 * \param drvno selects PCIe board
 * \param adc_mode 0: normal mode, 2: custom pattern
 * \param custom_pattern fixed output for test mode, ignored when test mode FALSE
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes Cam3010_Init(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern)
{
	ES_LOG("Init camera 3010, adc_mode: %u, custom_pattern: %u\n", adc_mode, custom_pattern);
	es_status_codes status = Cam3010_ADC_reset(drvno);
	if (status != es_no_error) return status;
	return Cam3010_ADC_setOutputMode(drvno, adc_mode, custom_pattern);
}


/**
 * \brief ADC reset routine for Camera System 3010.
 *
 * 	ADC LTC2271 needs a reset via SPI first. Bit D7
 * 	of the reset register A0 with address 00h is set to 1.
 * 	D6:D0 are don't care. So address is 00h and data is
 * 	80h = 10000000b for e.g.
 * 	This has to be done after every startup.
 * 	Then the ADC can be programmed further via SPI in the next frames.
 * 	Called by Cam3010_Init
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3010_ADC_reset(uint32_t drvno)
{
	ES_LOG("Camera 3010 ADC reset\n");
	return Cam_SendData(drvno, maddr_adc, adc_ltc2271_regaddr_reset, adc_ltc2271_msg_reset);
}

/**
 * \brief
 * \param drvno selects PCIe board
 * \param adc_mode
 * \param custom_pattern
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3010_ADC_setOutputMode(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern) {

	if (2 == adc_mode) {
		ES_LOG("Camera 3010 ADC set output mode to 4-lane mode and test pattern\n");
		es_status_codes status = Cam_SendData(drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_custompattern);
		if (status != es_no_error) return status;
		return Cam3010_ADC_sendTestPattern(drvno, custom_pattern);
	}
	else {
		ES_LOG("Camera 3010 ADC set output mode to 4-lane mode and normal operation\n");
		return Cam_SendData(drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_normal_mode);
	}
}

/**
 * \brief
 * \param drvno selects PCIe board
 * \param custom_pattern
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3010_ADC_sendTestPattern(uint32_t drvno, uint16_t custom_pattern) {
	es_status_codes status;

	uint8_t  highByte = custom_pattern >> 8;
	uint8_t  lowByte = custom_pattern & 0x00FF;

	status = Cam_SendData(drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_msb, highByte);
	if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_lsb, lowByte);

	ES_LOG("Camera 3010, ADC test pattern MSB: %u, LSB: %u\n", highByte, lowByte);
	return status;
}

/**
 * \brief Init routine for Camera System 3030.
 *
 * 	Sets registers in ADC ADS5294.
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_Init(uint32_t drvno)
{
	ES_LOG("Init camera 3030\n");
	es_status_codes status = Cam3030_ADC_reset(drvno);
	if (status != es_no_error) return status;
	//two wire mode output interface for pal versions P209_2 and above
	status = Cam3030_ADC_twoWireModeEN(drvno);
	if (status != es_no_error) return status;
	status = Cam3030_ADC_SetGain(drvno, (uint8_t)settings_struct.camera_settings[drvno].adc_gain);
	if (status != es_no_error) return status;
	if (settings_struct.camera_settings[drvno].adc_mode)
		status = Cam3030_ADC_RampOrPattern(drvno, (uint8_t)settings_struct.camera_settings[drvno].adc_mode, (uint16_t)settings_struct.camera_settings[drvno].adc_custom_pattern);
	if (status != es_no_error) return status;
	for (uint32_t camera = 0; camera < settings_struct.camera_settings[drvno].camcnt; camera++)
	{
		status = DAC8568_enableInternalReference(drvno, DAC8568_camera, camera);
		if (status != es_no_error) return status;
		bool is_hs_ir = false;
		if (settings_struct.camera_settings[drvno].sensor_type == sensor_type_hsir)
			is_hs_ir = true;
		status = DAC8568_setAllOutputs(drvno, DAC8568_camera, camera, settings_struct.camera_settings[drvno].dac_output[camera], !is_hs_ir);
		if (status != es_no_error) return status;
	}
	// Sample mode is currently not in use. 11/22, P209_8
	status = Cam3030_ADC_SetSampleMode(drvno, 0);
	return status;
}

/**
 * \brief ADC reset routine for Camera System 3030.
 *
 * 	Resets register of ADC ADS5294 to default state (output interface is 1 wire!).
 * 	Called by Cam3030_Init
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_reset(uint32_t drvno)
{
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_reset, adc_ads5294_msg_reset);
}

/**
 * \brief ADC output interface config routine for Camera System 3030.
 *
 * 	Enables two wire LVDS data transfer mode of ADC ADS5294.
 * 	Only works with PAL versions P209_2 and above.
 * 	Called by Cam3030_Init - comment for older versions and rebuild
 * 	or use on e-lab test computer desktop LabView folder lv64hs (bool switch in 3030 init tab)
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_twoWireModeEN(uint32_t drvno)
{
	es_status_codes status = Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_2wireMode, adc_ads5294_msg_2wireMode);
	if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_wordWiseOutput, adc_ads5294_msg_wordWiseOutput);
	if (status != es_no_error) return status;
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_ddrClkAlign, adc_ads5294_msg_ddrClkAlign);
}

/**
 * \brief ADC gain config routine for Camera System 3030.
 *
 * 	Sets gain of ADC ADS5294 0...15 by calling SetADGain() subroutine.
 * 	Called by Cam3030_Init
 * \param drvno selects PCIe board
 * \param gain of ADC
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes Cam3030_ADC_SetGain(uint32_t drvno, uint8_t gain)
{
	return SetADGain(drvno, 1, gain, gain, gain, gain, gain, gain, gain, gain);
}

/**
 * \brief Set gain for ADS5294.
 *
 * \param fkt =0 reset to db=0, fkt=1 set to g1..g8
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param g1 channel 1
 * \param g2 channel 2
 * \param g3 channel 3
 * \param g4 channel 4
 * \param g5 channel 5
 * \param g6 channel 6
 * \param g7 channel 7
 * \param g8 channel 8
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes SetADGain(uint32_t drvno, uint8_t fkt, uint8_t g1, uint8_t g2, uint8_t g3, uint8_t g4, uint8_t g5, uint8_t g6, uint8_t g7, uint8_t g8)
{
	uint16_t data = 0;
	uint8_t a, b, c, d, e, f, g, h;
	a = 0;
	b = 0;
	c = 0;
	d = 0;
	e = 0;
	f = 0;
	g = 0;
	h = 0;
	if (fkt != 0)
	{
		a = g1;  //values 0..f
		b = g2;
		c = g3;
		d = g4;
		e = g5;
		f = g6;
		g = g7;
		h = g8;
	}
	data = a;
	data = (uint16_t)(data << 4);
	data |= e;
	data = (uint16_t)(data << 4);
	data |= b;
	data = (uint16_t)(data << 4);
	data |= f;
	es_status_codes status = Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_gain_1_to_4, data);	//gain1..4
	if (status != es_no_error) return status;
	data = h;
	data = (uint16_t)(data << 4);
	data |= d;
	data = (uint16_t)(data << 4);
	data |= g;
	data = (uint16_t)(data << 4);
	data |= c;
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_gain_5_to_8, data);	//gain7..8
}

/**
 * \brief ADC debug mode for Camera System 3030.
 *
 * Lets ADC send a ramp or a custom pattern (value) instead of ADC sample data.
 * Called by Cam3030_Init when adc_mode > 0.
 * \param drvno selects PCIe board
 * \param adc_mode 1: ramp, 2: custom pattern
 * \param custom_pattern (only used when adc_mode = 2)
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_RampOrPattern(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern)
{
	es_status_codes status = es_no_error;
	switch (adc_mode)
	{
	case 1: //ramp
		status = Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_ramp);
		break;
	case 2: //custom pattern
		//to activate custom pattern the following messages are necessary: d - data
		//at address 0x25 (mode and higher bits): 0b00000000000100dd
		status = Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_custompattern | ((custom_pattern >> 12) & 0x3));
		if (status != es_no_error) return status;
		//at address 0x26 (lower bits): 0bdddddddddddd0000
		status = Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_custompattern, (uint16_t)(custom_pattern << 4));
		break;
	default:
		break;
	}
	return status;
}

/**
 * \brief Enable or disable filters for all 8 channels
 *
 * Global enable must be set to true, if you want to use at least one filter. Filters can be enabled / disabled separately by Cam3030_ADC_SetFilter(). When the global filter enable is true, all channels are either passed through the filter or through a dummy delay so that the overall latency of all channels is 20 clock cycles.
 * \param drvno selects PCIe board
 * \param enable true:
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_Global_En_Filter(uint32_t drvno, bool enable)
{
	ES_LOG("Cam3030_ADC_Global_En_Filter: %u\n", enable);
	uint16_t payload = 0;
	// the global_en_filter bit is on bit 1 of register 0x29
	if (enable) payload = 1 << 1;
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_global_en_filter, payload);
}

/**
 * \brief Set all parameters for one filter determined by channel.
 *
 * To use a filter, the global enable must be set to true by Cam3030_ADC_Global_En_Filter().
 * \param drvno selects PCIe board
 * \param channel Channel to which the filter parameters should be applied. 1...8
 * \param coeff_set Select stored coefficient set.
 * \param decimation_factor Set decimation factor aka FILTER_RATE.
 *		- 0x00 decimate by 2
 *		- 0x01 decimate by 4
 *		- 0x04 decimate by 8
 * \param odd_tap 1: Use odd tap filter. 0: even tap
 * \param use_filter 1: enable filter, 0: disable filter
 * \param hpf_corner high pass filter corner in values k from 2 to 10
 * \param en_hpf 1: high pass filter enabled, 0: disabled
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 *		- es_parameter_out_of_range
 */
es_status_codes Cam3030_ADC_SetFilterSettings(uint32_t drvno, uint8_t channel, uint8_t coeff_set, uint8_t decimation_factor, uint8_t odd_tap, uint8_t use_filter, uint8_t hpf_corner, uint8_t en_hpf)
{
	ES_TRACE("Cam3030_ADC_SetFilterSettings(), setting channel %u, coeff_set %u, decimation_factor %u, odd_tap %u, use_filter %u, hpf_corner %u, en_hpf %u\n", channel, coeff_set, decimation_factor, odd_tap, use_filter, hpf_corner, en_hpf);
	uint16_t payload = (use_filter & 1) | (odd_tap & 1) << 2 | (decimation_factor & 7) << 4 | (coeff_set & 7) << 7 | (hpf_corner & 7) << 10 | (en_hpf & 1) << 14;
	if (channel > 8 || channel < 1) return es_parameter_out_of_range;
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_filter1 + channel - 1, payload);
}

/**
 * \param drvno selects PCIe board
 * \param channel 1...8
 * \param coefficient_number 0...11
 * \param enable 0: disable, 1: enable
 * \param coefficient 12 bit signed value
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 *		- es_parameter_out_of_range
 */
es_status_codes Cam3030_ADC_SetFilterCustomCoefficient(uint32_t drvno, uint8_t channel, uint8_t coefficient_number, uint8_t enable, uint16_t coefficient)
{
	ES_TRACE("Cam3030_ADC_SetFilterCustomCoefficient(), setting channel %u, coefficient %u to %u\n", channel, coefficient_number, coefficient);
	uint16_t payload = (coefficient & 0xFFF) | (enable & 1) << 15;
	if (channel > 8 || channel < 1 || coefficient_number > 11) return es_parameter_out_of_range;
	uint8_t address = adc_ads5294_regaddr_coeff0_filter1 + coefficient_number + (channel - 1) * 12;
	return Cam_SendData(drvno, maddr_adc, address, payload);
}

/**
 * \brief Set the data rate of the ADC output.
 *
 * Data rate specifies the ratio between ADC sampling rate and how many digital output values are generated.
 * \param drvno selects PCIe board
 * \param data_rate
 *		- 0: All converted values at the ADC sampling rate are shown on the digital output
 *		- 1: 1/2 of ADC sampling rate
 *		- 2: 1/4 of ADC sampling rate
 *		- 3: 1/8 of ADC sampling rate
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_SetDataRate(uint32_t drvno, uint8_t data_rate)
{
	ES_TRACE("Cam3030_ADC_SetDataRate(), setting data rate to %u\n", data_rate);
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_data_rate, data_rate & 0x3);
}

/**
 * \brief Enables or disables low frequency noise suppression mode.
 *
 * \param drvno selects PCIe board
 * \param enable
 *		- true: enable noise suppression mode
 *		- false: disable noise suppression mode
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_SetLFNS(uint32_t drvno, bool enable)
{
	ES_TRACE("Cam3030_ADC_SetLFNS(), Enable %u\n", enable);
	uint16_t payload;
	if (enable)
		payload = 0xFF;
	else
		payload = 0;
	return Cam_SendData(drvno, maddr_adc, adc_ads5294_regaddr_LFNSM, payload);
}

/**
 * \brief Currently not in use. 11/22, P209_8. Set over how many samples of one pixel the ADC averages.
 *
 * \param drvno selects PCIe board
 * \param sample_mode:
 *		- 0: 1 sample per pixel (default), adc clk = sen clk = adc data rate = 25MHz
 *		- 1: average over 2 samples per pixel, adc clk = 50MHz,  sen clk = adc data rate = 12,5Mhz. Notice here: 4 samples are taken during 1 pixel, but the first two samples are thrown away, because the video signal has not reached it's high during sampling time. The "throwing away" is done with the ADC filters.
 *		- 2: 1 sample per pixel, adc clk = 25 MHz, 12,5 MHz. 2 samples are taken during 1 pixel, but one is thrown away.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_SetSampleMode(uint32_t drvno, uint8_t sample_mode)
{
	ES_LOG("Cam3030_ADC_SetSampleMode(), setting sample mode to %u\n", sample_mode);
	// send the sample mode to the camera
	es_status_codes status = Cam_SendData(drvno, maddr_cam, cam_adaddr_sample_mode, sample_mode);
	// When sample mode is 0, the camera should work the same like before this feature was implemented.
	if (sample_mode == 0)
	{
		status = Cam3030_ADC_Global_En_Filter(drvno, 0);
		if (status != es_no_error) return status;
		status = Cam3030_ADC_SetDataRate(drvno, 0);
	}
	// When sample mode is not 0, more complicated stuff is done here. Filters in the ADC are activated and set.
	else
	{
		uint8_t active_coefficients;
		// coefficient_value * active_coefficients * 2 must equal 2048 to achieve averaging
		// see equation 1 on page 41 of ADS5294 datasheet for reference
		uint16_t coefficient_value;
		// when active_coefficients is even, set odd_tap to 0, when active_coefficients is odd, set odd_tap to 1
		uint8_t odd_tap;
		switch (sample_mode)
		{
		case 1:
			status = Cam3030_ADC_Global_En_Filter(drvno, 1);
			if (status != es_no_error) return status;
			active_coefficients = 1;
			odd_tap = 0;
			coefficient_value = 1024;
			status = Cam3030_ADC_SetDataRate(drvno, 2);
			break;
		case 2:
			// disable filter, because only one sample is used in sample mode 2
			status = Cam3030_ADC_Global_En_Filter(drvno, 0);
			if (status != es_no_error) return status;
			status = Cam3030_ADC_SetDataRate(drvno, 1);
			// Return this function here, because the whole coefficient setting thing is not needed for this case. This is not good code style though.
			return status;
			break;
			// More cases are possible, but not implemented yet.
			//case ?:
			//	active_coefficients = 2;
			//	coefficient_value = 512;
			//	break;
			//case ?:
			//	active_coefficients = 4;
			//	coefficient_value = 256;
			//	break;
		default:
			return es_parameter_out_of_range;
		}
		if (status != es_no_error) return status;
		// The number of ADC channels are given by the ADC and cannot be changed.
		uint8_t number_of_adc_channels = 8;
		// Only 3 filter channels are usable, because of the current address implementation in the FPGA. This could be deleted, when the implementation in the FPGA would be correct.
		uint8_t unusable_adc_channels = 5;
		// The number of coefficients is given by the ADC internals and cannot be changed.
		uint8_t number_of_coefficients = 12;
		// Coefficient sets are unused, because only custom coefficients are used.
		uint8_t coeff_set = 0;
		// The decimation factor reduces the output of data per channel. This function is unused and instead the global reduction per Cam3030_ADC_SetDataRate is used.
		uint8_t decimation_factor = 0;
		// Always enable filters and coefficients. Disabling is done with Cam3030_ADC_Global_En_Filter.
		uint8_t enable = 1;
		// high pass filter is not used
		uint8_t hpf_corner = 0;
		uint8_t en_hpf = 0;

		for (uint8_t channel = 1; channel <= number_of_adc_channels - unusable_adc_channels; channel++)
		{
			status = Cam3030_ADC_SetFilterSettings(drvno, channel, coeff_set, decimation_factor, odd_tap, enable, hpf_corner, en_hpf);
			if (status != es_no_error) return status;
			for (uint8_t coefficient = 0; coefficient < number_of_coefficients; coefficient++)
			{
				if (coefficient >= number_of_coefficients - active_coefficients)
					status = Cam3030_ADC_SetFilterCustomCoefficient(drvno, channel, coefficient, enable, coefficient_value);
				else
					status = Cam3030_ADC_SetFilterCustomCoefficient(drvno, channel, coefficient, enable, 0);
				if (status != es_no_error) return status;
			}
		}

		// Disable filters of unusable channels.
		enable = 0;
		for (uint8_t channel = number_of_adc_channels - unusable_adc_channels + 1; channel <= number_of_adc_channels; channel++)
			status = Cam3030_ADC_SetFilterSettings(drvno, channel, coeff_set, decimation_factor, odd_tap, enable, hpf_corner, en_hpf);
	}
	return status;
}

/**
 * \brief Sets the sensor reset length register in the camera, which controls the length of the ARG pulse.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sensor_reset_or_hsir_ec See \ref camera_register_addresses_t.cam_adaddr_sensor_reset_length for more information.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_SetSensorResetOrHsirEc(uint32_t drvno, uint16_t sensor_reset_or_hsir_ec)
{
	ES_LOG("Cam_SetSensorResetOrHsirEc(), setting sensor reset length to %u (HSVIS: %u ns, HSIR: %u ns)\n", sensor_reset_or_hsir_ec, sensor_reset_or_hsir_ec * 4, sensor_reset_or_hsir_ec * 160);
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_sensor_reset_length, sensor_reset_or_hsir_ec);
}

/**
 * \brief Set temperature level for cooled cameras.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param level level 0..7 / 0=off, 7=min -> see cooling manual
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_SetTemp(uint32_t drvno, uint8_t level)
{
	if (level >= 8) level = 0;
	ES_LOG("Set temperature level: %u\n", level);
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_coolTemp, level);
}

/**
 * \brief Sends data via fiber link, e.g. used for sending data to ADC (ADS5294).
 *
 * Send setup:
 * - d0:d15 = data for AD-Reg  ADS5294
 * - d16:d23 = ADR of  AD-Reg
 * - d24 = ADDR0		AD=1
 * - d25 = ADDR1		AD=0
 * - d26 makes load pulse
 * - all written to DB0 in Space0 = Long0
 * - for AD set maddr=01, adaddr address of reg
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param maddr master address for specifying device (2 for ADC)
 * \param adaddr register address
 * \param data data
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_SendData(uint32_t drvno, uint8_t maddr, uint8_t adaddr, uint16_t data)
{
	ES_TRACE("Cam_SendData(): maddr 0x%x, adaddr: 0x%x, data 0x%x (%u)\n", maddr, adaddr, data, data);
	es_status_codes status = FindCam(drvno);
	if (status != es_no_error) return status;
	uint32_t ldata = 0;
	ldata = maddr;
	ldata = ldata << 8;
	ldata |= adaddr;
	ldata = ldata << 16;
	ldata |= data;
	status = writeRegisterS0_32(drvno, ldata, S0Addr_DBR);
	if (status != es_no_error) return status;
	WaitforTelapsed(500);
	//load val
	ldata |= 0x4000000;
	status = writeRegisterS0_32(drvno, ldata, S0Addr_DBR);
	if (status != es_no_error) return status;
	WaitforTelapsed(500);
	//rs load
	ldata = 0;
	return writeRegisterS0_32(drvno, ldata, S0Addr_DBR);
}

/**
 * \brief Is basically the old vclk register (cam_adaddr_vclk). Sets:
 * - en_area	(bit 15)		- as before
 * - vfreq		(bits 14...1)	- for cams with FFT_v2 FPGAs, that generate their own vclks inside the cam
 * - is_fft		(bit 0)			- as before but legacy to run FFT_v1 cameras
 */
es_status_codes Cam_SetVfreqRegister(uint32_t drvno) // was vclk register
{
	es_status_codes status = es_no_error;
	uint16_t is_area_mode = 0;
	if (settings_struct.camera_settings[drvno].fft_mode == area_mode) is_area_mode = 0x8000;
	uint16_t is_fft;
	if (settings_struct.camera_settings[drvno].sensor_type == sensor_type_fft)
		is_fft = 1;
	else
		is_fft = 0;
	uint16_t vfreqFPGA = (uint16_t)settings_struct.camera_settings[drvno].vfreq << 5; //multiplikation mit 32 damit FPGA vclks aehnlich lang wie die PCIe vclks sind
	uint16_t vfreqRegsiter = (is_area_mode | (vfreqFPGA & 0x3FFF) << 1 | is_fft);
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclk, vfreqRegsiter);
	return status;
}

es_status_codes Cam_SetupFullBinning(uint32_t drvno)
{
	es_status_codes status = es_no_error;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount1, (uint16_t)settings_struct.camera_settings[drvno].fft_lines); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount2, (uint16_t)0x0000); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount3, (uint16_t)0x0000); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount4, (uint16_t)0x0000); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount5, (uint16_t)0x0000); if (status != es_no_error) return status;
	return status;
}

es_status_codes Cam_SetupPartialBinning(uint32_t drvno)
{
	es_status_codes status = es_no_error;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount1, (uint16_t)settings_struct.camera_settings[drvno].region_size[0]); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount2, (uint16_t)settings_struct.camera_settings[drvno].region_size[1]); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount3, (uint16_t)settings_struct.camera_settings[drvno].region_size[2]); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount4, (uint16_t)settings_struct.camera_settings[drvno].region_size[3]); if (status != es_no_error) return status;
	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_vclks_amount5, (uint16_t)settings_struct.camera_settings[drvno].region_size[4]); if (status != es_no_error) return status;
	return status;
}

/**
 * \brief Disables all camera leds to suppress stray light.
 *
 * 	Sets corresponding camera register: maddr = 0, adadr = 5;
 * \param drvno selects PCIe board
 * \param LED_OFF 1 -> leds off, 0 -> led on
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_SetLedOff(uint32_t drvno, uint8_t LED_OFF)
{
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_LEDoff, (uint16_t)LED_OFF);
}
/**
 * \brief This functions sets the camera position register of the first camera to 0.
 *
 * When there are more cameras in line, the cameras are handing their positions one to another.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_SetPosition(uint32_t drvno)
{
	ES_LOG("Set camera position of first camera in row to 1\n");
	// 0x8000 is a test value. Camera position is set to 0
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_camera_position, 0x8000);
}

/**
 * \brief Set the pixel where IOCtrl starts inserting its data.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param startPixel Position of IOCtrl data in pixel
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes CamIOCtrl_setImpactStartPixel(uint32_t drvno, uint16_t startPixel)
{
	ES_LOG("Set IOCtrl impact start pixel: %u\n", startPixel);
	return Cam_SendData(drvno, maddr_ioctrl, ioctrl_impact_start_pixel, startPixel);
}

/**
 * \brief Set paramters of one pulse output of IOCTRL.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param number Number of output: 1 ... 7
 * \param width_in_5ns Set width of pulse in 5ns steps.
 * \param delay_in_5ns Set delay of pulse in 5ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes CamIOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	ES_LOG("Set IOCtrl output %u, width %u, delay %u\n", number, width_in_5ns, delay_in_5ns);
	uint8_t addrWidth = 0;
	uint8_t addrDelay = 0;
	switch (number)
	{
	case 1:
		addrWidth = ioctrl_t1;
		addrDelay = ioctrl_d1;
		break;
	case 2:
		addrWidth = ioctrl_t2;
		addrDelay = ioctrl_d2;
		break;
	case 3:
		addrWidth = ioctrl_t3;
		addrDelay = ioctrl_d3;
		break;
	case 4:
		addrWidth = ioctrl_t4;
		addrDelay = ioctrl_d4;
		break;
	case 5:
		addrWidth = ioctrl_t5;
		addrDelay = ioctrl_d5;
		break;
	case 6:
		addrWidth = ioctrl_t6;
		addrDelay = ioctrl_d6;
		break;
	case 7:
		addrWidth = ioctrl_t7;
		addrDelay = ioctrl_d7;
		break;
	default:
		return es_parameter_out_of_range;
	}
	es_status_codes status = Cam_SendData(drvno, maddr_ioctrl, addrWidth, width_in_5ns);
	if (status != es_no_error) return status;
	return Cam_SendData(drvno, maddr_ioctrl, addrDelay, delay_in_5ns);
}

/**
 * \brief Set parameters of all pulses output of IOCTRL.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param width_in_5ns Set width of pulse in 5ns steps. Array with 7 entries.
 * \param delay_in_5ns Set delay of pulse in 5ns steps. Array with 7 entries.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes CamIOCtrl_setAllOutputs(uint32_t drvno, uint32_t* width_in_5ns, uint32_t* delay_in_5ns)
{
	es_status_codes status = es_no_error;
	for (uint8_t i = 0; i <= 6; i++)
	{
		status = CamIOCtrl_setOutput(drvno, i + 1, (uint16_t)width_in_5ns[i], (uint16_t)delay_in_5ns[i]);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Set period of IOCtrl pulse outputs base frequency T0.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param period_in_10ns Period of T0 in 10ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes CamIOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns)
{
	ES_LOG("Set IOCtrl T0 period %u\n", period_in_10ns);
	uint16_t period_in_10ns_L = (uint16_t)period_in_10ns;
	uint16_t period_in_10ns_H = (uint16_t)(period_in_10ns >> 16);
	es_status_codes status = Cam_SendData(drvno, maddr_ioctrl, ioctrl_t0h, period_in_10ns_H);
	if (status != es_no_error) return status;
	return Cam_SendData(drvno, maddr_ioctrl, ioctrl_t0l, period_in_10ns_L);
}

es_status_codes Cam_DAC8568_sendData(uint32_t drvno, uint32_t data, uint8_t cameraPosition)
{
	uint16_t hi_bytes = (uint16_t)(data >> 16);
	uint16_t lo_bytes = (uint16_t)data;
	uint8_t adaddr = dac_hi_byte_addr | cameraPosition << campos_bit_index;
	es_status_codes status = Cam_SendData(drvno, maddr_dac, adaddr, hi_bytes);
	if (status != es_no_error) return status;
	adaddr = dac_lo_byte_addr | cameraPosition << campos_bit_index;
	status = Cam_SendData(drvno, maddr_dac, adaddr, lo_bytes);
	return status;
}

es_status_codes Cam_SetPixelRegister(uint32_t drvno)
{
	ES_LOG("Set pixel register in camera to %u\n", settings_struct.camera_settings[drvno].pixel);
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_pixel, (uint16_t)settings_struct.camera_settings[drvno].pixel);
}

es_status_codes Cam_SetTriggerInput(uint32_t drvno)
{
	ES_LOG("Set trigger input in camera to %u\n", settings_struct.camera_settings[drvno].trigger_mode_integrator);
	return Cam_SendData(drvno, maddr_cam, cam_adaddr_trig_in, (uint16_t)settings_struct.camera_settings[drvno].trigger_mode_integrator);
}

es_status_codes Cam_SetConfigRegister(uint32_t drvno)
{
	es_status_codes status = es_no_error;

	uint16_t sensor_gain = ((uint16_t)settings_struct.camera_settings[drvno].sensor_gain << cam_config_register_bitindex_sensor_gain) & cam_config_register_bits_sensor_gain;
	uint16_t trigger_mode = ((uint16_t)settings_struct.camera_settings[drvno].trigger_mode_integrator << cam_config_register_bitindex_trigger_mode_cc) & cam_config_register_bits_trigger_mode_cc;
	uint16_t cool_level = ((uint8_t)settings_struct.camera_settings[drvno].temp_level << cam_config_register_bitindex_temp_level) & cam_config_register_bits_temp_level;
	uint16_t led_off = ((uint16_t)settings_struct.camera_settings[drvno].led_off << cam_config_register_bitindex_led_off) & cam_config_register_bits_led_off;
	uint16_t bnc_out = ((uint16_t)settings_struct.camera_settings[drvno].bnc_out << cam_config_register_bitindex_bnc_out) & cam_config_register_bits_bnc_out;
	uint16_t channel_select = 0;
	switch (settings_struct.camera_settings[drvno].channel_select)
	{
	case channel_select_A:
		channel_select = cam_config_register_bit_channel_select_a;
		break;
	case channel_select_B:
		channel_select = cam_config_register_bit_channel_select_b;
		break;
	default:
	case channel_select_A_B:
		channel_select = cam_config_register_bit_channel_select_a | cam_config_register_bit_channel_select_b;
		break;
	}
	uint16_t sensor_gain_2 = ((uint16_t)settings_struct.camera_settings[drvno].sensor_gain << (cam_config_register_bitindex_sensor_gain_2 - 1)) & cam_config_register_bit_sensor_gain_2;
	uint16_t configRegister = bnc_out | led_off | cool_level | trigger_mode | sensor_gain | channel_select | sensor_gain_2;

	status = Cam_SendData(drvno, maddr_cam, cam_adaddr_config, configRegister);
	if (status != es_no_error) return status;
	return status;
}

es_status_codes Cam_SetupFFT(uint32_t drvno)
{
	es_status_codes status = es_no_error;
	status = Cam_SetVfreqRegister(drvno); if (status != es_no_error) return status;

	switch (settings_struct.camera_settings[drvno].fft_mode)
	{
	case full_binning:
		status = Cam_SetupFullBinning(drvno);
		break;
	case partial_binning:
	{
		status = Cam_SetupPartialBinning(drvno);
		break;
	}
	case area_mode:
		break;
	default:
		return es_parameter_out_of_range;
	}

	return status;
}
