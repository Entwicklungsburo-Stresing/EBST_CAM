#include "Board.h"
#include "../shared_src/UIAbstractionLayer.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <math.h> // for sqrt()
#include <stdio.h>
#ifdef __linux__
#define sprintf_s snprintf
#endif

#ifdef _USRDLL
#define DLLTAB "\t"
#else
#define DLLTAB
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

/**
 * \brief Set global settings struct.
 * 
 * Call this every time you changed settings before InitMeasurement. 
 * \param settings struct measurement_settings
 */
void SetGlobalSettings(struct measurement_settings settings)
{
	settings_struct = settings;
}

/**
 * \brief Initialize measurement (using board select).
 * 
 * Call this every time you changed settings before starting the measurement. When you didn't change any settings, you can start the next measurement without calling InitMeasurement everytime.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_parameter_out_of_range
 *		- es_allocating_memory_failed
 *		- es_not_enough_ram
 *		- es_getting_dma_buffer_failed
 *		- es_enabling_interrupts_failed
 *		- es_camera_not_found
 */
es_status_codes InitMeasurement()
{
	ES_LOG("\n*** Init Measurement ***\n");
	ES_LOG("struct global_settings: ");
	for (uint32_t i = 0; i < sizeof(settings_struct)/4; i++)
		ES_LOG("%u ", *(&settings_struct.board_sel + i));
	ES_LOG("\n");
	es_status_codes status = es_camera_not_found;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = _InitMeasurement(drvno);
			if (status != es_no_error) return status;
		}
	}
	ES_LOG("*** Init Measurement done ***\n\n");
	return status;
}

void SetGlobalVariables(uint32_t drvno)
{
	ES_LOG("Set global variables\n");
	*Nob = settings_struct.nob;
	*Nospb = settings_struct.nos;
	aPIXEL[drvno] = settings_struct.camera_settings[drvno].pixel;
	if (settings_struct.camera_settings[drvno].camcnt)
		aCAMCNT[drvno] = settings_struct.camera_settings[drvno].camcnt;
	else
		// if camcnt = 0, treat as camcnt = 1, but write 0 to register
		aCAMCNT[drvno] = 1;
	return;
}

es_status_codes InitSoftware(uint32_t drvno)
{
	ES_LOG("\nInit software for board %u\n", drvno);
	if (settings_struct.nos < 2 || settings_struct.nob < 1) return es_parameter_out_of_range;
	SetGlobalVariables(drvno);
	abortMeasurementFlag = false;
	es_status_codes status = allocateUserMemory(drvno);
	if (status != es_no_error) return status;
	uint32_t dmaBufferPartSizeInScans = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS; //500
	if(dmaBufferPartSizeInScans)
		numberOfInterrupts[drvno] = (*Nob * (*Nospb) * aCAMCNT[drvno]) / dmaBufferPartSizeInScans;
	ES_LOG("Number of interrupts: %u \n", numberOfInterrupts[drvno]);
	if (settings_struct.camera_settings[drvno].use_software_polling)
		status = disableInterrupt(drvno);
	else
		status = enableInterrupt(drvno);
	if (status != es_no_error) return status;
	status = SetupDma(drvno);
	if (status != es_no_error) return status;
	return status;
}

es_status_codes InitPcieBoard(uint32_t drvno)
{
	ES_LOG("\nInit hardware board %u\n", drvno);
	es_status_codes status = StopSTimer(drvno);
	if (status != es_no_error) return status;
	status = RSFifo(drvno);
	if (status != es_no_error) return status;
	status = SetDMABufRegs(drvno);
	if (status != es_no_error) return status;
	status = ClearAllUserRegs(drvno);
	if (status != es_no_error) return status;
	status = SetNosRegister(drvno);
	if (status != es_no_error) return status;
	status = SetNobRegister(drvno);
	if (status != es_no_error) return status;
	status = SetPixelCountRegister(drvno);
	if (status != es_no_error) return status;
	status = SetCamCountRegister(drvno);
	if (status != es_no_error) return status;
	status = SetCameraSystem(drvno, (uint16_t)settings_struct.camera_settings[drvno].camera_system);
	if (status != es_no_error) return status;
	status = SetSensorType(drvno, (uint16_t)settings_struct.camera_settings[drvno].sensor_type);
	if (status != es_no_error) return status;
	if (settings_struct.camera_settings[drvno].sensor_type == sensor_type_fft)
	{
		switch (settings_struct.camera_settings[drvno].fft_mode)
		{
		case full_binning:
			status = SetupFullBinning(drvno, settings_struct.camera_settings[drvno].fft_lines, (uint8_t)settings_struct.camera_settings[drvno].vfreq);
			break;
		case partial_binning:
		{
			uint8_t regionSize[8];
			for (int i = 0; i < 8; i++) regionSize[i] = (uint8_t)settings_struct.camera_settings[drvno].region_size[i];
			// keep is a deprecated setting. all bits are set to 1
			status = SetupROI(drvno, (uint16_t)settings_struct.camera_settings[drvno].number_of_regions, settings_struct.camera_settings[drvno].fft_lines, 0xFF, regionSize, (uint8_t)settings_struct.camera_settings[drvno].vfreq);
			break;
		}
		case area_mode:
			status = SetupArea(drvno, settings_struct.camera_settings[drvno].lines_binning, (uint8_t)settings_struct.camera_settings[drvno].vfreq);
			break;
		default:
			return es_parameter_out_of_range;
		}
	}
	else *useSWTrig = false;
	if (status != es_no_error) return status;
	status = SetSSlope(drvno, settings_struct.camera_settings[drvno].sslope);
	if (status != es_no_error) return status;
	status = SetBSlope(drvno, settings_struct.camera_settings[drvno].bslope);
	if (status != es_no_error) return status;
	status = SetSTI(drvno, (uint8_t)settings_struct.camera_settings[drvno].sti_mode);
	if (status != es_no_error) return status;
	status = SetBTI(drvno, (uint8_t)settings_struct.camera_settings[drvno].bti_mode);
	if (status != es_no_error) return status;
	status = SetSTimer(drvno, settings_struct.camera_settings[drvno].stime_in_microsec);
	if (status != es_no_error) return status;
	status = SetBTimer(drvno, settings_struct.camera_settings[drvno].btime_in_microsec);
	if (status != es_no_error) return status;
	bool isTdc = false;
	status = GetIsTdc(drvno, &isTdc);
	if (status != es_no_error) return status;
	if (isTdc) status = InitGPX(drvno, settings_struct.camera_settings[drvno].gpx_offset);
	if (status != es_no_error) return status;
	status = SetSDAT(drvno, settings_struct.camera_settings[drvno].sdat_in_10ns);
	if (status != es_no_error) return status;
	status = SetBDAT(drvno, settings_struct.camera_settings[drvno].bdat_in_10ns);
	if (status != es_no_error) return status;
	continiousPauseInMicroseconds = settings_struct.cont_pause_in_microseconds;
	ES_LOG("Setting continuous pause to %u\n", continiousPauseInMicroseconds);
	status = SetBEC(drvno, settings_struct.camera_settings[drvno].bec_in_10ns);
	if (status != es_no_error) return status;
	status = SetXckdelay(drvno, settings_struct.camera_settings[drvno].xckdelay_in_10ns);
	if (status != es_no_error) return status;
	status = SetHardwareTimerStopMode(drvno, true);
	if (status != es_no_error) return status;
	status = SetDmaRegister(drvno, settings_struct.camera_settings[drvno].pixel);
	if (status != es_no_error) return status;
	status = SetDmaStartMode(drvno, HWDREQ_EN);
	if (status != es_no_error) return status;
	status = SetTicnt(drvno, (uint8_t)settings_struct.camera_settings[drvno].ticnt);
	if (status != es_no_error) return status;
	status = SetTocnt(drvno, (uint8_t)settings_struct.camera_settings[drvno].tocnt);
	if (status != es_no_error) return status;
	status = SetSEC(drvno, settings_struct.camera_settings[drvno].sec_in_10ns);
	if (status != es_no_error) return status;
	status = SetTORReg(drvno, (uint8_t)settings_struct.camera_settings[drvno].tor);
	if (status != es_no_error) return status;
	status = SetS1S2ReadDelay(drvno);
	return status;
}

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
es_status_codes InitCamera(uint32_t drvno)
{
	ES_LOG("\nInit camera %u\n", drvno);
	es_status_codes status = FindCam(drvno);
	if (status != es_no_error) return status;
	status = SetCameraPosition(drvno);
	if (status != es_no_error) return status;
	// when cooled camera: disable PCIe FIFO when cool cam transmits cool status (legacy code for old cameras)
	if (settings_struct.camera_settings[drvno].is_cooled_cam)
		status = Use_ENFFW_protection(drvno, true);
	else
		status = Use_ENFFW_protection(drvno, false);
	//set camera pixel register
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_pixel, (uint16_t)settings_struct.camera_settings[drvno].pixel);
	if (status != es_no_error) return status;
	//set trigger input of CamControl
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_trig_in, (uint16_t)settings_struct.camera_settings[drvno].trigger_mode_cc);
	if (status != es_no_error) return status;
	//set led off
	status = SetLedOff(drvno, (uint8_t)settings_struct.camera_settings[drvno].led_off);
	if (status != es_no_error) return status;
	//set gain switch (mostly for IR sensors)
	status = SetConfigRegister(drvno);
	if (status != es_no_error) return status;

	if (settings_struct.camera_settings[drvno].sensor_type == sensor_type_fft)
	{
		status = SetupFFT(drvno);
		if (status != es_no_error) return status;
	}

	if (status != es_no_error) return status;
	switch (settings_struct.camera_settings[drvno].camera_system)
	{
	case camera_system_3001:
		status = InitCamera3001(drvno);
		break;
	case camera_system_3010:
		status = InitCamera3010(drvno, (uint8_t)settings_struct.camera_settings[drvno].adc_mode, (uint16_t)settings_struct.camera_settings[drvno].adc_custom_pattern);
		break;
	case camera_system_3030:
		status = InitCamera3030(drvno);
		break;
	default:
		return es_parameter_out_of_range;
	}
	if (status != es_no_error) return status;
	//for cooled Cam
	status = SetTemp(drvno, (uint8_t)settings_struct.camera_settings[drvno].temp_level);
	if (status != es_no_error) return status;
	status = IOCtrl_setImpactStartPixel(drvno, (uint16_t)settings_struct.camera_settings[drvno].ioctrl_impact_start_pixel);
	if (status != es_no_error) return status;
	for (uint8_t i = 1; i <= 7; i++)
	{
		status = IOCtrl_setOutput(drvno, i, (uint16_t)settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[i - 1], (uint16_t)settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[i - 1]);
		if (status != es_no_error) return status;
	}
	status = IOCtrl_setT0(drvno, settings_struct.camera_settings[drvno].ioctrl_T0_period_in_10ns);
	if (status != es_no_error) return status;
	status = Cam_SetSensorResetLength(drvno, settings_struct.camera_settings[drvno].sensor_reset_length_in_4_ns);
	return status;
}


es_status_codes SetConfigRegister(uint32_t drvno)
{
	es_status_codes status = es_no_error;

	uint16_t sensor_gain = ((uint16_t)settings_struct.camera_settings[drvno].sensor_gain << cam_config_register_bitindex_sensor_gain) & cam_config_register_bits_sensor_gain;
	uint16_t trigger_mode = ((uint16_t)settings_struct.camera_settings[drvno].trigger_mode_cc << cam_config_register_bitindex_trigger_mode_cc) & cam_config_register_bits_trigger_mode_cc;
	uint16_t cool_level = ((uint8_t)settings_struct.camera_settings[drvno].temp_level << cam_config_register_bitindex_temp_level) & cam_config_register_bits_temp_level;
	uint16_t led_off = ((uint16_t)settings_struct.camera_settings[drvno].led_off << cam_config_register_bitindex_led_off) & cam_config_register_bits_led_off;
	uint16_t bnc_out = ((uint16_t)settings_struct.camera_settings[drvno].bnc_out << cam_config_register_bitindex_bnc_out) & cam_config_register_bits_bnc_out;
	uint16_t configRegister = bnc_out | led_off | cool_level | trigger_mode | sensor_gain;

	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_config, configRegister);
	if (status != es_no_error) return status;
	return status;
}

es_status_codes SetupFFT(uint32_t drvno)
{
	es_status_codes status = es_no_error;
	status = SetVfreqRegister(drvno); if (status != es_no_error) return status;

	switch (settings_struct.camera_settings[drvno].fft_mode)
	{
	case full_binning:
		status = SetupFullBinningInCamera(drvno);
		break;
	case partial_binning:
	{
		status = SetupPartialBinningInCamera(drvno);
		break;
	}
	case area_mode:
		break;
	default:
		return es_parameter_out_of_range;
	}

	return status;
}

/**
 * \brief Is basically the old vclk register (cam_adaddr_vclk). Sets:
 * - en_area	(bit 15)		- as before
 * - vfreq		(bits 14...1)	- for cams with FFT_v2 FPGAs, that generate their own vclks inside the cam
 * - is_fft		(bit 0)			- as before but legacy to run FFT_v1 cameras
 */
es_status_codes SetVfreqRegister(uint32_t drvno) // was vclk register
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
	uint16_t vfreqRegsiter = (is_area_mode | (vfreqFPGA & 0x3FFF) << 1 | is_fft );
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclk, vfreqRegsiter);
	return status;
}

es_status_codes SetupFullBinningInCamera(uint32_t drvno)
{
	es_status_codes status = es_no_error;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount1, (uint16_t)settings_struct.camera_settings[drvno].fft_lines); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount2, (uint16_t)0x0000); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount3, (uint16_t)0x0000); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount4, (uint16_t)0x0000); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount5, (uint16_t)0x0000); if (status != es_no_error) return status;
	return status;
}

es_status_codes SetupPartialBinningInCamera(uint32_t drvno)
{
	es_status_codes status = es_no_error;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount1, (uint16_t)settings_struct.camera_settings[drvno].region_size[0]); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount2, (uint16_t)settings_struct.camera_settings[drvno].region_size[1]); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount3, (uint16_t)settings_struct.camera_settings[drvno].region_size[2]); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount4, (uint16_t)settings_struct.camera_settings[drvno].region_size[3]); if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_cam, cam_adaddr_vclks_amount5, (uint16_t)settings_struct.camera_settings[drvno].region_size[4]); if (status != es_no_error) return status;
	return status;
}

/**
 * \brief Initialize Measurement (using drvno).
 *
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_parameter_out_of_range
 *		- es_allocating_memory_failed
 *		- es_not_enough_ram
 *		- es_getting_dma_buffer_failed
 *		- es_enabling_interrupts_failed
 *		- es_camera_not_found
 */
es_status_codes _InitMeasurement(uint32_t drvno)
{
	es_status_codes status = InitSoftware(drvno);
	if (status != es_no_error) return status;
	status = InitPcieBoard(drvno);
	if (status != es_no_error) return status;
	if (settings_struct.camera_settings[drvno].camcnt > 0)
		status = InitCamera(drvno);
	return status;
}

/**
 * \brief Set pixel count
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetPixelCountRegister(uint32_t drvno)
{
	ES_LOG("Set pixel count: %u\n", aPIXEL[drvno]);
	return writeBitsS0_32(drvno, aPIXEL[drvno], 0xFFFF, S0Addr_PIXREGlow);
}

/**
 * \brief Clears DAT and EC.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes 
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes ClearAllUserRegs(uint32_t drvno)
{
	ES_LOG("Clear all user registers.\n");
	es_status_codes status = writeRegisterS0_32( drvno, 0, S0Addr_BDAT );
	if (status != es_no_error) return status;
	status = writeRegisterS0_32( drvno, 0, S0Addr_BEC );
	if (status != es_no_error) return status;
	status = writeRegisterS0_32( drvno, 0, S0Addr_SDAT );
	if (status != es_no_error) return status;
	return writeRegisterS0_32( drvno, 0, S0Addr_SEC );
}

/**
 * \brief Use this function to abort measurement.
 * 
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes AbortMeasurement()
{
	ES_LOG("Abort Measurement\n");
	abortMeasurementFlag = true;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		status = StopSTimer(drvno);
		if (status != es_no_error) return status;
		status = resetBlockOn(drvno);
		if (status != es_no_error) return status;
		status = resetMeasureOn(drvno);
		if (status != es_no_error) return status;
		status = ResetDma(drvno);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Sets abortMeasurementFlag to true.
 * 
 * \return es_no_error
 */
es_status_codes SetAbortMeasurementFlag()
{
	abortMeasurementFlag = true;
	return es_no_error;
}

/**
 * \brief Sets BlockOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes setBlockOn( uint32_t drvno )
{
	ES_LOG("Set block on\n");
	notifyBlockStart();
	return setBitS0_32( drvno, PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS );
}

/**
 * \brief Sets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes setMeasureOn( uint32_t drvno )
{
	ES_LOG("Set measure on\n");
	notifyMeasureStart();
	return setBitS0_32( drvno, PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS );
}

/**
 * \brief Resets BlockOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes resetBlockOn( uint32_t drvno )
{
	ES_LOG("Reset block on\n");
	notifyBlockDone();
	return resetBitS0_32( drvno, PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS );
}

/**
 * \brief Resets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes resetMeasureOn( uint32_t drvno )
{
	ES_LOG("Reset measure on\n");
	notifyMeasureDone();
	return resetBitS0_32( drvno, PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS );
}

/**
 * \brief 
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0.
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes ResetDma( uint32_t drvno )
{
	ES_LOG("Reset DMA\n");
	uint32_t BitMask = 0x1;
	uint32_t RegisterValues = 0x1;
	es_status_codes status = writeBitsDma_32(drvno, RegisterValues, BitMask, DmaAddr_DCSR);
	if (status != es_no_error)
	{
		ES_LOG("switch on the Initiator Reset for the DMA failed\n");
		return status;
	}
	// DCSR: reset the Initiator Reset 
	RegisterValues = 0x0;
	status = writeBitsDma_32(drvno, RegisterValues, BitMask, DmaAddr_DCSR);
	if (status != es_no_error)
		ES_LOG("switch off the Initiator Reset for the DMA failed\n");
	return status;
}

/**
 * \brief Set cam count
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetCamCountRegister(uint32_t drvno)
{
	ES_LOG("Set cam count: %u\n", settings_struct.camera_settings[drvno].camcnt);
	return writeBitsS0_32(drvno, settings_struct.camera_settings[drvno].camcnt, 0xF, S0Addr_CAMCNT);
}

/**
 * \brief Sets sensor type bits in register camera type
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sensor_type Determines sensor type. See enum \ref sensor_type_t in enum_settings.h for options.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes SetSensorType( uint32_t drvno, uint16_t sensor_type )
{
	ES_LOG("Setting sensor type: %u\n", sensor_type);
	es_status_codes status;
	if (sensor_type == sensor_type_fft)
		status = setBitS0_8(drvno, TOR_MSB_bitindex_ISFFT_LEGACY, S0Addr_TOR_MSB);
	else
		status = resetBitS0_8(drvno, TOR_MSB_bitindex_ISFFT_LEGACY, S0Addr_TOR_MSB);
	if (status != es_no_error) return status;
	uint32_t data = sensor_type << camera_type_sensor_type_bit_index;
	return writeBitsS0_32(drvno, data, camera_type_sensor_type_bits, S0Addr_CAMERA_TYPE);
}

/**
 * \brief Sets camera system bits in register camera type
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param camera_system Determines the camera system. See enum \ref camera_system_t in enum_settings.h for options.
 * \return es_status_codes
 */
es_status_codes SetCameraSystem(uint32_t drvno, uint16_t camera_system)
{
	ES_LOG("Setting camera system: %u\n", camera_system);
	uint32_t data = camera_system << camera_type_camera_system_bit_index;
	return writeBitsS0_32(drvno, data, camera_type_camera_system_bits, S0Addr_CAMERA_TYPE);
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 * 
 * @param data 4 bytes (32 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFFFFFFFF - all bits 32 bits are written, 0 - no bits are written.
 * @param address Address of the register in S0 space.
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes writeBitsS0_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint16_t address  )
{
	return writeBitsDma_32(drvno, data, bitmask, address + S0_SPACE_OFFSET);
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 *
 * @param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * @param data 4 bytes (32 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFFFFFFFF - all bits 32 bits are written, 0 - no bits are written.
 * @param address Address of the register in S0 space.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes writeBitsS0_32_allBoards(uint32_t board_sel, uint32_t data, uint32_t bitmask, uint16_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = writeBitsDma_32(drvno, data, bitmask, address + S0_SPACE_OFFSET);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 * 
 * @param data 1 bytes (8 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFF - all bits 8 bits are written, 0 - no bits are written.
 * @param address Address of the register in S0 space.
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes writeBitsS0_8( uint32_t drvno, uint8_t data, uint8_t bitmask, uint16_t address  )
{
	return writeBitsDma_8(drvno, data, bitmask, address + S0_SPACE_OFFSET);
}

/**
 * @brief Set bit to 1 in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes setBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32(drvno, 0xFFFFFFFF, bitmask, address);
}

/**
 * @brief Set bit to 1 in S0 register at memory address.
 *
 * @param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes setBitS0_32_allBoards(uint32_t board_sel, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32_allBoards(board_sel, 0xFFFFFFFF, bitmask, address);
}

/**
 * @brief Set bit to 1 in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...7, 0 is LSB, 7 MSB
 * @param address register address. 1 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes setBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint8_t bitmask = (uint8_t)(0x1 << bitnumber);
	return writeBitsS0_8(drvno, 0xFF, bitmask, address);
}

/**
 * @brief Set bit to 0 in register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes resetBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32(drvno, 0x0, bitmask, address);
}

/**
 * @brief Set bit to 0 in register at memory address.
 *
 * @param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes resetBitS0_32_allBoards(uint32_t board_sel, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32_allBoards(board_sel, 0x0, bitmask, address);
}

/**
 * @brief Set bit to 0 in register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...7, 0 is LSB, 7 MSB
 * @param address register address. 1 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes resetBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint8_t bitmask = (uint8_t)(0x1 << bitnumber);
	return writeBitsS0_8(drvno, 0x0, bitmask, address);
}

/**
 * \brief Write 4 byte of a register in S0 space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Data to write.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterS0_32( uint32_t drvno, uint32_t data, uint16_t address )
{
	return writeRegister_32(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Write 4 bytes of a register in S0 space.
 *
 * \param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \param data Data to write.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterS0_32_allBoards(uint32_t board_sel, uint32_t data, uint16_t address)
{
	return writeRegister_32_allBoards(board_sel, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Write 4 bytes of a register.
 * 
 * \param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \param data 4 byte of data to write.
 * \param address Address of the register to write.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegister_32_allBoards(uint32_t board_sel, uint32_t data, uint16_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = writeRegister_32(drvno, data, address);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \brief Write 2 bytes of a register in S0 space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Data to write.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterS0_16( uint32_t drvno, uint16_t data, uint16_t address )
{
	return writeRegister_16(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Write 1 byte of a register in S0 space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Data to write.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterS0_8( uint32_t drvno, uint8_t data, uint16_t address )
{
	return writeRegister_8(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Write the same 1 byte to a register in S0 space of all boards.
 *
 * \param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \param data Data to write.
 * \param address Address of the register to write.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterS0_8_allBoards(uint32_t board_sel, uint8_t data, uint16_t address)
{
	return writeRegister_8_allBoards(board_sel, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Write the same 1 byte to a register of all boards.
 *
 * \param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \param data 1 byte data to write.
 * \param address Address of the register to write.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegister_8_allBoards(uint32_t board_sel, uint8_t data, uint16_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = writeRegister_8(drvno, data, address);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \brief Read 4 bytes of a register in S0 space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readRegisterS0_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
	return readRegister_32(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Read 4 bytes of a register in S0 space of all boards.
 *
 * @param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readRegisterS0_32_allBoards(uint32_t board_sel, uint32_t** data, uint16_t address)
{
	return readRegister_32_allBoards(board_sel, data, address + S0_SPACE_OFFSET);
}

/**
 * @brief Reads 4 bytes on DMA area of all PCIe boards.
 *
 * @param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * @param data buffer array for data
 * @param address Offset from BaseAdress - in Bytes ! 0..3= Regs of Board.
 * @return es_status_codes
	- es_no_error
	- es_register_read_failed
 */
es_status_codes readRegister_32_allBoards(uint32_t board_sel, uint32_t** data, uint16_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = readRegister_32(drvno, *(data + drvno), address);
			if (status != es_no_error) return status;
		}
	}
	return status;
};

/**
 * \brief Read 2 bytes of a register in S0 space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readRegisterS0_16( uint32_t drvno, uint16_t* data, uint16_t address )
{
	return readRegister_16(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Read 1 byte of a register in S0 space.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readRegisterS0_8( uint32_t drvno, uint8_t* data, uint16_t address )
{
	return readRegister_8(drvno, data, address + S0_SPACE_OFFSET);
}
/**
 * \brief Read 1 bit of 1 of 4 bytes.
 * 
 * \param drvno identifier of PCIE card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param address Address of the register to read.
 * \param bitnumber Address of the bit to read.
 * \param isBitHigh Tells if bit is high or low.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes ReadBitS0_32(uint32_t drvno, uint16_t address, uint8_t bitnumber, bool* isBitHigh)
{
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, address);
	if (data & (1 << bitnumber)) *isBitHigh = true;
	else *isBitHigh = false;

	return status;
}

/**
 * \brief	Read 1 bit of a byte.
 * 
 * \param drvno identifier of PCIE card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param address Address of the register to read.
 * \param bitnumber Address of the bit to read.
 * \param isBitHigh Tells if bit is high or low.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes ReadBitS0_8(uint32_t drvno, uint16_t address, uint8_t bitnumber, bool* isBitHigh)
{
	uint8_t data = 0;
	es_status_codes status = readRegisterS0_8(drvno, &data, address);

	if (data & (1 << bitnumber)) *isBitHigh = true;
	else *isBitHigh = false;

	return status;
}

/**
 * \brief Open shutter for sensors with EC (exposure control) / sets IFC signal = high.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OpenShutter( uint32_t drvno )
{
	ES_LOG("Open shutter\n");
	return setBitS0_8(drvno, CTRLB_bitindex_SHON, S0Addr_CTRLB);
}

/**
 * \brief For FFTs: Setup full binning.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param lines Lines in camera.
 * \param vfreq Frequency for vertical clock.
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetupFullBinning( uint32_t drvno, uint32_t lines, uint8_t vfreq )
{
	ES_LOG("Setup full binning\n");
	*useSWTrig = false;
	es_status_codes status = SetupVCLKReg( drvno, lines, vfreq );
	if (status != es_no_error) return status;
	return ResetPartialBinning( drvno );
}

/**
 * \brief Set REG VCLKCTRL for FFT sensors.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param lines number of vertical lines
 * \param vfreq vertical clock frequency
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetupVCLKReg( uint32_t drvno, uint32_t lines, uint8_t vfreq )
{
	ES_LOG( "Setup VCLK register. drvno: %u, lines: %u, vfreq: %u\n", drvno, lines, vfreq);
	es_status_codes status = writeRegisterS0_32( drvno, lines * 2, S0Addr_VCLKCTRL );// write no of vclks=2*lines
	if (status != es_no_error) return status;
	return writeRegisterS0_8( drvno, vfreq, S0Addr_VCLKFREQ );
}

/**
 * \brief sets Vertical Partial Binning in registers R10,R11 and R12. Only for FFT sensors.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param range specifies R 1..5
 * \param lines number of vertical clocks for next read
 * \param keep TRUE if scan should be written to FIFO
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetupVPB(uint32_t drvno, uint32_t range, uint32_t lines, bool keep)
{
	ES_LOG("SetupVPB, range: 0x%x, lines: 0x%x, keep: %x\n", range, lines, keep);
	uint16_t adr = 0;
	lines *= 2; //vclks=lines*2
	switch (range)
	{
	case 1:
		adr = S0Addr_ROI0;
		break;
	case 2:
		adr = S0Addr_ROI0 + 2;
		break;
	case 3:
		adr = S0Addr_ROI1;
		break;
	case 4:
		adr = S0Addr_ROI1 + 2;
		break;
	case 5:
		adr = S0Addr_ROI2;
		break;
	case 6:
		adr = S0Addr_ROI2 + 2;
		break;
	case 7:
		adr = S0Addr_XCKDLY;
		break;
	case 8:
		adr = S0Addr_XCKDLY + 2;
		break;
	}
	if (keep) { lines |= 0x8000; }
	else { lines &= 0x7fff; }
	es_status_codes  status = writeRegisterS0_8(drvno, (uint8_t)lines, adr);
	if (status != es_no_error) return status;
	return writeRegisterS0_8(drvno, (uint8_t)(lines >> 8), adr + 1);
}// SetupVPB

/**
 * \brief Turn partial binning on.
 *
 * \param drvno  PCIe board identifier.
 * \param number_of_regions number of regions for partial binning
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetPartialBinning( uint32_t drvno, uint16_t number_of_regions )
{
	ES_LOG("Set partial binning\n");
	es_status_codes status = writeRegisterS0_32(drvno, number_of_regions, S0Addr_ARREG);
	if (status != es_no_error) return status;
	return setBitS0_32(drvno, 15, S0Addr_ARREG);//this turns ARREG on and therefore partial binning too
}

/**
 * \brief Turns 
 * ARREG off and therefore partial binning too.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes ResetPartialBinning( uint32_t drvno )
{
	ES_LOG("Reset partial binning\n");
	return resetBitS0_32(drvno, 15, S0Addr_ARREG );
}

/**
 * \brief Stop S Timer.
 * 
 * Clear Bit30 of XCK-Reg: 0= timer off
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes StopSTimer( uint32_t drvno )
{
	ES_LOG("Stop S Timer\n");
	return resetBitS0_8(drvno, XCKMSB_bitindex_stimer_on, S0Addr_XCKMSB);
}

/**
 * \brief reset FIFO and FFcounter
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes RSFifo( uint32_t drvno )
{
	ES_LOG("Reset FIFO\n");
	es_status_codes status = setBitS0_8(drvno, FFCTRL_bitindex_RSFIFO, S0Addr_FFCTRL);
	if (status != es_no_error) return status;
	return resetBitS0_8(drvno, FFCTRL_bitindex_RSFIFO, S0Addr_FFCTRL);
}

/**
 * Allocate user memory.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 *		- es_allocating_memory_failed
 *		- es_not_enough_ram
 */
es_status_codes allocateUserMemory( uint32_t drvno )
{
	//free old memory before allocating new one
	free( userBuffer[drvno] );
	uint64_t memory_all = 0;
	uint64_t memory_free = 0;
	FreeMemInfo( &memory_all, &memory_free );
	uint64_t memory_free_mb = memory_free / (1024 * 1024);
	uint64_t needed_mem = (uint64_t)aCAMCNT[drvno] * (uint64_t)(*Nob) * (uint64_t)(*Nospb) * (uint64_t)aPIXEL[drvno] * (uint64_t)sizeof( uint16_t );
	uint64_t needed_mem_mb = needed_mem / (1024 * 1024);
	ES_LOG( "Allocate user memory, available memory:%ld MB, memory needed: %ld MB (%ld)\n", memory_free_mb, needed_mem_mb, needed_mem );
	//check if enough space is available in the physical ram
	if (memory_free > (uint64_t)needed_mem)
	{
		uint16_t* userBufferTemp = (uint16_t*) calloc( needed_mem, 1 );
		if (userBufferTemp)
		{
			userBufferEndPtr[drvno] = userBufferTemp + needed_mem / sizeof(uint16_t);
			userBuffer[drvno] = userBufferTemp;
			userBufferWritePos[drvno] = userBufferTemp;
			userBufferWritePos_last[drvno] = userBufferTemp;
			ES_LOG("user buffer space: %p - %p\n", userBuffer[drvno], userBufferEndPtr[drvno]);
			return es_no_error;
		}
		else
		{
			ES_LOG( "Allocating user memory failed.\n" );
			return es_allocating_memory_failed;
		}
	}
	else
	{
		ES_LOG( "ERROR for buffer %d: available memory: %ld MB \n \tmemory needed: %ld MB\n", number_of_boards, memory_free_mb, needed_mem_mb );
		return es_not_enough_ram;
	}
}

/**
 * \brief Set DMA register
 * 
 * Sets DMA_BUFFER_SIZE_IN_SCANS, DMA_DMASPERINTR, NOS, NOB, CAMCNT
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetDMABufRegs( uint32_t drvno )
{
	ES_LOG("Set DMA buffer registers, ");
	//DMABufSizeInScans - use 1 block
	es_status_codes status = writeBitsS0_32(drvno, settings_struct.camera_settings[drvno].dma_buffer_size_in_scans, 0xffffffff, S0Addr_DmaBufSizeInScans);
	if (status != es_no_error) return status;
	//scans per interrupt must be 2x per DMA_BUFFER_SIZE_IN_SCANS to copy hi/lo part
	//aCAMCNT: double the INTR if 2 cams
	uint32_t dmasPerInterrupt = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS;
	status = writeBitsS0_32(drvno, dmasPerInterrupt, 0xffffffff, S0Addr_DMAsPerIntr);
	if (status != es_no_error) return status;
	ES_LOG( "scansPerInterrupt/camcnt: %u \n", dmasPerInterrupt / aCAMCNT[drvno] );
	return status;
}

es_status_codes SetNosRegister(uint32_t drvno)
{
	ES_LOG("Set NOS register to %u", *Nospb);
	return writeBitsS0_32(drvno, *Nospb, 0xffffffff, S0Addr_NOS);
}

es_status_codes SetNobRegister(uint32_t drvno)
{
	ES_LOG("Set NOB register to %u", *Nob);
	return writeBitsS0_32(drvno, *Nob, 0xffffffff, S0Addr_NOB);
}

/**
 * \brief Sets the IFC bit of interface for sensors with shutter function. IFC=low
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes CloseShutter( uint32_t drvno )
{
	ES_LOG("Close shutter\n");
	return resetBitS0_8(drvno, CTRLB_bitindex_SHON, S0Addr_CTRLB);
}

/**
 * \brief Exposure control (EC) signal is used for mechanical shutter or sensors with EC function.
 * 
 * Starts after delay after trigger (DAT) signal and is active for ecin10ns.
 * Resets additional delay after trigger with ecin10ns = 0.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param ecin10ns Time in 10 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetSEC( uint32_t drvno, uint32_t ecin10ns)
{
	ES_LOG("Set SEC. EC in 10 ns: %u\n", ecin10ns);
	return writeRegisterS0_32(drvno, ecin10ns, S0Addr_SEC);
}

/**
 * \brief Set signal of output port of PCIe card.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param tor select output signal. See enum tor_out in enum.h for options.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetTORReg( uint32_t drvno, uint8_t tor )
{
	ES_LOG("Set TOR: %u\n", tor);
	// TOR register layout:
	// bit		31	30	29	28	27
	// meaning	TO3	TO2	TO1	TO0	TOSELG
	// use lower 4 bits of input tor for the upper nibble TO0 - TO3
	uint8_t tor_upper_nibble = tor << TOR_MSB_bitindex_TO0;
	// use bit 5 of input tor for bit TOSELG
	uint8_t toselg = tor & 0x10;
	toselg = toselg >> 4;
	// shift the bit to the correct position
	toselg = toselg << TOR_MSB_bitindex_TOSEL;
	uint8_t data = tor_upper_nibble | toselg;
	return writeBitsS0_8(drvno, data, TOR_MSB_BITS_TO, S0Addr_TOR_MSB);
}

/**
 * \brief Set the external trigger slope for scan trigger (PCI Reg CrtlA:D5 -> manual).
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sslope Choose slope:
 *		- 0: high slope
 *		- 1: low slope
 *		- 2: both slope
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes SetSSlope(uint32_t drvno, uint32_t sslope)
{
	ES_LOG("Set scan slope, %u\n", sslope);
	es_status_codes status = es_no_error;
	switch (sslope)
	{
	// high slope
	case 0:
		status = setBitS0_32(drvno, CTRLA_bitindex_SLOPE, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		status = resetBitS0_32(drvno, CTRLA_bitindex_BOTH_SLOPE, S0Addr_CTRLA);
		break;
	// low slope
	case 1:
		status = resetBitS0_32(drvno, CTRLA_bitindex_SLOPE, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		status = resetBitS0_32(drvno, CTRLA_bitindex_BOTH_SLOPE, S0Addr_CTRLA);
		break;
	// both slope
	case 2:
		status = setBitS0_32(drvno, CTRLA_bitindex_SLOPE, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		status = setBitS0_32(drvno, CTRLA_bitindex_BOTH_SLOPE, S0Addr_CTRLA);
		break;
	default:
		return es_parameter_out_of_range;
	}
	return status;
}

/**
 * \brief Sets slope for block trigger.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param slope:
 *		- 0: negative slope
 *		- 1: positive slope
 *		- 2: both
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetBSlope( uint32_t drvno, uint32_t slope )
{
	ES_LOG("Set BSlope: %u\n", slope);
	return writeRegisterS0_32( drvno, slope, S0Addr_BSLOPE );
}

/**
* \brief Chooses trigger input for scan trigger input (STI)
* \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
* \param sti_mode Defines the input mode for STI.
* 	- 0: I
* 	- 1: S1
* 	- 2: S2
* 	- 3: unused
* 	- 4: S Timer
* 	- 5: ASL
* \return es_status_codes
* 	- es_no_error
* 	- es_register_read_failed
* 	- es_register_write_failed
*/
es_status_codes SetSTI( uint32_t drvno, uint8_t sti_mode )
{
	ES_LOG("Set STI: %u\n", sti_mode);
	return writeBitsS0_8( drvno, sti_mode, CTRLB_bit_STI0 | CTRLB_bit_STI1 | CTRLB_bit_STI2, S0Addr_CTRLB );
}

/**
* \brief Chooses trigger input for block trigger input (BTI)
* \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
* \param bti_mode Defines the input mode for BTI.
* 	- 0: I
* 	- 1: S1
* 	- 2: S2
* 	- 3: S1&s2
* 	- 4: BTIMER
*	- 5: S1 chopper
*	- 6: S2 chopper
*	- 7: S1&S2 chopper
* \return es_status_codes
* 	- es_no_error
* 	- es_register_read_failed
* 	- es_register_write_failed
*/
es_status_codes SetBTI( uint32_t drvno, uint8_t bti_mode )
{
	ES_LOG("Set BTI: %u\n", bti_mode);
	return writeBitsS0_8( drvno, (uint8_t)(bti_mode << CTRLB_bitindex_BTI0), CTRLB_bit_BTI0 | CTRLB_bit_BTI1 | CTRLB_bit_BTI2, S0Addr_CTRLB );
}

/**
 * \brief Sets time for scan timer.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param stime_in_microseconds Scan time in microseconds.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetSTimer( uint32_t drvno, uint32_t stime_in_microseconds )
{
	ES_LOG("Set stime in microseconds: %u\n", stime_in_microseconds);
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32( drvno, &data, S0Addr_XCKLL );
	if (status != es_no_error) return status;
	data &= 0xF0000000;
	data |= stime_in_microseconds & 0x0FFFFFFF;
	return writeRegisterS0_32( drvno, data, S0Addr_XCKLL );
}

/**
 * \brief Sets time for block timer.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param btime_in_microseconds Block time in microseconds.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetBTimer( uint32_t drvno, uint32_t btime_in_microseconds )
{
	ES_LOG("Set btime in microseconds: %u\n", btime_in_microseconds);
	if (btime_in_microseconds)
	{
		uint32_t data = btime_in_microseconds | 0x80000000;
		return writeRegisterS0_32( drvno, data, S0Addr_BTIMER );
	}
	else
		return writeRegisterS0_32( drvno, 0, S0Addr_BTIMER );
}

/**
 * \brief Initialize the TDC-GPX chip. TDC: time delay counter option.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param delay GPX offset is used to increase accuracy. A counter value can be added, usually 1000.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 * register dump with _AboutGPX() but with error status
 */
es_status_codes InitGPX( uint32_t drvno, uint32_t delay )
{
	ES_LOG("Init GPX, delay: %u\n", delay);
	uint32_t regData, err_cnt = 0;
	uint32_t mask = 0x3FFFF;
	delay &= mask;
	uint32_t regVal = 0x08200000 | delay;
	uint32_t RegData[12][2] = {
		{ 0, 0x000000AB },	// write to reg0: 0x80 disable inputs
	{ 1, 0x0620620 },	// write to reg1: 0x0620620 channel adjust
	{ 2, 0x00062FFC },	// write to reg2: 62E04  R-mode, disable all CH
	{ 3, 0x00000000 },	// write to reg3: 0 set to ECL
	{ 4, 0x02000000 },	// write to reg4: 0x02000000 EF flag=on
	{ 6, 0x08000001 },	// write to reg6: ECL + FILL=1
	{ 7, 0x00001FB4 },	// write to reg7: res= 27ps 
	{ 11, 0x07ff0000 },	// write to reg11: 7ff all error flags (layout flag is not connected)
	{ 12, 0x00000000 },	// write to reg12: no IR flags - is used anyway when 1 hit
	{ 14, 0x0 },
	//arm
	{ 4, 0x02400000 },	// write to reg4: master reset
	{ 2, 0x00062004 }	// write to reg2: 62E04  R-mode, en CH0..5 (3 values
	};
	// setup GPX chip for mode M
	//reset GPX  ´bit0 in GPXCTRL reg
	es_status_codes status = readRegisterS0_32( drvno, &regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	regData |= TDCCtrl_bit_reset;
	status = writeRegisterS0_32( drvno, regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	regData &= ~TDCCtrl_bit_reset;
	status = writeRegisterS0_32( drvno, regData, S0Addr_TDCCtrl ); //reset bit
	if (status != es_no_error) return status;
	//setup R mode -> time between start and stop
	status = SetGPXCtrl( drvno, 5, regVal ); // write to reg5: 82000000 retrigger, disable after start-> reduce to 1 val
	if (status != es_no_error) return status;
	for (int write_reg = 0; write_reg < 12; write_reg++)
	{
		status = SetGPXCtrl( drvno, (uint8_t)RegData[write_reg][0], RegData[write_reg][1] );//write
		if (status != es_no_error) return status;
		status = ReadGPXCtrl( drvno, (uint8_t)RegData[write_reg][0], &regData );//read
		if (RegData[write_reg][1] != regData) err_cnt++;//compare write data with read data
	}
	return status;
}

/**
 * \brief Set GPXCtrl register.
 * 
 * \param drvno select PCIe board
 * \param GPXAddress address to access
 * \param GPXData data to write
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetGPXCtrl( uint32_t drvno, uint8_t GPXAddress, uint32_t GPXData )
{
	uint32_t data = 0;
	//shift gpx addr to the right place for the gpx ctrl reg
	data = (uint32_t)(GPXAddress << TDCCtrl_bitindex_adr0);
	uint32_t bitmask = TDCCtrl_bit_cs | TDCCtrl_bit_adr0 | TDCCtrl_bit_adr1 | TDCCtrl_bit_adr2 | TDCCtrl_bit_adr3;
	es_status_codes status = writeBitsS0_32(drvno, data, bitmask, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return writeRegisterS0_32( drvno, GPXData, S0Addr_TDCData );
}

/**
 * \brief Read GPXCtrl register.
 * 
 * \param drvno select PCIe board
 * \param GPXAddress address to access
 * \param GPXData pointer where read data is written to
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes ReadGPXCtrl( uint32_t drvno, uint8_t GPXAddress, uint32_t* GPXData )
{
	uint32_t data = 0;
	//shift gpx addr to the right place for the gpx ctrl reg
	data = (uint32_t)GPXAddress << TDCCtrl_bitindex_adr0;
	//set CSexpand bit set CS Bit
	data |= TDCCtrl_bit_cs;
	uint32_t bitmask = TDCCtrl_bit_cs | TDCCtrl_bit_adr0 | TDCCtrl_bit_adr1 | TDCCtrl_bit_adr2 | TDCCtrl_bit_adr3;
	es_status_codes status = writeBitsS0_32(drvno, data, bitmask, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return readRegisterS0_32( drvno, GPXData, S0Addr_TDCData );
}

/**
 * \brief Sets delay after trigger hardware register.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param datin10ns Time in 10 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetSDAT( uint32_t drvno, uint32_t datin10ns )
{
	ES_LOG("Set SDAT in 10ns: %u\n", datin10ns);
	if (datin10ns)
	{
		datin10ns |= 0x80000000; // enable delay
		return writeRegisterS0_32(drvno, datin10ns, S0Addr_SDAT);
	}
	else return writeRegisterS0_32(drvno, 0, S0Addr_SDAT);
}

/**
 * \brief Sets delay after trigger hardware register.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param datin10ns Time in 10 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetBDAT( uint32_t drvno, uint32_t datin10ns )
{
	ES_LOG("Set BDAT in 10ns: %u\n", datin10ns);
	if (datin10ns)
	{
		datin10ns |= 0x80000000; // enable delay
		return writeRegisterS0_32(drvno, datin10ns, S0Addr_BDAT);
	}
	else return writeRegisterS0_32(drvno, 0, S0Addr_BDAT);
}

/**
 * \brief Protects ENFFW from cool cam status transmission. Enable with cool cam, disable with HS > 50 kHz.
 * 
 * Legacy code for old cameras.
 * RX_VALID usually triggers ENFFW. This must be disabled when cool cams transmit their cooling status.
 * RX_VALID_EN is enabled with XCKI and disabled with ~CAMFFXCK_ALL, after all frame data is collected.
 * If RX_VALID raises again for cool status values, it doesn't effect ENFFW when RX_VALID_EN is low.
 * \param drvno selects PCIe board
 * \param USE_ENFFW_PROTECT enables or disables RX_VALID write protection
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes Use_ENFFW_protection( uint32_t drvno, bool USE_ENFFW_PROTECT )
{
	if (USE_ENFFW_PROTECT)
		return setBitS0_32( drvno, PCIEFLAGS_bitindex_USE_ENFFW_PROTECT, S0Addr_PCIEFLAGS );
	else
		return resetBitS0_32( drvno, PCIEFLAGS_bitindex_USE_ENFFW_PROTECT, S0Addr_PCIEFLAGS );
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
es_status_codes SendFLCAM( uint32_t drvno, uint8_t maddr, uint8_t adaddr, uint16_t data )
{
	ES_TRACE("SendFLCAM(): maddr 0x%x, adaddr: 0x%x, data 0x%x (%u)\n", maddr, adaddr, data, data);
	es_status_codes status = FindCam(drvno);
	if (status != es_no_error) return status;
	uint32_t ldata = 0;
	ldata = maddr;
	ldata = ldata << 8;
	ldata |= adaddr;
	ldata = ldata << 16;
	ldata |= data;
	status = writeRegisterS0_32( drvno, ldata, S0Addr_DBR );
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
 * \brief Init routine for Camera System 3001.
 * 
 * 	Sets register in camera.
 * \param drvno selects PCIe board
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes InitCamera3001( uint32_t drvno )
{
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
es_status_codes InitCamera3010( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern )
{
	ES_LOG("Init camera 3010, adc_mode: %u, custom_pattern: %u\n", adc_mode, custom_pattern);
	es_status_codes status = Cam3010_ADC_reset( drvno );
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
 * 	Called by InitCamera3010
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3010_ADC_reset( uint32_t drvno )
{
	ES_LOG("Camera 3010 ADC reset\n");
	return SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_reset, adc_ltc2271_msg_reset );
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
		es_status_codes status = SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_custompattern);
		if (status != es_no_error) return status;
		return Cam3010_ADC_sendTestPattern(drvno, custom_pattern);
	}
	else {
		ES_LOG("Camera 3010 ADC set output mode to 4-lane mode and normal operation\n");
		return SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_normal_mode);
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
	
	status = SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_msb, highByte);
	if (status != es_no_error) return status;
	status = SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_lsb, lowByte);

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
es_status_codes InitCamera3030(uint32_t drvno)
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
	for (uint32_t camera = 0; camera < settings_struct.camera_settings->camcnt; camera++)
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
 * 	Called by InitCamera3030
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_reset( uint32_t drvno )
{
	return SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_reset, adc_ads5294_msg_reset );
}

/**
 * \brief ADC output interface config routine for Camera System 3030.
 * 
 * 	Enables two wire LVDS data transfer mode of ADC ADS5294.
 * 	Only works with PAL versions P209_2 and above.
 * 	Called by InitCamera3030 - comment for older versions and rebuild
 * 	or use on e-lab test computer desktop LabView folder lv64hs (bool switch in 3030 init tab)
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_twoWireModeEN( uint32_t drvno )
{
	es_status_codes status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_2wireMode, adc_ads5294_msg_2wireMode );
	if (status != es_no_error) return status;
	status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_wordWiseOutput, adc_ads5294_msg_wordWiseOutput );
	if (status != es_no_error) return status;
	return SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_ddrClkAlign, adc_ads5294_msg_ddrClkAlign );
}

/**
 * \brief ADC gain config routine for Camera System 3030.
 * 
 * 	Sets gain of ADC ADS5294 0...15 by calling SetADGain() subroutine.
 * 	Called by InitCamera3030
 * \param drvno selects PCIe board
 * \param gain of ADC
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes Cam3030_ADC_SetGain( uint32_t drvno, uint8_t gain )
{
	return SetADGain( drvno, 1, gain, gain, gain, gain, gain, gain, gain, gain );
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
es_status_codes SetADGain( uint32_t drvno, uint8_t fkt, uint8_t g1, uint8_t g2, uint8_t g3, uint8_t g4, uint8_t g5, uint8_t g6, uint8_t g7, uint8_t g8 )
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
	es_status_codes status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_gain_1_to_4, data );	//gain1..4
	if (status != es_no_error) return status;
	data = h;
	data = (uint16_t)(data << 4);
	data |= d;
	data = (uint16_t)(data << 4);
	data |= g;
	data = (uint16_t)(data << 4);
	data |= c;
	return SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_gain_5_to_8, data );	//gain7..8
}

/**
 * \brief ADC debug mode for Camera System 3030.
 * 
 * Lets ADC send a ramp or a custom pattern (value) instead of ADC sample data.
 * Called by InitCamera3030 when adc_mode > 0.
 * \param drvno selects PCIe board
 * \param adc_mode 1: ramp, 2: custom pattern
 * \param custom_pattern (only used when adc_mode = 2)
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam3030_ADC_RampOrPattern( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern )
{
	es_status_codes status = es_no_error;
	switch (adc_mode)
	{
	case 1: //ramp
		status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_ramp );
		break;
	case 2: //custom pattern
		//to activate custom pattern the following messages are necessary: d - data
		//at address 0x25 (mode and higher bits): 0b00000000000100dd
		status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_custompattern | ((custom_pattern >> 12) & 0x3) );
		if (status != es_no_error) return status;
		//at address 0x26 (lower bits): 0bdddddddddddd0000
		status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_custompattern, (uint16_t)(custom_pattern << 4) );
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
	return SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_global_en_filter, payload);
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
	return SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_filter1 + channel - 1 , payload);
}

/**
 * .
 * 
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
	return SendFLCAM(drvno, maddr_adc, address, payload);
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
	return SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_data_rate, data_rate & 0x3);
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
	return SendFLCAM(drvno, maddr_adc, adc_ads5294_regaddr_LFNSM, payload);
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
	es_status_codes status = SendFLCAM(drvno, maddr_cam, cam_adaddr_sample_mode, sample_mode);
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
				if(coefficient >= number_of_coefficients - active_coefficients)
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
 * \param sensor_reset_length_in_8_ns value * 8ns = sensor reset length. range: 0ns ... 524280ns = 524.28us
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes Cam_SetSensorResetLength(uint32_t drvno, uint16_t sensor_reset_length_in_4_ns)
{
	ES_LOG("Cam_SetSensorResetLength(), setting sensor reset length to %u (%u ns)\n", sensor_reset_length_in_4_ns, sensor_reset_length_in_4_ns * 4 );
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_sensor_reset_length_in_4_ns, sensor_reset_length_in_4_ns);
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
es_status_codes SetTemp( uint32_t drvno, uint8_t level )
{
	if (level >= 8) level = 0;
	ES_LOG("Set temperature level: %u\n", level);
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_coolTemp, level );
}

/**
 * \brief Sends data to DAC8568.
 * 
 * Mapping of bits in DAC8568: 4 prefix, 4 control, 4 address, 16 data, 4 feature.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param location Switch for the different locations of DAC85689. See enum \ref DAC8568_location_t in enum_settings.h for details.
 * \param cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * \param ctrlBits 4 control bits
 * \param addrBits 4 address bits
 * \param dataBits 16 data bits
 * \param featureBits 4 feature bits
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes DAC8568_sendData( uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t ctrlBits, uint8_t addrBits, uint16_t dataBits, uint8_t featureBits )
{
	uint32_t data = 0;
	es_status_codes status;
	if (ctrlBits & 0xF0) //4 ctrl bits => only lower 4 bits allowed
	{
		ES_LOG( "DAC8568_sendData: Only values between 0 and 15 are allowed for control bits." );
		return es_parameter_out_of_range;
	}
	if (addrBits & 0xF0) //4 addr bits => only lower 4 bits allowed
	{
		ES_LOG( "DAC8568_sendData: Only values between 0 and 15 are allowed for address bits." );
		return es_parameter_out_of_range;
	}
	if (featureBits & 0xF0) //4 ctrl bits => only lower 4 bits allowed
	{
		ES_LOG( "DAC8568_sendData: Only values between 0 and 15 are allowed for feature bits." );
		return es_parameter_out_of_range;
	}
	data |= 0x0;	//4 prefix bits, first bit always 0 (0xxx)
	data <<= 4;
	data |= ctrlBits & 0x0F;	//4 control bits
	data <<= 4;
	data |= addrBits & 0x0F;	//4 address bits
	data <<= 16;
	data |= dataBits;		//16 data bits
	data <<= 4;
	data |= featureBits & 0x0F;	//4 feature bits
	switch (location)
	{
		case DAC8568_camera:
		{
			uint16_t hi_bytes = (uint16_t) (data >> 16);
			uint16_t lo_bytes = (uint16_t) data;
			uint8_t adaddr = dac_hi_byte_addr | cameraPosition << campos_bit_index;
			status = SendFLCAM(drvno, maddr_dac, adaddr, hi_bytes);
			if (status != es_no_error) return status;
			adaddr = dac_lo_byte_addr | cameraPosition << campos_bit_index;
			status = SendFLCAM(drvno, maddr_dac, adaddr, lo_bytes);
			break;
		}
		case DAC8568_pcie:
			status = writeRegisterS0_32(drvno, data, S0Addr_DAC);
			if (status != es_no_error) return status;
			// set load bit to 1, to start SPI data transfer to DAC
			data |= 0x80000000;
			status = writeRegisterS0_32(drvno, data, S0Addr_DAC);
			break;
		default:
			return es_parameter_out_of_range;
	}
	return status;
}

/**
 * \brief Sets all outputs of the DAC8568 in camera 3030 or on PCIe board.
 * 
 * Use this function to set the outputs, because it is resorting the channel numeration correctly.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param location Switch for the different locations of DAC85689. See enum \ref DAC8568_location_t in enum.h for details.
 * \param cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * \param output all output values that will be converted to analog voltage (0 ... 0xFFFF)
 * \param reorder_channels used to reorder DAC channels for high speed camera
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes DAC8568_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, bool reorder_channels)
{
	es_status_codes status = es_no_error;
	int* reorder_ch;
	if (reorder_channels)
	{
		int tmp[8] = { 3, 4, 0, 5, 1, 6, 2, 7 }; //HS VIS
		reorder_ch = tmp;
	}
	else
	{
		int tmp[8] = { 0, 1, 2, 3, 4, 5, 6, 7 }; //HS IR or PCIe board
		reorder_ch = tmp;
	}
	for (uint8_t channel = 0; channel < 8; channel++)
	{
		status = DAC8568_setOutput(drvno, location, cameraPosition, channel, (uint16_t)output[reorder_ch[channel]]);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Sets the output of the DAC8568 on PCB 2189-7.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param location Switch for the different locations of DAC85689. See enum \ref DAC8568_location_t in enum_settings.h for details.
 * \param cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * \param channel select one of eight output channel (0 ... 7)
 * \param output output value that will be converted to analog voltage (0 ... 0xFFFF)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes DAC8568_setOutput( uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output )
{
	//ctrl bits 3: write and update DAC register
	ES_LOG("Set DAC: board %u, location %u, cameraPosition %u, output ch%u = %u\n", drvno, location, cameraPosition, channel, output);
	return DAC8568_sendData( drvno, location, cameraPosition, 3, channel, output, 0 );
}

/**
 * \brief Enable the internal reference in static mode.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param location Switch for the different locations of DAC85689. See enum \ref DAC8568_location_t in enum_settings.h for details.
 * \param cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes DAC8568_enableInternalReference(uint32_t drvno, uint8_t location, uint8_t cameraPosition)
{
	ES_LOG("DAC %u: enable internal reference\n", location);
	return DAC8568_sendData(drvno, location, cameraPosition, 8, 0, 0, 1);
}

/**
 * \brief This function sets the register BEC.
 * 
 * The Block Exposure control (BEC) signal can be used to open and close a mechanical shutter.
 * BEC starts after the block delay after trigger (BDAT) signal and is active for bec_in_10ns.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param bec_in_10ns:
 *	- =0 no BEC
 *	- >0 Time in 10 ns steps. Min: 1 * 10 ns, Max: 4294967295 * 10ns = 42949672950ns = 42,94967295s
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetBEC( uint32_t drvno, uint32_t bec_in_10ns )
{
	ES_LOG("Set BEC in 10 ns: %u\n", bec_in_10ns);
	return writeRegisterS0_32(drvno, bec_in_10ns, S0Addr_BEC);
}

/**
 * \brief Set XCK delay.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param xckdelay_in_10ns XCK delay
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetXckdelay(uint32_t drvno, uint32_t xckdelay_in_10ns)
{
	ES_LOG("Set XCK delay: %u\n", xckdelay_in_10ns);
	es_status_codes status = es_no_error;
	if (xckdelay_in_10ns)
	{
		xckdelay_in_10ns |= 0x80000000;
		status = writeRegisterS0_32(drvno, xckdelay_in_10ns, S0Addr_XCKDLY);
	}
	else
		status = writeRegisterS0_32(drvno, 0, S0Addr_XCKDLY);
	return status;
}

/**
 * \brief Set DMA related registers like TLP mode and DMA addresses.
 * 
 * \param drvno
 * \param pixel
 * \return 
 */
es_status_codes SetDmaRegister( uint32_t drvno, uint32_t pixel )
{		
	ES_LOG("Set DMA register: drv: %u, pixel: %u\n", drvno, pixel);
	if (pixel % 64 != 0 && !MANUAL_OVERRIDE_TLP)
	{
		ES_LOG("Could not choose TLP size, no valid pixel count.\n");
		return es_invalid_pixel_count;
	}
	// Pixelsize = TLPS * TLPC - 1*TLPS
	uint32_t no_tlps = pixel / 64;
	if (LEGACY_202_14_TLPCNT) no_tlps = no_tlps + 1;
	uint32_t data = 0;
	es_status_codes status = readConfig_32(drvno, &data, PCIeAddr_PCIExpressDeviceCapabilities);
	if (status != es_no_error) return status;
	int tlpmode = data & PciExpressDeviceCapabilities_MaxPayloadSizeSupported_bits;
	if (FORCETLPS128) tlpmode = 0;
	//delete the old values
	data &= 0xFFFFFF1F;
	//set maxreadrequestsize
	data |= (0x2 << 12);
	//default = 0x20 A.M. Dec'20 //with0x21: crash
	uint32_t tlp_size = 0x20;
	switch (tlpmode)
	{
	case 0:
		data |= 0x00;//set to 128 bytes = 32 DWORDS 
		//BData |= 0x00000020;//set from 128 to 256 
		//WriteLongIOPort( drvno, BData, PCIeAddr_DeviceControl );
		//NO_TLPS setup now in setboardvars
		tlp_size = 0x20;
		break;
	case 1:
		data |= 0x20;//set to 256 bytes = 64 DWORDS 
		//BData |= 0x00000020;//set to 256 
		//WriteLongIOPort( drvno, BData, PCIeAddr_DeviceControl );
		//NO_TLPS = 0x9;//x9 was before. 0x10 is calculated in aboutlp and 0x7 is working;
		tlp_size = 0x40;
		break;
	case 2:
		data |= 0x40;//set to 512 bytes = 128 DWORDS 
		//BData |= 0x00000020;//set to 512 
		//WriteLongIOPort( drvno, BData, PCIeAddr_DeviceControl );
		//NO_TLPS = 0x5;
		tlp_size = 0x80;
		break;
	}
	if (MANUAL_OVERRIDE_TLP)
	{
		data |= 0x00; // sets max TLP size {0x00, 0x20, 0x40} <=> {128 B, 256 B, 512 B}
		tlp_size = 0x20; // sets utilized TLP size in DWORDS (1 DWORD = 4 byte)
		no_tlps = 0x11; // sets number of TLPs per scan/frame
	}
	status = writeConfig_32(drvno, data, PCIeAddr_DeviceControl);
	if (status != es_no_error) return status;
	uint64_t dma_physical_address = getPhysicalDmaAddress(drvno);
	// WDMATLPA (Reg name): write the lower part (bit 02:31) of the DMA address to the DMA controller
	ES_LOG("Set WDMATLPA to physical address of DMA buffer 0x%016lx\n", dma_physical_address);
	status = writeBitsDma_32(drvno, (uint32_t)dma_physical_address, 0xFFFFFFFC, DmaAddr_WDMATLPA);
	if (status != es_no_error) return status;
	//WDMATLPS: write the upper part (bit 32:39) of the address
	data = ((dma_physical_address >> 8) & 0xFF000000) | tlp_size;
	//64bit address enable
	if (DMA_64BIT_EN)
		data |= 1 << 19;
	ES_LOG("Set WDMATLPS to 0x%016x (0x%016x)\n", data, data & 0xFF081FFF);
	status = writeBitsDma_32(drvno, data, 0xFF081FFF, DmaAddr_WDMATLPS);
	if (status != es_no_error) return status;
	ES_LOG("set WDMATLPC to 0x%08x (0x%08x)\n", no_tlps, no_tlps & 0x0000FFFF);
	return writeBitsDma_32(drvno, no_tlps, 0xFFFF, DmaAddr_WDMATLPC);
}

/**
 * @brief Set specified bits to 1 in DMA register at memory address.
 * 
 * @param data 4 bytes (32 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFFFFFFFF - all bits 32 bits are written, 0 - no bits are written.
 * @param address Address of the register in DMA space.
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes writeBitsDma_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint16_t address  )
{
	uint32_t OldRegisterValues = 0;
	//read the old Register Values in the S0 Address Reg
	es_status_codes status = readRegisterDma_32( drvno, &OldRegisterValues, address );
	if (status != es_no_error) return status;
	//step 0: delete not needed "1"s
	data &= bitmask;
	//step 1: save Data as setbitmask for making this part human readable
	uint32_t Setbit_mask = data;
	//step 2: setting high bits in the Data
	uint32_t OldRegVals_and_SetBits = OldRegisterValues | Setbit_mask;
	//step 3: prepare to clear bits
	uint32_t Clearbit_mask = data | ~bitmask;
	//step 4: clear the low bits in the Data
	uint32_t NewRegisterValues = OldRegVals_and_SetBits & Clearbit_mask;
	//write the data to the S0 controller
	return writeRegisterDma_32( drvno, NewRegisterValues, address );
}

/**
 * @brief Set specified bits to 1 in DMA register at memory address.
 * 
 * @param data 1 bytes (8 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFF - all bits 8 bits are written, 0 - no bits are written.
 * @param address Address of the register in DMA space.
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes writeBitsDma_8( uint32_t drvno, uint8_t data, uint8_t bitmask, uint16_t address  )
{
	uint8_t OldRegisterValues = 0;
	//read the old Register Values in the S0 Address Reg
	es_status_codes status = readRegisterDma_8( drvno, &OldRegisterValues, address );
	if (status != es_no_error) return status;
	//step 0: delete not needed "1"s
	data &= bitmask;
	//step 1: save Data as setbitmask for making this part human readable
	uint8_t Setbit_mask = data;
	//step 2: setting high bits in the Data
	uint8_t OldRegVals_and_SetBits = OldRegisterValues | Setbit_mask;
	//step 3: prepare to clear bits
	uint8_t Clearbit_mask = data | ~bitmask;
	//step 4: clear the low bits in the Data
	uint8_t NewRegisterValues = OldRegVals_and_SetBits & Clearbit_mask;
	//write the data to the S0 controller
	return writeRegisterDma_8( drvno, NewRegisterValues, address );
}

/**
 * \brief Write 4 bytes to a register in DMA space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterDma_32( uint32_t drvno, uint32_t data, uint16_t address )
{
	return writeRegister_32(drvno, data, address);
}

/**
 * \brief Write 1 byte to a register in DMA space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes writeRegisterDma_8( uint32_t drvno, uint8_t data, uint16_t address )
{
	return writeRegister_8(drvno, data, address);
}

/**
 * \brief Read 4 bytes of a register in DMA space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readRegisterDma_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
	return readRegister_32(drvno, data, address);
}

/**
 * \brief Read 1 byte of a register in DMA space.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param data Read buffer.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readRegisterDma_8( uint32_t drvno, uint8_t* data, uint16_t address )
{
	return readRegister_8(drvno, data, address);
}

/**
 * @brief Set DMA Start Mode 
 * 
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param start_by_hardware true: every XCK h->l starts DMA by hardware, false: by software
 * @return es_status_codes 
 */
es_status_codes SetDmaStartMode( uint32_t drvno, bool start_by_hardware)
{
	ES_LOG("Set DMA start mode: %u\n", start_by_hardware);
	uint32_t data = 0;
	if (start_by_hardware)
		data = 0x40000000;
	return writeBitsS0_32( drvno, data, 0x40000000, S0Addr_IRQREG );
}

/**
 * \brief  This function is starting the measurement and returns when the measurement is done.
 *
 * When there are multiple boards, all boards are starting the measurement. Create a new thread for calling this function, when you don't want to have a blocking call.
 * 
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_camera_not_found
 *		- es_register_write_failed
 *		- es_already_running
 *		- es_parameter_out_of_range
 *		- es_setting_thread_priority_failed
 */
es_status_codes StartMeasurement()
{
	ES_LOG("\n*** Start Measurement ***\n");
	if (isRunning)
		return es_already_running;
	else
		isRunning = true;
	abortMeasurementFlag = false;
	es_status_codes status = es_no_error;
#ifndef MINIMAL_BUILD
	setTimestamp();
#endif
	measurement_cnt = 0;
	memset(data_available, 0, sizeof(size_t) * MAXPCIECARDS);
	continiousMeasurementFlag = (bool)settings_struct.contiuous_measurement;//0 or 1
	continiousPauseInMicroseconds = settings_struct.cont_pause_in_microseconds;
#ifndef MINIMAL_BUILD
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1 && settings_struct.camera_settings[drvno].write_to_disc)
			startWriteToDiscThead(drvno);
	}
#endif
	// Set the measure on hardware bit
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = setMeasureOn(drvno);
			if (status != es_no_error) return ReturnStartMeasurement(status);
		}
	}
	do
	{
		measurement_cnt++;
		ES_LOG("measurement count: %u\n", measurement_cnt);
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
				SetAllInterruptsDone(drvno);
		}
		// Reset the hardware block counter and scan counter.
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
			{
				status = ResetHardwareCounter(drvno);
				if (status != es_no_error) return ReturnStartMeasurement(status);
			}
		}
		// Reset the buffer write pointers and software ISR counter.
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
				ResetBufferWritePos(drvno);
		}
		// Only on Linux: Because on Linux it is not possible to copy data in the ISR,
		// a new thread is started here that copies the data from the DMA buffer to
		// the user buffer. The ISR is giving a signal to this thread when to copy data.
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
			{
				status = StartCopyDataToUserBufferThread(drvno);
				if (status != es_no_error) return ReturnStartMeasurement(status);
			}
		}
		// Increase the priority of the measurement thread to maximum.
		// This is done to decrease latency while doing handshakes between software and hardware.
		// The priority is reset to the old value when the block for loop is finished.
		status = SetPriority();
		if (status != es_no_error) return ReturnStartMeasurement(status);
		// Block read for loop.
		for (uint32_t blk_cnt = 0; blk_cnt < *Nob; blk_cnt++)
		{
			// Increase the block count hardware register.
			// This must be done, before the block starts, so doing this first is a good idea.
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
			{
				// Check if the drvno'th bit is set
				if ((settings_struct.board_sel >> drvno) & 1)
				{
					status = countBlocksByHardware(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
				}
			}
			// The software polls the block trigger hardware register in a loop,
			// so the software is trapped here, until the hardware gives the signal
			// via this register. Pressing ESC can cancel this loop.
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
			{
				// Check if the drvno'th bit is set
				if ((settings_struct.board_sel >> drvno) & 1)
				{
					status = waitForBlockTrigger(drvno);
					if (status == es_abortion)
					{
						status = AbortMeasurement();
						return ReturnStartMeasurement(status);
					}
					else if (status != es_no_error) return ReturnStartMeasurement(status);
					// Only wait for the block trigger of one board
					break;
				}
			}
			ES_LOG("Block %u triggered\n", blk_cnt);
			// setBlockOn, StartSTimer and DoSoftwareTrigger are starting the measurement.
			// timer must be started in each block as the scan counter stops it by hardware at end of block
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
			{
				// Check if the drvno'th bit is set
				if ((settings_struct.board_sel >> drvno) & 1)
				{
					status = StartSTimer(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					status = setBlockOn(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					//start scan for first read if area or ROI
					if (*useSWTrig) status = DoSoftwareTrigger(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					timerOn[drvno] = true;
				}
				else
					timerOn[drvno] = false;
			}
			// Main read loop. The software waits here until the flag RegXCKMSB:b30 = TimerOn is reset by hardware,
			// if flag HWDREQ_EN is TRUE.
			// This is done when nos scans are counted by hardware. Pressing ESC can cancel this loop.
			ES_LOG("Wait for the end of block %u.\n", blk_cnt);
			while (timerOn[0] || timerOn[1] || timerOn[2] || timerOn[3] || timerOn[4])
			{
				for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
				{
					// Check if the drvno'th bit is set
					if ((settings_struct.board_sel >> drvno) & 1)
					{
						if ((FindCam(drvno) != es_no_error) || abortMeasurementFlag)
						{
							status = AbortMeasurement();
							return ReturnStartMeasurement(status);
						}
						if(!abortMeasurementFlag && checkEscapeKeyState())
							abortMeasurementFlag = true;
						status = IsTimerOn(drvno, &timerOn[drvno]);
						if (status != es_no_error) return ReturnStartMeasurement(status);
					}
				}
			}
			ES_LOG("Block %u done.\n", blk_cnt);
			// When the software reaches this point, all scans for the current block are done.
			// So blockOn is reset here.
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
			{
				// Check if the drvno'th bit is set
				if ((settings_struct.board_sel >> drvno) & 1)
				{
					status = resetBlockOn(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
				}
			}
		// This is the end of the block for loop. Until nob is reached this loop is repeated.
		}
		// Reset the thread priority to the previous value.
		status = ResetPriority();
		if (status != es_no_error) return ReturnStartMeasurement(status);
		// Only on Linux: The following mutex prevents ending the measurement before all data has been copied from DMA buffer to user buffer.
#ifdef __linux__
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
			{
				pthread_mutex_lock(&mutex[drvno]);
				pthread_mutex_unlock(&mutex[drvno]);
			}
		}
#endif
		WaitForAllInterruptsDone();
		// When the number of scans is not a integer multiple of 500 there will be data in the DMA buffer
		// left, which is not copied to the user buffer. The copy process for these scans is done here.
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
			{
				status = StopSTimer(drvno);
				if (status != es_no_error) return ReturnStartMeasurement(status);
				if (!settings_struct.camera_settings[drvno].use_software_polling)
				{
					status = GetLastBufPart(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
				}
			}
		}
		notifyAllBlocksDone();
		// Maybe this is not needed anymore because of WaitForAllInterruptsDone
		// This sleep is here to prevent the measurement being interrupted too early. When operating with 2 cameras the last scan could be cut off without the sleep. This is only a workaround. The problem is that the software is waiting for RSTIMER being reset by the hardware before setting measure on and block on to low, but the last DMA is done after RSTIMER being reset. BLOCKON and MEASUREON should be reset after all DMAs are done.
		// RSTIMER --------________
		// DMAWRACT _______-----___
		// BLOCKON ---------_______
		// MEASUREON ---------_____
		//WaitforTelapsed(100);
		// When space key or ESC key was pressed, continuous measurement stops.
		if (continiousMeasurementFlag && checkSpaceKeyState())
			continiousMeasurementFlag = false;
		if (!abortMeasurementFlag && checkEscapeKeyState())
			abortMeasurementFlag = true;
		WaitforTelapsed(continiousPauseInMicroseconds);
	} while (continiousMeasurementFlag && !abortMeasurementFlag);
	// Reset the hardware bit measure on.
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = resetMeasureOn(drvno);
			if (status != es_no_error) return ReturnStartMeasurement(status);
		}
	}
	ES_LOG("*** Measurement done ***\n\n");
	return ReturnStartMeasurement(status);
}

/**
 * \brief This is a helper function to return startMeasurement.
 * 
 * This function sets isRunning = false and returns the given status.
 * \param status Status that will be returned.
 * \return Returns input parameter status.
 */
es_status_codes ReturnStartMeasurement(es_status_codes status)
{
	isRunning = false;
	return status;
}

/**
 * \brief Test if SFP module is there and fiber is linked up.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes FindCam( uint32_t drvno )
{
	if (settings_struct.camera_settings[drvno].camcnt == 0)
	{
		// Camcnt is 0. FindCam is returning without error
		return es_no_error;
	}
	bool linkUp = false;
	bool sfpError = false;
	bool firstFailure = true;
	es_status_codes status;
#ifdef WIN32
	int64_t timestampFirstFailureInMicroseconds = GetTimestampInMicroseconds();
	int64_t timeoutInMicroseconds = 10000;
	// Check the connection again while timout is not reached.
	// This loop is here to prevent checking during link down spikes
	while(GetTimestampInMicroseconds() - timestampFirstFailureInMicroseconds < timeoutInMicroseconds)
	{
#endif
		status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_linkup_sfp1, &linkUp);
		if (status != es_no_error) return status;
		status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_error_sfp1, &sfpError);
		if (status != es_no_error) return status;
		// When link up and sfp error are correct, the function is allowed to return
		if (linkUp && !sfpError)
			return es_no_error;
		// When a failure is detected set a timestamp on the first failure
#ifdef WIN32
		else if (firstFailure)
		{
			timestampFirstFailureInMicroseconds = GetTimestampInMicroseconds();
			firstFailure = false;
		}
	}
#endif
	ES_LOG("Fiber or Camera error, connection timeout\n");
	return es_camera_not_found;
}

/**
 * \brief Reset the hardware block counter and scan counter.
 * 
 * \param drvno board number
 * \return es_status_codes
 *		- es_no_error
 * 		- es_register_read_failed
 *		- es_register_write_failed
*/
es_status_codes ResetHardwareCounter( uint32_t drvno )
{
	ES_LOG( "Reset hardware counter\n" );
	es_status_codes status = pulseBitS0_32(drvno, DMAsPerIntr_bitindex_counter_reset, S0Addr_DMAsPerIntr);
	if (status != es_no_error) return status;
	status = pulseBitS0_32(drvno, DmaBufSizeInScans_bitindex_counter_reset, S0Addr_DmaBufSizeInScans);
	if (status != es_no_error) return status;
	status = pulseBitS0_32(drvno, BLOCKINDEX_bitindex_counter_reset, S0Addr_BLOCKINDEX);
	if (status != es_no_error) return status;
	return pulseBitS0_32(drvno, ScanIndex_bitindex_counter_reset, S0Addr_ScanIndex);
}

/**
 * \brief Reset the internal intr collect counter.
 * 
 * \param drvno board number
 * \param stop_by_hardware true: timer is stopped by hardware if nos is reached
 * \return es_status_codes
 *		- es_no_error
 * 		- es_register_read_failed
 *		- es_register_write_failed
*/
es_status_codes SetHardwareTimerStopMode( uint32_t drvno, bool stop_by_hardware )
{
	ES_LOG( "Set hardware timer stop mode: %u\n", stop_by_hardware);
	es_status_codes status;
	if (stop_by_hardware)
		//when SCANINDEX reaches NOS, the timer is stopped by hardware.
		status = setBitS0_32(drvno, PCIEFLAGS_bitindex_ENRSTIMERHW, S0Addr_PCIEFLAGS);
	else
		//stop only with write to RS_Timer Reg
		status = resetBitS0_32(drvno, PCIEFLAGS_bitindex_ENRSTIMERHW, S0Addr_PCIEFLAGS);
	return status;
}

/**
 * @brief Pulse bit () -> 1 -> 0) in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes pulseBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	es_status_codes status = setBitS0_32(drvno, bitnumber, address);
	if (status != es_no_error) return status;
	return resetBitS0_32(drvno, bitnumber, address);
}

/**
 * @brief Pulse bit () -> 1 -> 0) in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...7, 0 is LSB, 7 MSB
 * @param address register address. 1 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes pulseBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	es_status_codes status = setBitS0_8(drvno, bitnumber, address);
	if (status != es_no_error) return status;
	return resetBitS0_8(drvno, bitnumber, address);
}

/**
 * @brief Wait in loop until block trigger occurs.
 * If block trigger high: return
 * If block trigger low: wait for hi
 * Checks only PCIE board no 1
 * 
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0. May only work for 1
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes waitForBlockTrigger(uint32_t drvno)
{
	ES_LOG("Wait for block trigger\n");
	uint8_t data=0;
	es_status_codes status;
	while (!abortMeasurementFlag)
	{
		if (!abortMeasurementFlag && checkEscapeKeyState())
			abortMeasurementFlag = true;
		status = readRegisterS0_8( drvno, &data, S0Addr_CTRLA );
		if (status != es_no_error) return status;
		if ((data & CTRLA_bit_TSTART) > 0)
			return es_no_error;
	}
	return es_abortion;
}

/**
 * \brief Sends signal to hardware to count blocks
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes countBlocksByHardware( uint32_t drvno )
{
	ES_LOG("Increase hardware block counter\n");
	es_status_codes status =  pulseBitS0_32(drvno, PCIEFLAGS_bitindex_BLOCKTRIG, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	//reset scan counter
	return pulseBitS0_32(drvno, ScanIndex_bitindex_counter_reset, S0Addr_ScanIndex);
}

/**
 * \brief Sets Scan Timer on.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes StartSTimer( uint32_t drvno )
{
	ES_LOG("Start S Timer\n");
	return setBitS0_8(drvno, XCKMSB_bitindex_stimer_on, S0Addr_XCKMSB);
}

/**
 * \brief Triggers one camera read by calling this function.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes DoSoftwareTrigger( uint32_t drvno )
{
	ES_LOG("Do software trigger\n");
	es_status_codes status = setBitS0_8(drvno, FFCTRL_bitindex_SWTRIG, S0Addr_FFCTRL);
	if (status != es_no_error) return status;
	return resetBitS0_8(drvno, FFCTRL_bitindex_SWTRIG, S0Addr_FFCTRL);
}

/**
 * \brief Checks if timer is active (Bit30 of XCK-Reg).
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param on
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes IsTimerOn( uint32_t drvno, bool* on )
{	
	return ReadBitS0_8(drvno, S0Addr_XCKMSB, XCKMSB_bitindex_stimer_on, on);
}

/**
 * \brief For the rest part of the buffer.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes GetLastBufPart( uint32_t drvno )
{
	ES_TRACE( "Get the last buffer part\n" );
	// Get the rest if buffer is not multiple of 500 (BUFSIZEINSCANS/2)
	// Also if nos is < BUFSIZEINSCANS/2 there is data left in the DMA buffer, because no interrupt occurred after the last scans.
	uint32_t spi = 0;
	// Get scans per interrupt
	es_status_codes status = readRegisterS0_32( drvno, &spi, S0Addr_DMAsPerIntr ); 
	if (status != es_no_error) return status;
	// dmaHalfBufferSize is 500 with default values
	uint32_t dmaHalfBufferSize = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS;
	uint32_t scans_all_cams = (*Nospb) * (*Nob) * aCAMCNT[drvno];
	uint32_t rest_overall = scans_all_cams % dmaHalfBufferSize; 
	size_t rest_in_bytes = rest_overall * aPIXEL[drvno] * sizeof(uint16_t);
	ES_TRACE( "nos: %u, nob: %u, scansPerInterrupt: %u, camcnt: %u\n", (*Nospb), *Nob, spi, aCAMCNT[drvno]);
	ES_TRACE( "scans_all_cams: %u \n", scans_all_cams );
	ES_TRACE( "rest_overall: %u, rest_in_bytes: %u\n", rest_overall, rest_in_bytes );
	if (rest_overall)
		copyRestData(drvno, rest_in_bytes); 
	return status;
}

/**
 * \brief Initializes the PCIe board.
 * 
 * Call this after InitDriver and before InitMeasurement. It is only needed to be called once.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_invalid_driver_number
 *		- es_getting_device_info_failed
 *		- es_open_device_failed
 */
es_status_codes InitBoard()
{
	ES_LOG("\n*** Init board ***\n");
	// Initialize settings struct
	for (uint32_t drvno = 0; drvno < MAXPCIECARDS; drvno++)
		memcpy(&settings_struct.camera_settings[drvno], &camera_settings_default, sizeof(struct camera_settings));
	ES_LOG("Number of boards: %u\n", number_of_boards);
	if (number_of_boards < 1) return es_open_device_failed;
	es_status_codes status;
	for (int drvno = 0; drvno < number_of_boards; drvno++)
	{
		status = _InitBoard(drvno);
		if (status != es_no_error) return status;
		InitMutex(drvno);
	}
	ES_LOG("*** Init board done***\n\n");
	return status;
}

/**
 * \brief Initialize the driver. 
 * 
 * Call this before any other action. It is only needed to be called once at startup.
 * \return es_status_codes:
 *		- es_setting_driver_name_failed
 *		- es_debug_init_failed
 *		- es_driver_init_failed
 *		- es_device_not_found
 *		- es_no_error
 */
es_status_codes InitDriver()
{
	ES_LOG("\n*** Init driver ***\n");
	es_status_codes status = _InitDriver();
	ES_LOG("*** Init driver done***\n\n");
	return status;
}

/**
 * \brief Exit driver. Call this before exiting software for cleanup.
 * 
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_unlocking_dma_failed
 *		- es_parameter_out_of_range
 */
es_status_codes ExitDriver()
{
	ES_LOG("\n*** Exit driver ***\n");
	es_status_codes status = es_no_error;
	for (int drvno = 0; drvno < number_of_boards; drvno++)
	{
		status = CleanupDriver(drvno);
	}
	status = _ExitDriver();
	ES_LOG("*** Exit driver done***\n\n");
	return status;
}

/**
 * \brief Get data of a single measurement.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number ( 0...(nos - 1) )
 * \param block block number ( 0...(nob - 1) )
 * \param camera camera number ( 0...(CAMCNT - 1) )
 * \param pdest Pointer where frame data will be written. Make sure that the size is >= sizeof(uint16_t) * pixel
 * \param pixel Lenght of the frame to copy. Typically = pixel
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_memory_not_initialized
 */
es_status_codes ReturnFrame(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t pixel, uint16_t* pdest)
{
	//ES_TRACE( "Return frame: drvno: %u, sample: %u, block: %u, camera: %u, pdest %p, pixel: %u\n", drvno, sample, block, camera, pdest, pixel );
	uint16_t* pframe = NULL;
	es_status_codes status = GetAddressOfPixel( drvno, 0, sample, block, camera, &pframe);
	if (status != es_no_error) return status;
	//ES_LOG("pframe %p\n", pframe);
	//ES_LOG("userBuffer %p\n", userBuffer[drvno]);
	memcpy( pdest, pframe, pixel * sizeof( uint16_t ) );
	return status;
}


/**
 * \brief Returns the index of a pixel located in userBuffer.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param pixel position in one scan (0...(PIXEL-1))
 * \param sample position in samples (0...(nos-1))
 * \param block position in blocks (0...(nob-1))
 * \param camera position in camera count (0...(CAMCNT-1)
 * \param pIndex Pointer to index of pixel.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_memory_not_initialized
 */
es_status_codes GetIndexOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint64_t* pIndex )
{
	if (pixel >= aPIXEL[drvno] || sample >= *Nospb || block >= *Nob || camera >= aCAMCNT[drvno])
		return es_parameter_out_of_range;
	//init index with base position of pixel
	uint64_t index = pixel;
	//position of index at camera position
	index += (uint64_t)camera *((uint64_t)aPIXEL[drvno]); // AM! was +4 for PCIe version before P202_23 to shift scan counter in 2nd camera to the left
	//position of index at sample
	index += (uint64_t)sample * (uint64_t)aCAMCNT[drvno] * (uint64_t)aPIXEL[drvno];
	//position of index at block
	index += (uint64_t)block * (uint64_t)(*Nospb) * (uint64_t)aCAMCNT[drvno] * (uint64_t)aPIXEL[drvno];
	*pIndex = index;
	return es_no_error;
}

/**
 * \brief Returns the address of a pixel located in userBuffer.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param pixel position in one scan (0...(PIXEL-1))
 * \param sample position in samples (0...(nos-1))
 * \param block position in blocks (0...(nob-1))
 * \param camera position in camera count (0...(CAMCNT-1))
 * \param address Pointer to get address
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_memory_not_initialized
 */
es_status_codes GetAddressOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** address )
{
	uint64_t index = 0;
	es_status_codes status = GetIndexOfPixel(drvno, pixel, sample, block, camera, &index);
	if (status != es_no_error) return status;
	if (!userBuffer[drvno])
		return es_memory_not_initialized;
	*address = &userBuffer[drvno][index];
	return status;
}

/**
 * \brief Calculate the theoretical time needed for one measurement.
 *
 * \param nos number of samples
 * \param nob number of blocks
 * \param exposure_time_in_ms exposure time in ms
 * \return time in seconds
 */
double CalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms)
{
	double measureTime = (double)nos * (double)nob * exposure_time_in_ms / 1000;
	return measureTime;
}

/**
 * \brief Calculate needed RAM in MB for given nos and nob.
 *
 * \param nos number of samples
 * \param nob number of blocks
 * \return RAM in MB
 */
double CalcRamUsageInMB(uint32_t nos, uint32_t nob)
{
	ES_LOG("Calculate ram usage in MB, nos: %u:, nob: %u\n", nos, nob);
	double ramUsage = 0;
	for (int i = 0; i < number_of_boards; i++)
		ramUsage += (uint64_t)nos * (uint64_t)nob * (uint64_t)aPIXEL[i] * (uint64_t)aCAMCNT[i] * sizeof(uint16_t);
	ramUsage = ramUsage / 1048576;
	ES_LOG("Ram usage: %f\n", ramUsage);
	return ramUsage;
}

/**
 * \brief Calculate TRMS noise value of one pixel.
 *
 * Calculates RMS of TRMS_pixel in the range of samples from firstSample to lastSample. Only calculates RMS from one block.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param firstSample start sample to calculate RMS. 0...(nos-2). Typical value: 10, to skip overexposed first samples
 * \param lastSample last sample to calculate RMS. firstSample+1...(nos-1).
 * \param TRMS_pixel pixel for calculating noise (0...(PIXEL-1))
 * \param CAMpos index for camcount (0...(CAMCNT-1))
 * \param mwf pointer for mean value
 * \param trms pointer for noise
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_allocating_memory_failed
 *		- es_memory_not_initialized
 */
es_status_codes CalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms)
{
	if (firstSample >= lastSample || lastSample > *Nospb)
	{
		//error: firstSample must be smaller than lastSample
		ES_LOG("Calc Trms failed. lastSample must be greater than firstSample and both in boundaries of nos, drvno: %u, firstSample: %u, lastSample: %u, TRMS_pixel: %u, CAMpos: %u, Nospb: %u\n", drvno, firstSample, lastSample, TRMS_pixel, CAMpos, *Nospb);
		*mwf = -1;
		*trms = -1;
		return es_parameter_out_of_range;
	}
	uint32_t samples = lastSample - firstSample;
	uint16_t *TRMS_pixels = calloc(samples, sizeof(uint16_t));
	if (!TRMS_pixels) return es_allocating_memory_failed;
	//storing the values of one pixel for the RMS analysis
	for (uint32_t scan = 0; scan < samples; scan++)
	{
		uint64_t TRMSpix_of_current_scan = 0;
		es_status_codes status = GetIndexOfPixel(drvno, (uint16_t)TRMS_pixel, scan + firstSample, 0, CAMpos, &TRMSpix_of_current_scan);
		if (status != es_no_error) return status;
		TRMS_pixels[scan] = userBuffer[drvno][TRMSpix_of_current_scan];
	}
	//RMS analysis
	GetRmsVal(samples, TRMS_pixels, mwf, trms);
	free(TRMS_pixels);
	return es_no_error;
}//CalcTrms

void GetRmsVal(uint32_t nos, uint16_t *TRMSVals, double *mwf, double *trms)
{
	*trms = 0.0;
	*mwf = 0.0;
	double sumvar = 0.0;

	for (uint32_t i = 0; i < nos; i++)
	{//get mean val
		*mwf += TRMSVals[i];
	}
	*mwf /= nos;
	for (uint32_t i = 0; i < nos; i++)
	{// get variance
		*trms = TRMSVals[i];
		*trms = *trms - *mwf;
		*trms *= *trms;
		sumvar += *trms;
	}
	*trms = sumvar / (nos + 1);
	*trms = sqrt(*trms);
	return;
}//GetRmsVal

/**
 * \brief Checks content of FIFO.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param valid Is true (not 0) if FIFO keeps >= 1 complete lines (linecounter>0).
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes CheckFifoValid(uint32_t drvno, bool* valid)
{	// not empty & XCK = low -> true
	ES_LOG("checkFifoFlags\n");
	return ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_valid, valid);
}

/**
 * \brief Check ovl flag (overflow of FIFO).
 *
 * If occurred stays active until a call of FFRS.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param overflow
 * \return Is true (not 0) if overflow occurred (linecounter>0).
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes CheckFifoOverflow(uint32_t drvno, bool* overflow)
{
	ES_LOG("checkFifoOverflow\n");
	return ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_overflow, overflow);
}

/**
 * \brief Check empty flag (FIFO empty).
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param empty
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes CheckFifoEmpty(uint32_t drvno, bool* empty)
{
	ES_LOG("checkFifoEmpty\n");
	return ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_empty, empty);
}

/**
 * \brief Check full flag (FIFO full).
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param full
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes CheckFifoFull(uint32_t drvno, bool* full)
{
	ES_LOG("checkFifoFull\n");
	return ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_full, full);
}

/**
 * \brief Check if blockon bit is set.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param blockOn True when blockon bit is set.
 *  \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes isBlockOn(uint32_t drvno, bool* blockOn)
{
	return ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_BLOCKON, blockOn);
}

/**
 * \brief Check if measure on bit is set.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param measureOn True when measureon bit is set.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes isMeasureOn(uint32_t drvno, bool* measureOn)
{
	return ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_MEASUREON, measureOn);
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
es_status_codes SetLedOff(uint32_t drvno, uint8_t LED_OFF)
{
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_LEDoff, (uint16_t)LED_OFF);
}

/**
 * \brief Reset trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OutTrigLow(uint32_t drvno)
{
	return resetBitS0_32(drvno, CTRLA_bitindex_TRIG_OUT, S0Addr_CTRLA);
}

/**
 * \brief Set trigger out(Reg CtrlA:D3) of PCIe board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OutTrigHigh(uint32_t drvno)
{
	return setBitS0_32(drvno, CTRLA_bitindex_TRIG_OUT, S0Addr_CTRLA);
}

/**
 * \brief Pulses trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param pulseWidthInMicroseconds duration of pulse in us
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OutTrigPulse(uint32_t drvno, int64_t pulseWidthInMicroseconds)
{
	es_status_codes status = OutTrigHigh(drvno);
	if (status != es_no_error) return status;
	WaitforTelapsed(pulseWidthInMicroseconds);
	return OutTrigLow(drvno);
}

/**
 * \brief Reads the binary state of an ext. trigger input.
 *
 * Direct read of inputs for polling.
 * \param drvno board number
 * \param btrig_ch specify input channel
 * 			- btrig_ch=0 not used
 * 			- btrig_ch=1 is PCIe trig in I
 * 			- btrig_ch=2 is S1
 * 			- btrig_ch=3 is S2
 * 			- btrig_ch=4 is S1&S2
 * 			- btrig_ch=5 is TSTART (GTI - DAT - EC)
 * \param state false when low, otherwise true
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readBlockTriggerState(uint32_t drvno, uint8_t btrig_ch, bool* state)
{
	uint8_t val = 0;
	*state = false;
	es_status_codes status = es_no_error;
	switch (btrig_ch)
	{
	default:
	case 0:
		*state = true;
		break;
	case 1: //I
		status = readRegisterS0_8(drvno, &val, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		if ((val & CTRLA_bit_DIR_TRIGIN) > 0) *state = true;
		break;
	case 2: //S1
		status = readRegisterS0_8(drvno, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x02) > 0) *state = true;
		break;
	case 3: //S2
		status = readRegisterS0_8(drvno, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x04) > 0) *state = true;
		break;
	case 4: // S1&S2
		status = readRegisterS0_8(drvno, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x02) == 0) *state = false;
		status = readRegisterS0_8(drvno, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x04) == 0) *state = false;
		*state = true;
		break;
	case 5: // TSTART
		status = readRegisterS0_8(drvno, &val, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		if ((val & CTRLA_bit_TSTART) > 0) *state = true;
		break;
	}
	return status;
}

/**
 * \brief Returns when block on bit is 0.
 *
 * \param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes waitForBlockReady(uint32_t board_sel)
{
	bool blockOn[MAXPCIECARDS] = { false, false, false, false, false };
	es_status_codes status = es_no_error;
	do
	{
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((board_sel >> drvno) & 1)
			{
				status = isBlockOn(drvno, &blockOn[drvno]);
				if (status != es_no_error) return status;
			}
		}
	} while (blockOn[0] || blockOn[1] || blockOn[2] || blockOn[3] || blockOn[4]);
	return status;
}

/**
 * \brief Returns when measure on bit is 0.
 *
 * \param board_sel select PCIe boards bitwise: bit 0 - board 0...
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes waitForMeasureReady(uint32_t board_sel)
{
	bool measureOn[MAXPCIECARDS] = { false, false, false, false, false };
	es_status_codes status = es_no_error;
	do
	{
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((board_sel >> drvno) & 1)
			{
				status = isMeasureOn(drvno, &measureOn[drvno]);
				if (status != es_no_error) return status;
			}
		}
	} while (measureOn[0] || measureOn[1] || measureOn[2] || measureOn[3] || measureOn[4]);
	return status;
}

es_status_codes dumpS0Registers(uint32_t drvno, char** stringPtr)
{
	enum N
	{ 
		number_of_registers = 47,
		bufferLength = 40
	};
	char register_names[number_of_registers][bufferLength] = {
		"DBR"DLLTAB,
		"CTRLA"DLLTAB,
		"XCKLL"DLLTAB,
		"XCKCNTLL",
		"PIXREG"DLLTAB,
		"FIFOCNT"DLLTAB,
		"VCLKCTRL",
		"'EBST'"DLLTAB,
		"DAT"DLLTAB,
		"EC"DLLTAB,
		"TOR"DLLTAB,
		"ARREG"DLLTAB,
		"GIOREG"DLLTAB,
		"nc"DLLTAB,
		"IRQREG"DLLTAB,
		"PCI board version",
		"R0 PCIEFLAGS",
		"R1 NOS"DLLTAB,
		"R2 SCANINDEX",
		"R3 DMABUFSIZE",
		"R4 DMASPERINTR",
		"R5 BLOCKS",
		"R6 BLOCKINDEX",
		"R7 CAMCNT",
		"R8 GPX Ctrl",
		"R9 GPX Data",
		"R10 ROI 0",
		"R11 ROI 1",
		"R12 ROI 2",
		"R13 XCKDLY",
		"R14 S1S2 read delay",
		"R15 nc"DLLTAB,
		"R16 BTimer",
		"R17 BDAT",
		"R18 BEC"DLLTAB,
		"R19 BFLAGS",
		"R20 A1DSC",
		"R21 L1DSC",
		"R22 A2DSC",
		"R23 L2DSC",
		"R24 ATDC2",
		"R25 LTDC2",
		"R26 DSCCtrl",
		"R27 DAC",
		"R28 CAMSTATUS12",
		"R29 CAMSTATUS34",
		"R30 CAMERA TYPE"
	}; //Look-Up-Table for the S0 Registers
	uint32_t data = 0;
	//allocate string buffer
	*stringPtr = (char*) calloc(number_of_registers * bufferLength, sizeof(char));
	int len = 0;
	size_t bufferSize = number_of_registers * bufferLength;
	es_status_codes status = es_no_error;
	for (uint16_t i = 0; i <= number_of_registers - 1; i++)
	{
		//read register
		status = readRegisterS0_32(drvno, &data, i * 4);
		if (status != es_no_error)
		{
			//write error to buffer
			len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register 0x%x %s", i*4, register_names[i]);
			return status;
		}
		//write register name and value to buffer
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "0x%x  \t%s\t0x%x\n", i*4, register_names[i], data);
	}
	return status;
}



es_status_codes dumpHumanReadableS0Registers(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		bufferSize = 3000
	};
	char* charBuffer[bufferSize];
	int len = 0;
	*stringPtr = (char*)calloc(bufferSize, sizeof(char));
	bool isBitHigh;

	/*=======================================================================*/
	
	//CTRLA
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "CTRLA\n");

	//CTRLA VON
	es_status_codes status = ReadBitS0_8(drvno, S0Addr_CTRLA, CTRLA_bitindex_VONOFF, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tVON\t%i\n", isBitHigh);

	//CTRLA XCK
	status = ReadBitS0_8(drvno, S0Addr_CTRLA, CTRLA_bitindex_XCK, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tXCK\t % i\n", isBitHigh);

	//CTRLA TRIG OUT
	status = ReadBitS0_8(drvno, S0Addr_CTRLA, CTRLA_bitindex_TRIG_OUT, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTRIG OUT\t%i\n", isBitHigh);

	//CTRLA BOTH SLOPE
	status = ReadBitS0_8(drvno, S0Addr_CTRLA, CTRLA_bitindex_BOTH_SLOPE, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBOTH SLOPE\t%i\n", isBitHigh);

	//CTRLA SLOPE
	status = ReadBitS0_8(drvno, S0Addr_CTRLA, CTRLA_bitindex_SLOPE, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSLOPE\t%i\n", isBitHigh);

	//CTRLA STRIGIN
	status = ReadBitS0_8(drvno, S0Addr_CTRLA, CTRLA_bitindex_DIR_TRIGIN, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTRIGIN\t%i\n", isBitHigh);

	/*=======================================================================*/
	
	//CTRLB
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nCTRLB\n");

	bool sti0High, sti1High, sti2High;
	//CTRLB STI0
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_STI0, &isBitHigh);
	sti0High = isBitHigh;

	//CTRLB STI1
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_STI1, &isBitHigh);
	sti1High = isBitHigh;

	//CTRLB STI2
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_STI2, &isBitHigh);
	sti2High = isBitHigh;

	int combinedValueSTI = (sti2High << 2) | (sti1High << 1) | sti0High;

	switch (combinedValueSTI) {
	case 0: 
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tI\n");
		break;
	case 1:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tS1\n");
		break;
	case 2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tS2\n");
		break;
	case 3:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tS2ENI\n");
		break;
	case 4:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tST\n");
		break;
	case 5:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tASL\n");
		break;
	default:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTI\tinvalid\n");
		break;

	}

	//CTRLB SHON
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_SHON, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSHON\t%i\n", isBitHigh);


	bool bti0High, bti1High, bti2High;
	//CTRLB BTI0
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_BTI0, &bti0High);

	//CTRLB BTI1
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_BTI1, &bti1High);

	//CTRLB BTI2
	status = ReadBitS0_8(drvno, S0Addr_CTRLB, CTRLB_bitindex_BTI2, &bti2High);

	int combinedValueBTI = (bti2High << 2) | (bti1High << 1) | bti0High;

	switch (combinedValueBTI) 
	{
	case 0:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tI\n");
		break;
	case 1:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tS1\n");
		break;
	case 2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tS2\n");
		break;
	case 3:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tS1&S2\n");
		break;
	case 4:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tBT\n");
		break;
	case 5:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tS1 Chopper\n");
		break;
	case 6:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tS2 Chopper\n");
		break;
	case 7:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTI\tS1&S2 Chopper\n");
		break;
	}

	/*=======================================================================*/
	
	//CTRLC
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nCTRLC\n");

	//CTRLC I
	status = ReadBitS0_8(drvno, S0Addr_CTRLC, CTRLC_bitindex_I, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tI\t%i\n", isBitHigh);

	//CTRLC S1
	status = ReadBitS0_8(drvno, S0Addr_CTRLC, CTRLC_bitindex_S1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tS1\t%i\n", isBitHigh);

	//CTRLC S2
	status = ReadBitS0_8(drvno, S0Addr_CTRLC, CTRLC_bitindex_S2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tS2\t%i\n", isBitHigh);

	//CTRLC EOI
	status = ReadBitS0_8(drvno, S0Addr_CTRLC, CTRLC_bitindex_eoi, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEOI\t%i\n", isBitHigh);

	//CTRLC EOI-CHB
	status = ReadBitS0_8(drvno, S0Addr_CTRLC, CTRLC_bitindex_eoi_chb, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEOI-CHB\t%i\n", isBitHigh);

	/*=======================================================================*/
	
	//Register XCK
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister XCK\n");

	//XCK STimer
	uint8_t dataXCK1 = 0, dataXCK2 = 0, dataXCK3 = 0, dataXCK4 = 0;
	status = readRegisterS0_8(drvno, &dataXCK1, S0Addr_XCKLL);
	status = readRegisterS0_8(drvno, &dataXCK2, S0Addr_XCKLH);
	status = readRegisterS0_8(drvno, &dataXCK3, S0Addr_XCKHL);
	status = readRegisterS0_8(drvno, &dataXCK4, S0Addr_XCKMSB);

	uint32_t dataXCK = (uint32_t)(dataXCK4 & 0x0F) << 24 | (uint32_t)dataXCK3 << 16 | (uint32_t)dataXCK2 << 8 | (uint32_t)dataXCK1;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tXCK STIMER\t0x%x\n", dataXCK);

	//XCK Res_ns
	status = ReadBitS0_8(drvno, S0Addr_XCKMSB, XCKMSB_bitindex_reset_ns, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRes_ns\t%i\n", isBitHigh);

	//XCK Res_ms
	status = ReadBitS0_8(drvno, S0Addr_XCKMSB, XCKMSB_bitindex_reset_ms, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRes_ms\t%i\n", isBitHigh);

	//XCK RS
	status = ReadBitS0_8(drvno, S0Addr_XCKMSB, XCKMSB_bitindex_stimer_on, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRS\t%i\n", isBitHigh);

	/*=======================================================================*/
	
	//Register XCKCNT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister XCKCNT\n");

	uint8_t dataXCKCNT1 = 0, dataXCKCNT2 = 0, dataXCKCNT3 = 0, dataXCKCNT4 = 0;
	status = readRegisterS0_8(drvno, &dataXCKCNT1, S0Addr_XCKCNTLL);
	status = readRegisterS0_8(drvno, &dataXCKCNT2, S0Addr_XCKCNTLH);
	status = readRegisterS0_8(drvno, &dataXCKCNT3, S0Addr_XCKCNTHL);
	status = readRegisterS0_8(drvno, &dataXCKCNT4, S0Addr_XCKCNTMSB);

	uint32_t dataXCKCNT = (uint32_t)(dataXCKCNT4 & 0x0F) << 24 | (uint32_t)dataXCKCNT3 << 16 | (uint32_t)dataXCKCNT2 << 8 | (uint32_t)dataXCKCNT1;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tXCKCNT\t0x%x\n", dataXCKCNT);

	/*=======================================================================*/

	//Register PIXREG
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister PIXREG\n");

	uint8_t dataPIXREGHigh = 0, dataPIXREGLow = 0;
	status = readRegisterS0_8(drvno, &dataPIXREGHigh, S0Addr_PIXREGhigh);
	status = readRegisterS0_8(drvno, &dataPIXREGLow, S0Addr_PIXREGlow);

	uint32_t dataPIXREG = (uint16_t)(dataPIXREGHigh & 0x0F) << 8 | (uint16_t)dataPIXREGLow;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPIXREG\t%i\n", dataPIXREG);

	/*=======================================================================*/
	
	//FFCTRL
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nFFCTRL\n");

	//FFCTRL RSBTH
	status = ReadBitS0_8(drvno, S0Addr_FFCTRL, FFCTRL_bitindex_block_reset, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRSBTH\t%i\n", isBitHigh);

	//FFCTRL RSSTH
	status = ReadBitS0_8(drvno, S0Addr_FFCTRL, FFCTRL_bitindex_scan_reset, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRSSTH\t%i\n", isBitHigh);

	//FFCTRL SWTrig
	status = ReadBitS0_8(drvno, S0Addr_FFCTRL, FFCTRL_bitindex_SWTRIG, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSWTrig\t%i\n", isBitHigh);

	//FFCTRL RS_FF
	status = ReadBitS0_8(drvno, S0Addr_FFCTRL, FFCTRL_bitindex_RSFIFO, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRS_FF\t%i\n", isBitHigh);

	/*=======================================================================*/
	
	//FF_FLAGS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nFF_FLAGS\n");

	//FF_FLAGS BFTH
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_block_read, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBTFH\t%i\n", isBitHigh);

	//FF_FLAGS SFTH
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_scan_read, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSTFH\t%i\n", isBitHigh);

	//FF_FLAGS OVFL
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_overflow, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOVFL\t%i\n", isBitHigh);
	
	//FF_FLAGS XCKI no bitindex for xcki
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_xcki, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tXCKI\t%i\n", isBitHigh);
	
	//FF_FLAGS FF
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_full, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tFF\t%i\n", isBitHigh);

	//FF_FLAGS EF
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_empty, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEF\t%i\n", isBitHigh);

	//FF_FLAGS VALID
	status = ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_valid, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tVALID\t%i\n", isBitHigh);

	/*=======================================================================*/

	//FIFOCNT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nFIFOCNT\n");

	//WRCNT
	uint8_t dataFIFOCNT = 0;
	status = readRegisterS0_8(drvno, &dataFIFOCNT, S0Addr_FIFOCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tWRCNT\t%i\n", dataFIFOCNT);

	/*=======================================================================*/

	//VCLKCTRL
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nVCLKCTRL\n");

	//Register VCLKCNT
	uint32_t dataVCLKCNT = 0;
	status = readRegisterS0_32(drvno, &dataVCLKCNT, S0Addr_VCLKCTRL);
	dataVCLKCNT &= VCLKCNT_bit_control;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tVCLKCNT\t%i\n", dataVCLKCNT);

	//Register VCLKFREQ
	uint8_t dataVCLKFREQ = 0;
	status = readRegisterS0_8(drvno, &dataVCLKFREQ, S0Addr_VCLKFREQ);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tVCLKFREQ\t%i\n", dataVCLKFREQ);
	
	int translatedVCLKFREQ = "";
	
	if (dataVCLKFREQ == 0) translatedVCLKFREQ = 0;
	else translatedVCLKFREQ = VCLKFREQ_base_value + ((int)dataVCLKFREQ * VCLKFREQ_step_value);

	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTranslated VCLKFREQ\t%ins\n", translatedVCLKFREQ);

	/*=======================================================================*/

	//SDAT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nSDAT\n");
	uint32_t dataSDAT = 0;
	status = readRegisterS0_32(drvno, &dataSDAT, S0Addr_SDAT);

	uint32_t bitmaskSDAT = 1u << 30;
	
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEnabled\t%i\n", (dataSDAT & bitmaskSDAT));
	
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tDelay\t%i\n", (dataSDAT & SDAT_bit_control));

	/*=======================================================================*/

	//TOR Register
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister TOR\n");

	//TOCOUNT
	status = ReadBitS0_8(drvno, S0Addr_TOR_TOCNT, TOR_bitindex_TOCNT_EN, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTOCNT enabled\t%i\n", isBitHigh);
	uint8_t dataTOCNT = 0;
	status = readRegisterS0_8(drvno, &dataTOCNT, S0Addr_TOR_TOCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTOCNT\t%i\n\n", (dataTOCNT & TOR_bits_TOCNT));

	//TICOUNT
	status = ReadBitS0_8(drvno, S0Addr_TOR_TICNT, TOR_bitindex_TICNT_EN, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTICNT enabled\t%i\n", isBitHigh);
	uint8_t dataTICNT = 0;
	status = readRegisterS0_8(drvno, &dataTICNT, S0Addr_TOR_TICNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTICNT\t%i\n\n", (dataTICNT & TOR_bits_TICNT));

	//ISFFT
	status = ReadBitS0_8(drvno, S0Addr_TOR_MSB, TOR_MSB_bitindex_ISFFT_LEGACY, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tIS FFT\t%i\n\n", isBitHigh);

	//PCI Output
	bool isTO0 = 0, isTO1 = 0, isTO2 = 0, isTO3 = 0, isTOSEL = 0;
	status = ReadBitS0_8(drvno, S0Addr_TOR_MSB, TOR_MSB_bitindex_TO0, &isTO0);
	status = ReadBitS0_8(drvno, S0Addr_TOR_MSB, TOR_MSB_bitindex_TO1, &isTO1);
	status = ReadBitS0_8(drvno, S0Addr_TOR_MSB, TOR_MSB_bitindex_TO2, &isTO2);
	status = ReadBitS0_8(drvno, S0Addr_TOR_MSB, TOR_MSB_bitindex_TO3, &isTO3);
	status = ReadBitS0_8(drvno, S0Addr_TOR_MSB, TOR_MSB_bitindex_TOSEL, &isTOSEL);

	int combinedTO = (isTOSEL << 4) | (isTO3 << 3) | (isTO2 << 2) | (isTO1 << 1) | isTO0;

	switch (combinedTO)
	{
	case tor_xck:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tXCK\n", combinedTO);
		break;
	case tor_rego:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tREGO\n", combinedTO);
		break;
	case tor_von:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tVON\n", combinedTO);
		break;
	case tor_dma_act:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tDMA_ACT\n", combinedTO);
		break;
	case tor_asls:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tASLS\n", combinedTO);
		break;
	case tor_stimer:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tSTIMER\n", combinedTO);
		break;
	case tor_btimer:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue%i\n\t\tBTIMER\n", combinedTO);
		break;
	case tor_isr_act:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tISR_ACT\n", combinedTO);
		break;
	case tor_s1:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tS1\n", combinedTO);
		break;
	case tor_s2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tS2\n", combinedTO);
		break;
	case tor_bon:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue%i\n\t\tBON\n", combinedTO);
		break;
	case tor_measureon:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tMEASUREON\n", combinedTO);
		break;
	case tor_sdat:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tSDAT\n", combinedTO);
		break;
	case tor_bdat:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tBDAT\n", combinedTO);
		break;
	case tor_sec_mshut:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tSSHUT\n", combinedTO);
		break;
	case tor_bec_mshut:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tBSHUT\n", combinedTO);
		break;
	case tor_exposure_window:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tEXPOSURE_WINDOW\n", combinedTO);
		break;
	case tor_to_cnt_out:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tTO_CNT_OUT\n", combinedTO);
		break;
	case tor_secon:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tSECON\n", combinedTO);
		break;
	case tor_i:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tI\n", combinedTO);
		break;
	case tor_S1S2readDelay:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\S1S2readDelay\n", combinedTO);
		break;
	case tor_unused_23:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tUnused\n", combinedTO);
		break;
	case tor_unused_24:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tUnused\n", combinedTO);
		break;
	case tor_unused_25:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tUnused\n", combinedTO);
		break;
	case tor_unused_26:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tUnused\n", combinedTO);
		break;
	case tor_unused_27:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tUnused\n", combinedTO);
		break;
	case tor_unused_28:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tUnused\n", combinedTO);
		break;
	case tor_enffr:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tENFFR\n", combinedTO);
		break;
	case tor_enffw:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tENFFW\n", combinedTO);
		break;
	case tor_enffwrprot:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPCI Output\n\tValue\t%i\n\t\tENFFWRPROT\n", combinedTO);
		break;
	}

	/*=======================================================================*/

	//Register ARREG
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister ARREG\n");

	//Partial Binning enabled
	status = ReadBitS0_32(drvno, S0Addr_ARREG, ARREG_bitindex_partial_binning, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tPartial Binning\t%i\n", isBitHigh);

	//ROI Ranges
	uint16_t dataARREG = 0;
	status = readRegisterS0_16(drvno, &dataARREG, S0Addr_ARREG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tROI Ranges\t%i\n", (dataARREG & ARREG_bit_pb_control));

	/*=======================================================================*/

	//Register GIOREG
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister GIOREG\n");

	//Output 1
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 1\t%i\n", isBitHigh);
	
	//Output 2
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 2\t%i\n", isBitHigh);

	//Output 3
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O3, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 3\t%i\n", isBitHigh);

	//Output 4
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O4, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 4\t%i\n", isBitHigh);

	//Output 5
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O5, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 5\t%i\n", isBitHigh);

	//Output 6
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O6, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 6\t%i\n", isBitHigh);

	//Output 7
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O7, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 7\t%i\n", isBitHigh);

	//Output 8
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_O8, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tOutput 8\t%i\n", isBitHigh);

	//Input 1
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 1\t%i\n", isBitHigh);

	//Input 2
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 2\t%i\n", isBitHigh);

	//Input 3
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I3, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 3\t%i\n", isBitHigh);

	//Input 4
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I4, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 4\t%i\n", isBitHigh);

	//Input 5
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I5, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 5\t%i\n", isBitHigh);

	//Input 6
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I6, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 6\t%i\n", isBitHigh);

	//Input 7
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I7, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 7\t%i\n", isBitHigh);

	//Input 8
	status = ReadBitS0_32(drvno, S0Addr_GIOREG, GIOREG_bitindex_I8, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInput 8\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register IRQREG
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRegister IRQREG\n");

	//IRQLAT
	uint32_t dataIRQREG = 0;
	status = readRegisterS0_32(drvno, &dataIRQREG, S0Addr_IRQREG);
	uint32_t bistmaskIRQLAT = 0x0000FFFF;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tIRQLAT\t%i\n", (dataIRQREG & bistmaskIRQLAT));

	//IRQCNT
	uint32_t bitmaskIRQCNT = 0x3FFF0000;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tIRQCNT\t%i\n", (dataIRQREG & bitmaskIRQCNT) >> 16);

	//HWDREQ_EN
	status = ReadBitS0_32(drvno, S0Addr_IRQREG, IRQFLAGS_bitindex_HWDREQ_EN, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tHWDREQ_EN\t%i\n", isBitHigh);

	//ISR active
	status = ReadBitS0_32(drvno, S0Addr_IRQREG, IRQFLAGS_bitindex_INTRSR, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tISR active\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register PCIEFLAGS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nPCIEFLAGS\n");

	//XCKI
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_XCKI, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tXCKI\t%i\n", isBitHigh);

	//INTTRIG
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_INTTRIG, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tINTTRIG\t%i\n", isBitHigh);

	//ENRSTIMERHW
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_ENRSTIMERHW, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tENRSTIMERHW\t%i\n", isBitHigh);

	//USE_ENFFW_PROTECT
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_USE_ENFFW_PROTECT, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tUSE_ENFFW_PROTECT\t%i\n", isBitHigh);

	//BLOCKTRIG
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_BLOCKTRIG, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBLOCKTRIG\t%i\n", isBitHigh);

	//MEASUREON
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_MEASUREON, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tMeasure on\t%i\n", isBitHigh);

	//BON
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_BLOCKON, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBlock on\t%i\n", isBitHigh);

	//TDC
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_IS_TDC, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTDC\t%i\n", isBitHigh);
	
	//EWS
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_IS_DSC, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEWS\t%i\n", isBitHigh);

	//lnk_up SFP3
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_linkup_sfp3, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tlinkup SFP3\t%i\n", isBitHigh);

	//Error SFP3
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_error_sfp3, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tError SFP3\t%i\n", isBitHigh);

	//lnk_up SFP2
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_linkup_sfp2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tlinkup SFP2\t%i\n", isBitHigh);

	//Error SFP2
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_error_sfp2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tError SFP2\t%i\n", isBitHigh);

	//lnk_up SFP1
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_linkup_sfp1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tlinkup SFP1\t%i\n", isBitHigh);

	//Error SFP1
	status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_error_sfp1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tError SFP1\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register NOS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nNOS\n");

	//Number of Scans
	uint32_t dataNOS = 0;
	status = readRegisterS0_32(drvno, &dataNOS, S0Addr_NOS);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tNOS per Block\t%i\n", dataNOS);

	/*=======================================================================*/

	//Register SCANINDEX
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nScan index\n");

	//Value
	uint32_t dataSCANINDEX = 0;
	uint32_t bitmaskSCANINDEX = 0x7FFFFFFF;
	status = readRegisterS0_32(drvno, &dataSCANINDEX, S0Addr_ScanIndex);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", (dataSCANINDEX & bitmaskSCANINDEX));

	//Reset
	status = ReadBitS0_32(drvno, S0Addr_ScanIndex, ScanIndex_bitindex_counter_reset, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tReset\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register DMABUFSIZEINSCANS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nDMA buffer size in scans\n");
	
	//Value
	uint32_t dataDMABUFSIZE = 0;
	uint32_t bitmaskDMABUFSIZE = 0x7FFFFFFF;
	status = readRegisterS0_32(drvno, &dataDMABUFSIZE, S0Addr_DmaBufSizeInScans);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", (dataDMABUFSIZE & bitmaskDMABUFSIZE));

	//Reset
	status = ReadBitS0_32(drvno, S0Addr_DmaBufSizeInScans, DmaBufSizeInScans_bitindex_counter_reset, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tReset\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register DMASPERINTERRUPT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nDMAs per interrupt\n");

	//Value
	uint32_t dataDMASPERINTERRUPT = 0;
	uint32_t bitmaskDMASPERINTERRUPT = 0x7FFFFFFF;
	status = readRegisterS0_32(drvno, &dataDMASPERINTERRUPT, S0Addr_DMAsPerIntr);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", (dataDMASPERINTERRUPT & bitmaskDMASPERINTERRUPT));

	//Reset
	status = ReadBitS0_32(drvno, S0Addr_DMAsPerIntr, DMAsPerIntr_bitindex_counter_reset, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tReset\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register BLOCKS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nNOB\n");

	//Value
	uint32_t dataBLOCKS = 0;
	status = readRegisterS0_32(drvno, &dataBLOCKS, S0Addr_NOB);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", dataBLOCKS);

	/*=======================================================================*/

	//Register BLOCKINDEX
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nBlockindex\n");

	uint32_t dataBLOCKINDEX = 0;
	status = readRegisterS0_32(drvno, &dataBLOCKINDEX, S0Addr_BLOCKINDEX);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", dataBLOCKINDEX);


	/*=======================================================================*/

	//Register CAMCNT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nCam count\n");

	uint8_t dataCAMCNT = 0;
	uint8_t bitmaskCAMCNT = 0x0F;
	status = readRegisterS0_8(drvno, &dataCAMCNT, S0Addr_CAMCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", (dataCAMCNT & bitmaskCAMCNT));

	/*=======================================================================*/

	//Register TDC Control
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nTDC Control\n");

	//Reset
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_reset, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tReset\t%i\n", isBitHigh);

	//Interrupt
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_interrupt, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tInterrupt\t%i\n", isBitHigh);

	//LF1
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_load_fifo, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tLoad fifo\t%i\n", isBitHigh);

	//EF1
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_empty_fifo, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEmpty fifo\t%i\n", isBitHigh);

	//CSR
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_cs, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCSR\t%i\n", isBitHigh);

	//ADR
	bool isAdr0 = 0, isAdr1 = 0, isAdr2 = 0, isAdr3 = 0;
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_adr0, &isAdr0);
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_adr1, &isAdr1);
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_adr2, &isAdr2);
	status = ReadBitS0_32(drvno, S0Addr_TDCCtrl, TDCCtrl_bitindex_adr3, &isAdr3);

	int combinedAdr = (isAdr3 << 3) | (isAdr2 << 2) | (isAdr1 << 1) | isAdr0;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tADR\t%i\n", combinedAdr);

	/*=======================================================================*/

	//Register TDC Data
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nTDC Data\n");

	//Delay
	uint32_t dataTDCDATA = 0;
	status = readRegisterS0_32(drvno, &dataTDCDATA, S0Addr_TDCData);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tDelay\t%i\n", dataTDCDATA);

	/*=======================================================================*/

	//Register ROI
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nRange of interest\n");
	
	//Range 0
	uint32_t dataRange = 0;
	status = readRegisterS0_32(drvno, &dataRange, S0Addr_ROI0);
	uint16_t lowerBitsR0 = (uint16_t)(dataRange & 0xFFFF);
	uint16_t upperBitsR0 = (uint16_t)(dataRange >> 16);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tLower word R0\t%i\n", lowerBitsR0);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tUpper word R0\t%i\n", upperBitsR0);

	//Range 1
	status = readRegisterS0_32(drvno, &dataRange, S0Addr_ROI1);
	uint16_t lowerBitsR1 = (uint16_t)(dataRange & 0xFFFF);
	uint16_t upperBitsR1 = (uint16_t)(dataRange >> 16);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tLower word R1\t%i\n", lowerBitsR1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tUpper word R1\t%i\n", upperBitsR1);

	//Range 2
	status = readRegisterS0_32(drvno, &dataRange, S0Addr_ROI2);
	uint16_t lowerBitsR2 = (uint16_t)(dataRange & 0xFFFF);
	uint16_t upperBitsR2 = (uint16_t)(dataRange >> 16);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tLower word R2\t%i\n", lowerBitsR2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tUpper word R2\t%i\n", upperBitsR2);

	/*=======================================================================*/

	//Register XCKDLY
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nXCKDLY\n");

	//Delay value
	uint32_t dataXCKDLY = 0;
	uint32_t bitmaskXCKDLY = 0x7FFFFFFF;
	status = readRegisterS0_32(drvno, &dataXCKDLY, S0Addr_XCKDLY);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tDelay value\t%i\n", (dataXCKDLY & bitmaskXCKDLY));

	//Actual delay
	int val = 500 + ((int)(dataXCKDLY & bitmaskXCKDLY) * 10);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tActual delay\t%ins\n", val);

	/*=======================================================================*/

	//Register BTIMER
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nBTIMER\n");

	//Timer
	uint32_t dataBTIMER = 0;
	uint32_t bitmaskBTIMER = 0x0FFFFFFF;
	status = readRegisterS0_32(drvno, &dataBTIMER, S0Addr_BTIMER);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTimer\t%iμs\n", (dataBTIMER & bitmaskBTIMER));

	/*=======================================================================*/

	//Register BDAT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nBDAT\n");

	//Delay After Trigger
	uint32_t dataBDAT = 0;
	uint32_t bitmaskBDAT = 0x7FFFFFFF;
	status = readRegisterS0_32(drvno, &dataBDAT, S0Addr_BDAT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tDelay After Trigger\t%i\n", (dataBDAT & bitmaskBDAT));

	//Enabled
	status = ReadBitS0_32(drvno, S0Addr_BDAT, BDAT_bitindex_enabled, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tEnabled\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register BEC
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nBEC\n");

	//Exposure Control
	uint32_t dataBEC = 0;
	uint32_t bitmaskBEC = 0x7FFFFFFF;
	status = readRegisterS0_32(drvno, &dataBEC, S0Addr_BEC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tExposure Control\t%i\n", (dataBEC & bitmaskBEC));

	/*=======================================================================*/

	//Register BFLAGS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nBFLAGS\n");

	//BSLOPE
	status = ReadBitS0_32(drvno, S0Addr_BSLOPE, BSLOPE_bitindex_bslope, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBSLOPE\t%i\n", isBitHigh);

	//B both slopes
	status = ReadBitS0_32(drvno, S0Addr_BSLOPE, BSLOPE_bitindex_both_slopes, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBoth Slopes\t%i\n", isBitHigh);

	//BSWTRIG
	status = ReadBitS0_32(drvno, S0Addr_BSLOPE, BSLOPE_bitindex_bswtrig, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tBSLOPE\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register A1DSC (Actual Delay Stage Counter)
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nA1DSC\n");

	//CH1 DSC Value
	uint32_t dataA1DSC = 0;
	status = readRegisterS0_32(drvno, &dataA1DSC, S0Addr_A1DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCH1 DSC\t%i\n", dataA1DSC);

	/*=======================================================================*/

	//Register L1DSC (Last Delay Stage Counter)
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nL1DSC\n");

	//CH1 DSC Value
	uint32_t dataL1DSC = 0;
	status = readRegisterS0_32(drvno, &dataL1DSC, S0Addr_L1DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCH1 DSC\t%i\n", dataL1DSC);

	/*=======================================================================*/

	//Register A2DSC
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nA2DSC\n");

	//CH2 DSC Value
	uint32_t dataA2DSC = 0;
	status = readRegisterS0_32(drvno, &dataA2DSC, S0Addr_A2DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCH2 DSC\t%i\n", dataA2DSC);

	/*=======================================================================*/

	//Register L2DSC
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nL2DSC\n");

	//CH2 DSC Value
	uint32_t dataL2DSC = 0;
	status = readRegisterS0_32(drvno, &dataL2DSC, S0Addr_L2DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCH2 DSC\t%i\n", dataL2DSC);

	/*=======================================================================*/

	//Register ATDC2
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nATDC2\n");
	
	//TDC2
	uint32_t dataATDC2 = 0;
	status = readRegisterS0_32(drvno, &dataATDC2, S0Addr_ATDC2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTDC2\t%i\n", dataATDC2);

	/*=======================================================================*/

	//Register LTDC2
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nLTDC2\n");

	//TDC2
	uint32_t dataLTDC2 = 0;
	status = readRegisterS0_32(drvno, &dataLTDC2, S0Addr_LTDC2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTDC2\t%i\n", dataLTDC2);

	/*=======================================================================*/

	//Register DSCCtrl
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nDSCCtrl\n");

	//RS1
	status = ReadBitS0_32(drvno, S0Addr_DSCCtrl, DSCCtrl_bitindex_rs1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRS1\t%i\n", isBitHigh);

	//DIR1
	status = ReadBitS0_32(drvno, S0Addr_DSCCtrl, DSCCtrl_bitindex_dir1, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tDIR1\t%i\n", isBitHigh);

	//RS2
	status = ReadBitS0_32(drvno, S0Addr_DSCCtrl, DSCCtrl_bitindex_rs2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tRS2\t%i\n", isBitHigh);

	//DIR2
	status = ReadBitS0_32(drvno, S0Addr_DSCCtrl, DSCCtrl_bitindex_dir2, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tDIR2\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register DAC
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nDAC\n");

	//Value
	uint32_t dataDAC = 0;
	status = readRegisterS0_32(drvno, &dataDAC, S0Addr_DAC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tValue\t%i\n", dataDAC);

	/*=======================================================================*/

	//Register CAMSTATUS12
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nCAMSTATUS12\n");

	//Status ID
	uint32_t dataCAMSTATUS12 = 0;
	uint32_t bitmaskCAMSTATUS12 = 0xFFFFFFF7;
	status = readRegisterS0_32(drvno, &dataCAMSTATUS12, S0Addr_CAMSTATUS12);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tStaus ID\t%i\n", (dataCAMSTATUS12 & bitmaskCAMSTATUS12));

	//Temp
	status = ReadBitS0_32(drvno, S0Addr_CAMSTATUS12, CAMSTATUS12_bitindex_temp, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTemp Good\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register CAMSTATUS34
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nCAMSTATUS34\n");

	//Status ID
	uint32_t dataCAMSTATUS34 = 0;
	uint32_t bitmaskCAMSTATUS34 = 0xFFFFFFF7;
	status = readRegisterS0_32(drvno, &dataCAMSTATUS34, S0Addr_CAMSTATUS34);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tStatus ID\t%i\n", (dataCAMSTATUS34 & bitmaskCAMSTATUS34));

	//Temp
	status = ReadBitS0_32(drvno, S0Addr_CAMSTATUS34, CAMSTATUS34_bitindex_temp, &isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tTemp Good\t%i\n", isBitHigh);

	/*=======================================================================*/

	//Register Camera Type
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nCamera Type\n");

	//Sensor Type
	uint32_t dataCAMTYPE = 0;
	status = readRegisterS0_32(drvno, &dataCAMTYPE, S0Addr_CAMERA_TYPE);
	uint16_t lowerBitsCAMTYPE = (uint16_t)(dataCAMTYPE & 0x0000FFFF);
	uint16_t upperBitsCAMTYPE = (uint16_t)(dataCAMTYPE >> 16) & 0x0000FFFF;
	switch (lowerBitsCAMTYPE)
	{
	case 0x0000:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSensor Type\t%s\n", "PDA");
		break;
	case 0x0001:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSensor Type\t%s\n", "IR");
		break;
	case 0x0002:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSensor Type\t%s\n", "FFT");
		break;
	case 0x0003:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSensor Type\t%s\n", "CMOS");
		break;
	case 0x0004:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSensor Type\t%s\n", "HSVIS");
		break;
	case 0x0005:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tSensor Type\t%s\n", "HSIR");
		break;
	}

	//Camera System
	switch (upperBitsCAMTYPE)
	{
	case 0x0000:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCamera System\t%s\n", "3001");
		break;
	case 0x0001:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCamera System\t%s\n", "3010");
		break;
	case 0x0002:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\tCamera System\t%s\n", "3030");
		break;
	}

	/*=======================================================================*/

	return status;
}

es_status_codes dumpDmaRegisters(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		number_of_registers = 18,
		bufferLength = 40
	};

	char register_names[number_of_registers][bufferLength] = {
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
		"RCOMPDSIZW",
		"DLWSTAT",
		"DLTRSSTAT",
		"DMISCCONT"
	}; //Look-Up-Table for the DMA Registers
	uint32_t data = 0;
	//allocate string buffer
	*stringPtr = (char*)calloc(number_of_registers * bufferLength, sizeof(char));
	int len = 0;
	size_t bufferSize = number_of_registers * bufferLength;
	es_status_codes status = es_no_error;
	for (uint16_t i = 0; i < number_of_registers; i++)
	{
		es_status_codes status = readRegisterDma_32(drvno, &data, i * 4);
		if (status != es_no_error)
		{
			//write error to buffer
			len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register 0x%x %s", i * 4, register_names[i]);
			return status;
		}
		//write register name and value to buffer
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "0x%x  \t%s\t0x%x\n", i*4, register_names[i], data);
	}
	return status;
}

es_status_codes dumpTlpRegisters(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		bufferLength = 500
	};
	uint32_t data = 0;
	int len = 0;
	size_t bufferSize = bufferLength;
	//allocate string buffer
	*stringPtr = (char*)calloc(bufferLength, sizeof(char));
	es_status_codes status = readConfig_32(drvno, &data, PCIeAddr_PCIExpressDeviceCapabilities);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	data &= PciExpressDeviceCapabilities_MaxPayloadSizeSupported_bits;
	// See section 7.8.3, table 7 - 13 of the PCI Express Base Specification.
	// https://astralvx.com/storage/2020/11/PCI_Express_Base_4.0_Rev0.3_February19-2014.pdf
	uint32_t maxSizeEncoding[8] = { 128, 256, 512, 1024, 2048, 4096, 0, 0};
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Max_Payload_Size Supported:\t0x%x (%u bytes)\n", data, maxSizeEncoding[data]);
	status = readConfig_32(drvno, &data, PCIeAddr_DeviceControl);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	uint32_t maxPayloadSize = (data & deviceControl_maxPayloadSize_bits) >> deviceControl_maxPayloadSize_bitindex;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Max_Payload_Size:\t"DLLTAB"0x%x (%u bytes)\n", maxPayloadSize, maxSizeEncoding[maxPayloadSize]);
	uint32_t maxReadRequestSize = (data & deviceControl_maxReadRequestSize_bits) >> deviceControl_maxReadRequestSize_bitindex;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Max_Read_Request_Size:\t0x%x (%u bytes)\n", maxReadRequestSize, maxSizeEncoding[maxReadRequestSize]);
	uint32_t pixel = 0;
	status = readRegisterS0_32(drvno, &pixel, S0Addr_PIXREGlow);
	pixel &= 0xFFFF;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Number of pixels:\t"DLLTAB"%u\n", pixel);
	status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPS);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "TLPS in DMAReg is:\t%u (%u bytes)\n", data, data*4);
	uint32_t numberOfTlps = 0;
	if (data)
		numberOfTlps = (pixel - 1) / (data * 2) + 1;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "number of TLPs per scan should be:\t(number of pixels - 1) / (TLPS in DMAReg * 2) + 1 \n\t= %u / %u + 1\n\t=%u\n", pixel-1, data*2, numberOfTlps);
	status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPC);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "number of TLPs per scan is:\t"DLLTAB"%u", data);
	return status;
}

/**
 * \brief Reads registers 0 to 12 of TDC-GPX chip. Time delay counter option.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param stringPtr
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes _AboutGPX(uint32_t drvno, char** stringPtr)
{
	es_status_codes status = es_no_error;
	enum N
	{
		number_of_registers = 0x30,
		bufferLength = 100,
	};
	uint32_t regData;
	char LUTS0Reg[number_of_registers][bufferLength] = {
		"Reg0",
		"Reg1",
		"Reg2",
		"Reg3",
		"Reg4",
		"Reg5",
		"Reg6",
		"Reg7",
		"Reg8",
		"Reg9",
		"Reg10",
		"Reg11",
		"Reg12",
		"Reg13",
		"Reg14",
		"Reg15"
	};
	*stringPtr = (char*)calloc(number_of_registers * bufferLength, sizeof(char));
	int len = 0;
	size_t bufferSize = number_of_registers * bufferLength;
	for (uint8_t i = 0; i < 8; i++)
	{
		status = ReadGPXCtrl(drvno, i, &regData);
		if (status != es_no_error) return status;
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%s \t: 0x%x\n", LUTS0Reg[i], regData);
	}
	for (uint8_t i = 11; i < 13; i++)
	{
		status = ReadGPXCtrl(drvno, i, &regData);
		if (status != es_no_error) return status;
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%s \t: 0x%x\n", LUTS0Reg[i], regData);
	}
	//ErrorMsg("_AboutGPX");
	return status;
	/*
	//doesn't return to this part A.M. 24.08.2022
	bool abbr = false, space = false;
	int i = 0;
	while (!abbr)
	{
#ifdef WIN32
		status = WaitTrigger(1, false, &space, &abbr);
		if (status != es_no_error) return status;
#else
		// suppress unused warning
		(void)space;
#endif
		i = 0;
		len = sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "read- regs   \n");
		i += 1;
		status = ReadGPXCtrl(drvno, 8, &regData); //lege addr 8 an bus !!!!
		if (status != es_no_error) return status;
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d \t: 0x%x\n", i, regData);
		i += 1;
		status = ReadGPXCtrl(drvno, 9, &regData); //lege addr 9 an bus !!!!
		if (status != es_no_error) return status;
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d \t: 0x%x\n", i, regData);
	}
	status = ReadGPXCtrl(drvno, 11, &regData);
	if (status != es_no_error) return status;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%s \t: 0x%x\n", " stop hits", regData);
	status = ReadGPXCtrl(drvno, 12, &regData);
	if (status != es_no_error) return status;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%s \t: 0x%x\n", " flags", regData);
	return ReadGPXCtrl(drvno, 8, &regData); //read access follows set addr 8 to bus !!!!
	*/
}

es_status_codes dumpMeasurementSettings(char** stringPtr)
{
	enum N
	{
		bufferLength = 2000,
	};
	int len = 0;
	size_t bufferSize = bufferLength;
	//allocate string buffer
	*stringPtr = (char*)calloc(bufferLength, sizeof(char));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len,
		"board_sel\t"DLLTAB DLLTAB"%u\n"
		"nos\t" DLLTAB DLLTAB"%u\n"
		"nob\t"DLLTAB DLLTAB"%u\n"
		"cont_pause_in_microseconds\t" DLLTAB DLLTAB"%u\n",
		settings_struct.board_sel,
		settings_struct.nos,
		settings_struct.nob,
		settings_struct.cont_pause_in_microseconds);
	return es_no_error;
}

es_status_codes dumpCameraSettings(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		bufferLength = 2000,
	};
	int len = 0;
	size_t bufferSize = bufferLength;
	//allocate string buffer
	*stringPtr = (char*)calloc(bufferLength, sizeof(char));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len,
		"use_software_polling\t"DLLTAB DLLTAB"%u\n"
		"sti_mode\t"DLLTAB DLLTAB"%u\n"
		"bti_mode\t"DLLTAB"%u\n"
		"stime in microseconds\t%u\n"
		"btime in microseconds\t%u\n"
		"sdat in 10 ns\t"DLLTAB"%u\n"
		"bdat in 10 ns\t"DLLTAB"%u\n"
		"sslope\t"DLLTAB DLLTAB"%u\n"
		"bslope\t"DLLTAB DLLTAB"%u\n"
		"xckdelay_in_10ns\t"DLLTAB"%u\n"
		"shutterExpTimeIn10ns\t%u\n"
		"trigger mode cc\t"DLLTAB"%u\n"
		"sensor type\t"DLLTAB"%u\n"
		"camera system\t"DLLTAB"%u\n"
		"camcnt\t"DLLTAB DLLTAB"%u\n"
		"pixel\t"DLLTAB DLLTAB"%u\n"
		"mshut\t"DLLTAB DLLTAB"%u\n"
		"led off\t"DLLTAB DLLTAB"%u\n"
		"sensor_gain\t"DLLTAB"%u\n"
		"adc_gain\t"DLLTAB"%u\n"
		"temp level\t"DLLTAB"%u\n"
		"shortrs\t"DLLTAB"%u\n"
		"gpx offset\t"DLLTAB"%u\n"
		"fft_lines\t"DLLTAB DLLTAB"%u\n"
		"vfreq\t"DLLTAB DLLTAB"%u\n"
		"ffmode\t"DLLTAB DLLTAB"%u\n"
		"lines_binning\t"DLLTAB"%u\n"
		"number of regions\t%u\n"
		"s1s2_read_delay_in_10ns\t"DLLTAB"%u\n",
		settings_struct.camera_settings[drvno].use_software_polling,
		settings_struct.camera_settings[drvno].sti_mode,
		settings_struct.camera_settings[drvno].bti_mode,
		settings_struct.camera_settings[drvno].stime_in_microsec,
		settings_struct.camera_settings[drvno].btime_in_microsec,
		settings_struct.camera_settings[drvno].sdat_in_10ns,
		settings_struct.camera_settings[drvno].bdat_in_10ns,
		settings_struct.camera_settings[drvno].sslope,
		settings_struct.camera_settings[drvno].bslope,
		settings_struct.camera_settings[drvno].xckdelay_in_10ns,
		settings_struct.camera_settings[drvno].sec_in_10ns,
		settings_struct.camera_settings[drvno].trigger_mode_cc,
		settings_struct.camera_settings[drvno].sensor_type,
		settings_struct.camera_settings[drvno].camera_system,
		settings_struct.camera_settings[drvno].camcnt,
		settings_struct.camera_settings[drvno].pixel,
		settings_struct.camera_settings[drvno].mshut,
		settings_struct.camera_settings[drvno].led_off,
		settings_struct.camera_settings[drvno].sensor_gain,
		settings_struct.camera_settings[drvno].adc_gain,
		settings_struct.camera_settings[drvno].temp_level,
		settings_struct.camera_settings[drvno].shortrs,
		settings_struct.camera_settings[drvno].gpx_offset,
		settings_struct.camera_settings[drvno].fft_lines,
		settings_struct.camera_settings[drvno].vfreq,
		settings_struct.camera_settings[drvno].fft_mode,
		settings_struct.camera_settings[drvno].lines_binning,
		settings_struct.camera_settings[drvno].number_of_regions,
		settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "region size\t"DLLTAB);
	for (int i = 0; i < 8; i++)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%u ", settings_struct.camera_settings[drvno].region_size[i]);
	for (int camera = 0; camera < MAXCAMCNT; camera++)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\ndac output board %i, camera %i\t", drvno, camera);
		for (int i = 0; i < 8; i++)
			len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%u ", settings_struct.camera_settings[drvno].dac_output[camera][i]);
	}
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len,
		"\ntor mode\t"DLLTAB DLLTAB"%u\n"
		"adc mode\t"DLLTAB"%u\n"
		"adc custom pattern\t%u\n"
		"bec_in_10ns\t"DLLTAB"%u\n"
		"is_hs_ir\t"DLLTAB DLLTAB"%u\n"
		"ioctrl_impact_start_pixel\t%u\n",
		settings_struct.camera_settings[drvno].tor,
		settings_struct.camera_settings[drvno].adc_mode,
		settings_struct.camera_settings[drvno].adc_custom_pattern,
		settings_struct.camera_settings[drvno].bec_in_10ns,
		settings_struct.camera_settings[drvno].is_hs_ir,
		settings_struct.camera_settings[drvno].ioctrl_impact_start_pixel);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ioctrl_output_width_in_5ns\t");
	for (int i = 0; i < 7; i++)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%u ", settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[i]);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nIOCtrl_output_delay_in_5ns\t");
	for (int i = 0; i < 7; i++)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%u ", settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[i]);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len,
		"\nIOCtrl_T0_period_in_10ns\t%u\n"
		"dma_buffer_size_in_scans\t%u\n"
		"tocnt\t%u\n"
		"ticnt\t%u\n"
		"sensor_reset_length_in_4_ns\t%u\n"
		"write to disc\t%u\n"
		"file path\t%s\n"
		"file split mode\t%u\n"
		"is cooled cam\t%u\n",
		settings_struct.camera_settings[drvno].ioctrl_T0_period_in_10ns,
		settings_struct.camera_settings[drvno].dma_buffer_size_in_scans,
		settings_struct.camera_settings[drvno].tocnt,
		settings_struct.camera_settings[drvno].ticnt,
		settings_struct.camera_settings[drvno].sensor_reset_length_in_4_ns,
		settings_struct.camera_settings[drvno].write_to_disc,
		settings_struct.camera_settings[drvno].file_path,
		settings_struct.camera_settings[drvno].file_split_mode,
		settings_struct.camera_settings[drvno].is_cooled_cam);
	return es_no_error;
}

/**
 * \brief
 * 
 * \param drvno
 * \param stringPtr
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes dumpPciRegisters(uint32_t drvno, char** stringPtr)
{
	uint32_t data = 0;
	int length = 0;
	es_status_codes status = es_no_error;
	enum N
	{
		number_of_registers = 28,
		bufferLength = 100,
	};
	size_t bufferSize = number_of_registers * bufferLength;
	// See table 2-2 in documentation of Spartan-6 FPGA Integrated Endpoint Block for register names
	// https://docs.xilinx.com/v/u/en-US/s6_pcie_ug654
	char register_names[number_of_registers][bufferLength] = {
	"Device ID, Vendor ID"DLLTAB DLLTAB,
	"Status, Command"DLLTAB DLLTAB DLLTAB,
	"Class Code, Revision ID"DLLTAB DLLTAB,
	"BIST, Header Type, Lat. Timer, Cache Line S."DLLTAB,
	"Base Address 0 mem 32 bit"DLLTAB DLLTAB,
	"Base Address 1 io"DLLTAB DLLTAB DLLTAB,
	"Base Address 2 mem 32 bit"DLLTAB DLLTAB,
	"Base Address 3"DLLTAB DLLTAB DLLTAB,
	"Base Address 4"DLLTAB DLLTAB DLLTAB,
	"Base Address 5"DLLTAB DLLTAB DLLTAB,
	"Cardbus CIS Pointer"DLLTAB DLLTAB,
	"Subsystem ID, Subsystem Vendor ID"DLLTAB,
	"Expansion ROM Base Address"DLLTAB,
	"Reserved, Cap. Pointer"DLLTAB DLLTAB,
	"Reserved"DLLTAB DLLTAB DLLTAB DLLTAB,
	"Max Lat., Min Gnt., Interrupt Pin, Intterupt Line",
	"PM Capability, NxtCap, PM Cap",
	"Data, BSE, PMCSR",
	"MSI Control, NxtCap, MSI Cap",
	"Message Address (Lower)",
	"Message Address (Upper)",
	"Reserved, Message Data",
	"PE Capability, NxtCap, PE Cap",
	"PCI Express Device Capabilities",
	"Device Status, Device Control",
	"PCI Express Link Capabilties",
	"Link Status, Link Control",
	"Reserved Legacy Configuration Space"
	};
	*stringPtr = (char*)calloc(number_of_registers * bufferSize, sizeof(char));
	for (uint16_t i = 0; i < number_of_registers; i++)
	{
		status = readConfig_32(drvno, &data, i * 4);
		if (status != es_no_error)
		{
			//write error to buffer
			length += sprintf_s(*stringPtr + length, bufferSize - (size_t)length, "\nerror while reading register %i %s", i*4, register_names[i]);
			return status;
		}
		length += sprintf_s(*stringPtr + length, bufferSize - (size_t)length, "0x%x  \t%s\t0x%x\n", i * 4, register_names[i], data);
	}
	return status;
}

/**
* \brief Return infos about the PCIe board.
*
* - win1 : version of driver
* - win2 : ID = 53xx
* - win3 : length of space0 BAR =0x3f
* - win4 : vendor ID = EBST
* - win5 : PCI board version (same as label on PCI board)
* \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
* \param stringPtr string with driver information is given back here
* \return es_status_codes
* 	- es_no_error
* 	- es_register_read_failed
*	- es_no_space0
*/
es_status_codes _AboutDrv(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		bufferLength = 500,
	};
	size_t bufferSize = bufferLength;
	*stringPtr = (char*)calloc(bufferLength, sizeof(char));
	int len = 0;
	uint32_t data = 0;
	// read ISA Id from S0Base+7
	es_status_codes status = readRegisterS0_32(drvno, &data, S0Addr_CTRLA); // Board ID =5053
	if (status != es_no_error) return status;
	data = data >> 16;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, " Board #%i    ID \t= 0x%x\n", drvno, data);
	data = 0x07FF;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Board #%i     length \t= 0x%x\n", drvno, data);
	uint8_t udata1 = 0,
		udata2 = 0,
		udata3 = 0,
		udata4 = 0;
	if (data >= 0x1F)
	{//if WE -> has space 0x20
		status = readRegisterS0_8(drvno, &udata1, 0x1C);
		if (status != es_no_error) return status;
		status = readRegisterS0_8(drvno, &udata2, 0x1D);
		if (status != es_no_error) return status;
		status = readRegisterS0_8(drvno, &udata3, 0x1E);
		if (status != es_no_error) return status;
		status = readRegisterS0_8(drvno, &udata4, 0x1F);
		if (status != es_no_error) return status;
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Board #%i  ven ID \t= %c%c%c%c\n", drvno, udata1, udata2, udata3, udata4);
	}
	if (data >= 0x3F)
	{//if 9056 -> has space 0x40
		status = readRegisterS0_32(drvno, &data, S0Addr_PCI);
		if (status != es_no_error) return status;
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Board #%i   board version \t= 0x%x\n", drvno, data);
	}
	return es_no_error;
}

/**
 * @brief reset Delay Stage Counter
 *
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param DSCNumber 1: DSC 1; 2: DSC 2
 * @return es_status_codes
 */
es_status_codes ResetDSC( uint32_t drvno, uint8_t DSCNumber )
{
	es_status_codes status;
	ES_LOG( "Reset DSC %u\n", DSCNumber );
	uint32_t data = 0;
	switch (DSCNumber)
	{
	case 1: data = 0x1; break;
	case 2: data = 0x100; break;
	}
	//for reset you have to set a 1 to the reg and then a zero to allow a new start again
	status = writeBitsS0_32( drvno, data, data, S0Addr_DSCCtrl );
	if (status != es_no_error) return status;
	return writeBitsS0_32( drvno, 0, data, S0Addr_DSCCtrl );
}

/**
 * @brief set direction of Delay Stage Counter
 *
 * @param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param DSCNumber 1: DSC 1; 2: DSC 2
 * @param dir true: up; false: down
 * @return es_status_codes
 */
es_status_codes SetDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir )
{
	ES_LOG( "set DSC %u in direction %u\n", DSCNumber, dir );
	uint32_t data = 0;
	switch (DSCNumber)
	{
	case 1: data = 0x2; break;
	case 2: data = 0x200; break;
	}

	if (dir)
		return writeBitsS0_32( drvno, data, data, S0Addr_DSCCtrl );
	else
		return writeBitsS0_32( drvno, 0, data, S0Addr_DSCCtrl );

}

/**
 * \brief return all values of Delay Stage Counter
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param DSCNumber 1: DSC 1; 2: DSC 2
 * \param ADSC current DSC
 * \param LDSC last DSC
 * \return es_status_codes
 */
es_status_codes GetDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC )
{
	es_status_codes status;
	ES_LOG( "get DSC %u\n", DSCNumber );
	uint16_t addrADSC, addrLDSC;
	switch (DSCNumber)
	{
	default:
	case 1:
		addrADSC = S0Addr_A1DSC;
		addrLDSC = S0Addr_L1DSC;
		break;
	case 2:
		addrADSC = S0Addr_A2DSC;
		addrLDSC = S0Addr_L2DSC;
		break;
	}

	status = readRegisterS0_32( drvno, ADSC, addrADSC );
	if (status != es_no_error) return status;
	return readRegisterS0_32( drvno, LDSC, addrLDSC );
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
es_status_codes IOCtrl_setImpactStartPixel(uint32_t drvno, uint16_t startPixel)
{
	ES_LOG("Set IOCtrl impact start pixel: %u\n", startPixel);
	return SendFLCAM(drvno, maddr_ioctrl, ioctrl_impact_start_pixel, startPixel);
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
es_status_codes IOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
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
	es_status_codes status = SendFLCAM(drvno, maddr_ioctrl, addrWidth, width_in_5ns);
	if (status != es_no_error) return status;
	return SendFLCAM(drvno, maddr_ioctrl, addrDelay, delay_in_5ns);
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
es_status_codes IOCtrl_setAllOutputs(uint32_t drvno, uint32_t* width_in_5ns, uint32_t* delay_in_5ns)
{
	es_status_codes status = es_no_error;
	for (uint8_t i = 0; i <= 6; i++)
	{
		status = IOCtrl_setOutput(drvno, i + 1, (uint16_t)width_in_5ns[i], (uint16_t)delay_in_5ns[i]);
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
es_status_codes IOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns)
{
	ES_LOG("Set IOCtrl T0 period %u\n", period_in_10ns);
	uint16_t period_in_10ns_L = (uint16_t) period_in_10ns;
	uint16_t period_in_10ns_H = (uint16_t) (period_in_10ns >> 16);
	es_status_codes status = SendFLCAM(drvno, maddr_ioctrl, ioctrl_t0h, period_in_10ns_H);
	if (status != es_no_error) return status;
	return SendFLCAM(drvno, maddr_ioctrl, ioctrl_t0l, period_in_10ns_L);
}

/**
 * \brief This function copies valid data from DMA buffer to user buffer.
 * 
 * This function tracks the DMA buffer and every time there is new data available, it is copied to the user buffer.
 * The memory of the DMA buffer which was copied is then set to 0. Create a new thread for this function. This function 
 * should run parallel to the measurement. This function is only used when USE_SOFTWARE_POLLING is true.
 * 
 * \param drvno_p Pointer to PCIe board identifier.
 */
void PollDmaBufferToUserBuffer(uint32_t* drvno_p)
{
	uint32_t drvno = *drvno_p;
	free(drvno_p);
	ES_LOG("Poll DMA buffer to user buffer started. drvno: %u\n", drvno);
	// Get the pointer to DMA buffer.
	uint16_t* dmaBuffer = getVirtualDmaAddress(drvno);
	ES_TRACE("DMA buffer address: %p\n", (void*)dmaBuffer);
	// Set dmaBufferReadPos pointer to base address of DMA buffer. dmaBufferReadPos indicates the current read position in the DMA buffer.
	uint16_t* dmaBufferReadPos = dmaBuffer;
	// Calculate pointer to the end of the DMA buffer.
	uint16_t* dmaBufferEnd = dmaBufferReadPos + getDmaBufferSizeInBytes(drvno) / sizeof(uint16_t);
	ES_TRACE("DMA buffer end: %p\n", (void*)dmaBufferEnd);
	// Calculate the size of the complete measurement in bytes.
	uint32_t dataToCopyInBytes = aPIXEL[drvno] * aCAMCNT[drvno] * (*Nospb) * (*Nob) * sizeof(uint16_t);
	ES_TRACE("Data to copy in bytes: %u\n", dataToCopyInBytes);
	// Calculate the size of one scan.
	uint32_t sizeOfOneScanInBytes = aPIXEL[drvno] * sizeof(uint16_t);
	ES_TRACE("Size of one scan in bytes: %u\n", sizeOfOneScanInBytes);
	// Set userBufferWritePos_polling to the base address of userBuffer_polling. userBufferWritePos_polling indicates the current write position in the user buffer.
	uint16_t* userBufferWritePos_polling = userBuffer[drvno];
	ES_TRACE("Address of user buffer: %p\n", (void*)userBuffer[drvno]);
	bool allDataCopied = false;
	scanCounterTotal[drvno] = 0;
	uint32_t scanCounterHardwareMirror = 1;
	uint32_t blockCounterHardwareMirror = 1;
	uint32_t scanCounterHardware;
	uint32_t blockCounterHardware;
	bool measurementStopped = false;
	struct timeb timebuffer_measurementStopped;
	struct timeb timebuffer_now;
	while (!allDataCopied)
	{
		//ES_TRACE("dmaBufferReadPosNextScan: %p ", dmaBufferReadPosNextScan);
		// scan counter pixel are 4 and 5 and since P202_21 at the last pixel
		scanCounterHardware = (uint32_t)(dmaBufferReadPos[pixel_scan_index_high] << 16) | *(dmaBufferReadPos + aPIXEL[drvno] - 1);
		// block counter pixel are 2 and 3
		uint16_t block_index_high = dmaBufferReadPos[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bits_block_index;
		blockCounterHardware = (uint32_t)(block_index_high << 16) | dmaBufferReadPos[pixel_block_index_low];
		//ES_TRACE("scan: %u, scanmirror: %u, block: %u, blockmirror: %u\n", scanCounterHardware, scanCounterHardwareMirror, blockCounterHardware, blockCounterHardwareMirror);
		// Check if scan and block counter in DMA buffer are equal to their mirrors
		if (scanCounterHardwareMirror == scanCounterHardware && blockCounterHardwareMirror == blockCounterHardware)
		{
			ES_TRACE("DMA buffer read position: %p\n", (void*)dmaBufferReadPos);
			ES_TRACE("User buffer write position: %p\n", (void*)userBufferWritePos_polling);
			// Copy the data.
			memcpy(userBufferWritePos_polling, dmaBufferReadPos, sizeOfOneScanInBytes);
			// Set the memory of the copied data to 0 in the DMA buffer.
			memset(dmaBufferReadPos, 0, sizeOfOneScanInBytes);
			// Advance the pointers and counters.
			dmaBufferReadPos += sizeOfOneScanInBytes / sizeof(uint16_t);
			userBufferWritePos_polling += sizeOfOneScanInBytes / sizeof(uint16_t);
			dataToCopyInBytes -= sizeOfOneScanInBytes;
			scanCounterTotal[drvno]++;
			// get scan and block number for the next scan
			int64_t scan, block;
			GetScanNumber(drvno, 1, &scan, &block);
			scanCounterHardwareMirror = (uint32_t)(scan + 1);
			blockCounterHardwareMirror = (uint32_t)(block + 1);
			ES_TRACE("Scan: %li\n", scanCounterTotal[drvno]);
			// check if dmaBufferReadPos exceeds dmaBuffer
			if (dmaBufferReadPos >= dmaBufferEnd || dmaBufferReadPos < dmaBuffer)
			{
				// reset the read pointer to the base address of the dma buffer
				dmaBufferReadPos = dmaBuffer;
				ES_TRACE("Reset dmaBufferReadPos to: %p\n", (void*)dmaBuffer);
			}
			ES_TRACE("Data to copy: %u\n", dataToCopyInBytes);
			if (dataToCopyInBytes == 0) allDataCopied = true;
		}
		// Escape while loop after 100ms when Measurement stopped.
		if (!isRunning && !measurementStopped)
		{
			ES_LOG("Measurement aborted. Starting countdown for PollDmaBufferToUserBuffer\n");
			measurementStopped = true;
			ftime(&timebuffer_measurementStopped);
		}
		if (measurementStopped)
		{
			ftime(&timebuffer_now);
			int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_now.time - timebuffer_measurementStopped.time) + (timebuffer_now.millitm - timebuffer_measurementStopped.millitm));
			if (diff_in_ms > 100)
			{
				ES_LOG("Aborted. Exit PollDmaBufferToUserBuffer\n");
				break;
			}
		}
		// setting allInterruptsDone true gives the measurement loop the signal, that all data has been copied and the loop is allowed to continue
		allInterruptsDone[drvno] = true;
	}
	return;
}

/**
 * \brief Gives scan and block number of the last scan written to userBuffer.
 *
 * When settings parameter USE_SOFTWARE_POLLING is true this function converts scanCounterTotal to scan and block.
 * This is necessary, because scanCounterTotal is just counting each scan not regarding camcnt and blocks.
 * When USE_SOFTWARE_POLLING is false the scan and block number of the last interrupt is given.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample Scan number of the last scan in userBuffer. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * \param block Block number of the last scan in userBuffer. -1 when no scans has been written yet, otherwise 0...(nob-1)
 */
void GetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block)
{
	GetScanNumber(drvno, 0, sample, block);
	return;
}

/**
 * \copydoc GetCurrentScanNumber
 * \param offset from current scan number
 */
void GetScanNumber(uint32_t drvno, int64_t offset, int64_t* sample, int64_t* block)
{
	int64_t scanCount = 0;
	uint32_t dmasPerInterrupt = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS;
	if (settings_struct.camera_settings[drvno].use_software_polling)
		scanCount = scanCounterTotal[drvno];
	else
		scanCount = getCurrentInterruptCounter(drvno) * dmasPerInterrupt;
	ES_TRACE("scan counter %lu, Nospb %u, camcnt %u\n", scanCount + offset, *Nospb, aCAMCNT[drvno]);
	int64_t samples_done_all_cams = scanCount - 1 + offset;
	int64_t samples_done_per_cam = samples_done_all_cams / aCAMCNT[drvno];
	*block = samples_done_per_cam / *Nospb;
	int64_t count_of_scans_of_completed_blocks = *block * *Nospb;
	*sample = samples_done_per_cam - count_of_scans_of_completed_blocks;
	ES_TRACE("block %li, scan %li, samples_done_all_cams %li, samples_done_per_cam %li, count_of_scans_of_completed_blocks %li\n", *block, *sample, samples_done_all_cams, samples_done_per_cam, count_of_scans_of_completed_blocks);
	return;
}

/**
 * \brief Set the trigger input divider
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param divider
 *		- =0: disable this function (every trigger is used)
 *		- >0: omit n trigger
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes SetTicnt(uint32_t drvno, uint8_t divider)
{
	ES_LOG("Set TICNT to %u\n", divider);
	// If divider is not 0, set the enable bit to 1
	if (divider)
		divider |= TOR_bit_TICNT_EN;
	return writeRegisterS0_8(drvno, divider, S0Addr_TOR_TICNT);
}

/**
 * \brief Set the trigger output divider
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param divider 7 bit value
 *		- =0: disable this function (every trigger is used)
 *		- >0: use every n'th trigger
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes SetTocnt(uint32_t drvno, uint8_t divider)
{
	ES_LOG("Set TOCNT to %u\n", divider);
	// If divider is not 0, set the enable bit to 1
	if (divider)
		divider |= TOR_bit_TOCNT_EN;
	return writeRegisterS0_8(drvno, divider, S0Addr_TOR_TOCNT);
}

/**
 * \brief This function inserts data to user buffer for developing purpose.
 *  
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 */
void FillUserBufferWithDummyData(uint32_t drvno)
{
	//memset(userBuffer[drvno], 0xAAAA, aPIXEL[drvno] * (*Nospb) * (*Nob) * aCAMCNT[drvno] * sizeof(uint16_t));
	for (uint32_t scan = 0; scan < (*Nospb) * (*Nob) * aCAMCNT[drvno]; scan++)
	{
		int add = (scan % 2) * 1000;
		for (uint32_t pixel = 0; pixel < aPIXEL[drvno]; pixel++)
			userBuffer[drvno][scan * aPIXEL[drvno] + pixel] = pixel * 10 + add;
	}
	return;
}

/**
 * \brief Read TDC flag in PCIEFLAGS register.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param isTdc	TDC flag is written to this bool*. TRUE: TDC board detected, FALSE: no TDC board detected
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes GetIsTdc(uint32_t drvno, bool* isTdc)
{
	ES_LOG("Get is TDC, drvno %u\n", drvno);
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	// Check if TDC bit is set
	if (PCIEFLAGS_bit_IS_TDC & data)
		*isTdc = true;
	else
		*isTdc = false;
	return status;
}

/**
 * \brief Read DSC flag in PCIEFLAGS register.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param isDsc	DSC flag is written to this bool*. TRUE: DSC board detected, FALSE: no DSC board detected
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes GetIsDsc(uint32_t drvno, bool* isDsc)
{
	ES_LOG("Get is DSC, drvno %u\n", drvno);
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	// Check if DSC bit is set
	if (PCIEFLAGS_bit_IS_DSC & data)
		*isDsc = true;
	else
		*isDsc = false;
	return status;
}

void GetVerifiedDataDialog(struct verify_data_parameter* vd, char** resultString)
{
#ifndef MINIMAL_BUILD
	VerifyData(vd);
#endif
	enum N
	{
		bufferLength = 1024
	};
	*resultString = (char*)calloc(bufferLength, sizeof(char));
	uint32_t len = 0;
	len = sprintf_s(*resultString + len, bufferLength - (size_t)len, "Checked consistency of file\t%s\n\n", vd->filename_full);
	if(vd->error_cnt) len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data inconsistent, check counters below\n\n");
	else len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data found as expected\n\n");
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data found in file header:\n");
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "drvno:\t%u\n",vd->fh.drvno);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "pixel:\t%u\n",vd->fh.pixel);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "nos:\t%u\n",vd->fh.nos);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "nob:\t%u\n",vd->fh.nob);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "camcnt:\t%u\n",vd->fh.camcnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "measurement cnt:\t%llu\n",vd->fh.measurement_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "timestamp:\t%s\n",vd->fh.timestamp);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "filename_full:\t%s\n",vd->fh.filename_full);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "split mode:\t%u\n\n",vd->fh.split_mode);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data found:\n");
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "samples found:\t%u\n",vd->sample_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "blocks found:\t%u\n",vd->block_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "measurements found:\t%llu\n",vd->measurement_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "error counter:\t%u\n",vd->error_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last sample in data:\t%u\n",vd->last_sample);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last block in data:\t%u\n",vd->last_block);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last sample before error:\t%u\n",vd->last_sample_before_error);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last block before error:\t%u\n",vd->last_block_before_error);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last measurement before error:\t%llu\n",vd->last_measurement_before_error);
	return;
}

/**
 * \brief Control looping the measurement.
 * 
 * \param on 1: measurement runs in a loop, 0: measurement only runs once.
 */
void SetContinuousMeasurement(bool on)
{
	continiousMeasurementFlag = on;
	return;
}

/**
 * \brief This function returns the bit overTemp of a specific scan.
 * 
 * The information over temperature is given in the special pixel camera status (pixel_camera_status) in bit pixel_camera_status_bit_over_temp.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param overTemp Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp)
{
	uint16_t data[pixel_camera_status + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_camera_status + 1, data);
	if (data[pixel_camera_status] & pixel_camera_status_bit_over_temp)
		*overTemp = true;
	else
		*overTemp = false;
	return status;
}

/**
 * \brief This function returns the bit tempGood of a specific scan.
 *
 * The information temperature good is given in the special pixel camera status (pixel_camera_status) in bit pixel_camera_status_bit_temp_good. This bit is used only in cooled cameras.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param tempGood Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood)
{
	uint16_t data[pixel_camera_status + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_camera_status + 1, data);
	if (data[pixel_camera_status] & pixel_camera_status_bit_temp_good)
		*tempGood = true;
	else
		*tempGood = false;
	return status;
}

/**
 * \brief This function returns the block index of a specific scan.
 *
 * The information block index is given in the special pixels pixel_block_index_low and pixel_block_index_high_s1_s2.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param blockIndex Pointer to a uint32_t, where the information block index will be written. Block index is a 30 bit counter, so the highest two bits are not used.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex)
{
	uint16_t data[pixel_block_index_low + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_block_index_low + 1, data);
	uint16_t blockIndexHigh = data[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bits_block_index;
	*blockIndex = (uint32_t)blockIndexHigh << 16 | (uint32_t)data[pixel_block_index_low];
	return status;
}

/**
 * \brief This function returns the scan index of a specific scan.
 *
 * The information block index is given in the special pixels pixel_scan_index_low and pixel_scan_index_high.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param scanIndex Pointer to a uint32_t, where the information scan index will be written. Scan index is a 32 bit counter.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex)
{
	uint16_t data[pixel_scan_index_low + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_scan_index_low + 1, data);
	*scanIndex = (uint32_t)data[pixel_scan_index_high] << 16 | (uint32_t)data[pixel_scan_index_low];
	return status;
}

/**
 * \brief This function returns the bit S1 state of a specific scan.
 *
 * The information S1 is given in the special pixel pixel_block_index_high_s1_s2 in bit pixel_block_index_high_s1_s2_bit_s1.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param state Pointer to a bool, where the information S1 state will be written. true - S1 is high, false - S1 is low
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	uint16_t data[pixel_block_index_high_s1_s2 + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_block_index_high_s1_s2 + 1, data);
	if (data[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bit_s1)
		*state = true;
	else
		*state = false;
	return status;
}

/**
 * \brief This function returns the bit S2 state of a specific scan.
 *
 * The information S2 is given in the special pixel pixel_block_index_high_s1_s2 in bit pixel_block_index_high_s1_s2_bit_s2.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param state Pointer to a bool, where the information S2 state will be written. true - S2 is high, false - S2 is low
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	uint16_t data[pixel_block_index_high_s1_s2 + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_block_index_high_s1_s2 + 1, data);
	if (data[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bit_s2)
		*state = true;
	else
		*state = false;
	return status;
}

/**
 * \brief This function returns the impact signal 1 of a specific scan.
 *
 * The information impact signal 1 is given in the special pixels pixel_impact_signal_1_low and pixel_impact_signal_1_high. Impact signal 1 is either TDC 1 or DSC 1, depending on the PCIe daughter board.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param impactSignal Pointer to a uint32_t, where the information impact signal will be written.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	uint16_t data[pixel_impact_signal_1_low + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_impact_signal_1_low + 1, data);
	*impactSignal = (uint32_t)data[pixel_impact_signal_1_high] << 16 | (uint32_t)data[pixel_impact_signal_1_low];
	return status;
}

/**
 * \brief This function returns the impact signal 2 of a specific scan.
 *
 * The information impact signal 2 is given in the special pixels pixel_impact_signal_2_low and pixel_impact_signal_2_high. Impact signal 2 is either TDC 2 or DSC 2, depending on the PCIe daughter board.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param impactSignal Pointer to a uint32_t, where the information impact signal will be written.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	uint16_t data[pixel_impact_signal_2_low + 1];
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, pixel_impact_signal_2_low + 1, data);
	*impactSignal = (uint32_t)data[pixel_impact_signal_2_high] << 16 | (uint32_t)data[pixel_impact_signal_2_low];
	return status;
}

/**
 * \brief This function returns the all special pixel information of a specific scan.
 *
 * The information impact signal 2 is given in the special pixels pixel_impact_signal_2_low and pixel_impact_signal_2_high. Impact signal 2 is either TDC 2 or DSC 2, depending on the PCIe daughter board.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param sp struct special_pixels Pointer to struct special_pixel, where all special pixel information will be written.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	if (aPIXEL[drvno] <= 63) return es_invalid_pixel_count;
	uint16_t* data = (uint16_t*)malloc(aPIXEL[drvno] * sizeof(uint16_t));
	if (!data) return es_allocating_memory_failed;
	es_status_codes status = ReturnFrame(drvno, sample, block, camera_pos, aPIXEL[drvno], data);
	if (status != es_no_error) return status;
	//overTemp
	if (data[pixel_camera_status] & pixel_camera_status_bit_over_temp)
		sp->overTemp = 1;
	else
		sp->overTemp = 0;
	//tempGood
	if (data[pixel_camera_status] & pixel_camera_status_bit_temp_good)
		sp->tempGood = 1;
	else
		sp->tempGood = 0;
	//blockIndex
	uint16_t blockIndexHigh = data[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bits_block_index;
	sp->blockIndex = (uint32_t)blockIndexHigh << 16 | (uint32_t)data[pixel_block_index_low];
	//scanIndex
	sp->scanIndex = (uint32_t)data[pixel_scan_index_high] << 16 | (uint32_t)data[pixel_scan_index_low];
	//S1
	if (data[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bit_s1)
		sp->s1State = 1;
	else
		sp->s1State = 0;
	//S2
	if (data[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bit_s2)
		sp->s2State = 1;
	else
		sp->s2State = 0;
	//impactSignal1
	sp->impactSignal1 = (uint32_t)data[pixel_impact_signal_1_high] << 16 | (uint32_t)data[pixel_impact_signal_1_low];
	//impactSignal2
	sp->impactSignal2 = (uint32_t)data[pixel_impact_signal_2_high] << 16 | (uint32_t)data[pixel_impact_signal_2_low];
	//scanIndex2
	sp->scanIndex2 = (uint32_t)data[(aPIXEL[drvno] - 1) - pixel_scan_index2_high] << 16 | (uint32_t)data[(aPIXEL[drvno] - 1) - pixel_scan_index2_low];
	//cameraSystem3001
	if (data[pixel_camera_status] & pixel_camera_status_bit_3001)
		sp->cameraSystem3001 = 1;
	else
		sp->cameraSystem3001 = 0;
	//cameraSystem3010
	if (data[pixel_camera_status] & pixel_camera_status_bit_3010)
		sp->cameraSystem3010 = 1;
	else
		sp->cameraSystem3010 = 0;
	//cameraSystem3030
	if (data[pixel_camera_status] & pixel_camera_status_bit_3030)
		sp->cameraSystem3030 = 1;
	else
		sp->cameraSystem3030 = 0;
	//fpga ver
	sp->fpgaVerMinor = data[pixel_fpga_ver] >> pixel_fpga_ver_minor_bit;

	sp->fpgaVerMajor = data[pixel_fpga_ver] & pixel_fpga_ver_major_and_bit;
	free(data);
	return status;
}

/**
 * \brief Reads the ScanFrequency bit and checks if its high or low.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param scanFrequencyTooHigh True when scanFrequency bit is set
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes ReadScanFrequencyBit(uint32_t drvno, bool* scanFrequencyTooHigh) 
{
	return ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_scan_read, scanFrequencyTooHigh);
}

/**
 * \brief Resets the ScanFrequency bit.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes ResetScanFrequencyBit(uint32_t drvno) 
{
	return pulseBitS0_8(drvno, FFCTRL_bitindex_scan_reset, S0Addr_FFCTRL);
}

/**
 * \brief Reads the BlockFrequency bit and checks if its high or low.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param blockFrequencyTooHigh True when BlockFrequency bit is set.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes ReadBlockFrequencyBit(uint32_t drvno, bool* blockFrequencyTooHigh)
{
	return ReadBitS0_8(drvno, S0Addr_FF_FLAGS, FF_FLAGS_bitindex_block_read, blockFrequencyTooHigh);
}

/**
 * \brief Resets the BlockFrequency bit.
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes ResetBlockFrequencyBit(uint32_t drvno)
{
	return pulseBitS0_8(drvno, FFCTRL_bitindex_block_reset, S0Addr_FFCTRL);
}

es_status_codes GetOneBlockOfOneCamera(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t** address)
{
	es_status_codes status = es_no_error;
	// allocate memory for one block of one camera
	uint16_t* data = (uint16_t*) malloc(aPIXEL[drvno] * *Nospb * sizeof(uint16_t));
	// iterate through all samples of the requestet block
	for (uint32_t sample = 0; sample < *Nospb; sample++)
	{
		uint16_t* sample_address = NULL;
		// get the address of the current sample
		status = GetAddressOfPixel(drvno, 0, sample, block, camera, &sample_address);
		if (status != es_no_error) return status;
		// check if sample_address is not null
		if(sample_address)
			// copy one sample to the new memory
			memcpy(data + sample * aPIXEL[drvno], sample_address, aPIXEL[drvno] * sizeof(uint16_t));
	}
	// return the address of the new allocated data
	*address = data;
	return status;
}

void SetAllInterruptsDone(uint32_t drvno)
{
	// Where there are expected interrupts or software polling mode is on, set allInterruptsDone to false. The measurement loop then waits at the end of one measurement until allInterruptsDone is set to true by the last interrupt or by the software polling thread.
	if (numberOfInterrupts[drvno] > 0 || settings_struct.camera_settings[drvno].use_software_polling)
		allInterruptsDone[drvno] = false;
	else
		allInterruptsDone[drvno] = true;
	return;
}

/**
 * \brief Initializes region of interest.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param number_of_regions determines how many region of interests are initialized, choose 2 to 8
 * \param lines number of total lines in camera
 * \param keep kept regions are determined by bits of keep
 * \param region_size determines the size of each region. array of size number_of_regions.
 * 	When region_size[0]==0 the lines are equally distributed for all regions.
 * 	I don't know what happens when  region_size[0]!=0 and region_size[1]==0. Maybe don't do this.
 * 	The sum of all regions should equal lines.
 * \param vfreq VCLK frequency
 * \return es_status_codes
 * 		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetupROI(uint32_t drvno, uint16_t number_of_regions, uint32_t lines, uint8_t keep, uint8_t* region_size, uint8_t vfreq)
{
	bool keep_temp;
	es_status_codes status = es_no_error;
	// calculate how many lines are in each region when equally distributed
	uint32_t lines_per_region = lines / number_of_regions;
	// calculate the rest of lines when equally distributed
	uint32_t lines_in_last_region = lines - lines_per_region * (number_of_regions - 1);
	ES_LOG("Setup ROI: lines_per_region: %u , lines_in_last_region: %u\n", lines_per_region, lines_in_last_region);
	// go from region 1 to number_of_regions
	for (int i = 1; i <= number_of_regions; i++)
	{
		// check whether lines should be distributed equally or by custom region size
		keep_temp = keep & 0b1;//make the last bit bool, because this bit indicates the current range to keep or not
		if (*region_size == 0)
		{
			if (i == number_of_regions) status = SetupVPB(drvno, i, lines_in_last_region, keep_temp);
			else status = SetupVPB(drvno, i, lines_per_region, keep_temp);
		}
		else
		{
			status = SetupVPB(drvno, i, *(region_size + (i - 1)), keep_temp);
		}
		if (status != es_no_error) return status;
		keep >>= 1;//bitshift right to got the next keep for the next range
	}
	status = SetupVCLKReg(drvno, lines, vfreq);
	if (status != es_no_error) return status;
	status = SetPartialBinning(drvno, 0); //I don't know why there first is 0 written, I just copied it from Labview. - FH
	if (status != es_no_error) return status;
	status = SetPartialBinning(drvno, number_of_regions);
	if (status != es_no_error) return status;
	*useSWTrig = true;
	return SetSTI(drvno, sti_ASL);
}

/**
 * \brief For FFTs: Setup area mode.
 *
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param lines_binning Determines how many lines are binned (summed) when reading camera in area mode.
 * \param vfreq Frequency for vertical clock.
 * \return es_status_codes
 * 		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetupArea(uint32_t drvno, uint32_t lines_binning, uint8_t vfreq)
{
	ES_LOG("Setup Area\n");
	es_status_codes status = SetupVCLKReg(drvno, lines_binning, vfreq);
	if (status != es_no_error) return status;
	status = SetSTI(drvno, sti_ASL);
	if (status != es_no_error) return status;
	*useSWTrig = true; //software starts 1st scan
	return ResetPartialBinning(drvno);
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
es_status_codes SetCameraPosition(uint32_t drvno)
{
	ES_LOG("Set camera position of first camera in row to 1\n");
	// 0x8000 is a test value. Camera position is set to 0
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_camera_position, 0x8000);
}

/**
 * \brief This functions sets the register S1S2ReadDealy with the setting \ref camera_settings.s1s2_read_delay_in_10ns. 
 * 
 * \param drvno identifier of PCIe card, 0 ... MAXPCIECARDS, when there is only one PCIe board: always 0
 * \return es_status_codes
 */
es_status_codes SetS1S2ReadDelay(uint32_t drvno)
{
	ES_LOG("Set S1 & S2 read delay to %u\n", settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns);
	return writeRegisterS0_32(drvno, settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns, S0Addr_S1S2ReadDelay);
}
