#include "Board.h"
#include "enum.h"
#include "es_status_codes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../shared_src/UIAbstractionLayer.h"
#include <math.h>
#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _USRDLL
#define DLLTAB "\t"
#else
#define DLLTAB
#endif

/**
 * \brief Set global settings struct.
 * 
 * \param settings struct global_settings
 */
void SetGlobalSettings(struct global_settings settings)
{
	settings_struct = settings;
}

/**
 * \brief Initialize Measurement (using board select).
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
es_status_codes InitMeasurement()
{
	ES_LOG("\n*** Init Measurement ***\n");
	ES_LOG("struct global_settings: ");
	for (uint32_t i = 0; i < sizeof(settings_struct)/4; i++)
        ES_LOG("%u ", *(&settings_struct.unused + i));
	ES_LOG("\n");
	es_status_codes status = es_no_error;
	switch (settings_struct.board_sel)
	{
	case 1:
		status = _InitMeasurement(1);
		break;
	case 2:
		status = _InitMeasurement(2);
		break;
	case 3:
		status = _InitMeasurement(1);
		if (status != es_no_error) return status;
		status = _InitMeasurement(2);
		break;
	default:
		return es_parameter_out_of_range;
	}
	ES_LOG("*** Init Measurement done ***\n\n");
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
	ES_LOG("\nInit board %u\n", drvno);
	abortMeasurementFlag = false;
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	BOARD_SEL = settings_struct.board_sel;
	status = ClearAllUserRegs(drvno);
	if (status != es_no_error) return status;
	status = SetPixelCount(drvno, settings_struct.pixel);
	if (status != es_no_error) return status;
	status = SetCamCount(drvno, settings_struct.camcnt);
	if (status != es_no_error) return status;
	//set PDA and FFT
	status = SetSensorType(drvno, (uint8_t)settings_struct.sensor_type);
	if (status != es_no_error) return status;
	if (settings_struct.sensor_type == FFTsensor)
	{
		switch (settings_struct.FFTMode)
		{
		case full_binning:
			status = SetupFullBinning(drvno, settings_struct.FFTLines, (uint8_t)settings_struct.Vfreq);
			break;
#ifdef WIN32
		case partial_binning:
		{
			uint8_t regionSize[8];
			for (int i = 0; i < 8; i++) regionSize[i] = settings_struct.region_size[i];
			status = DLLSetupROI(drvno, (uint16_t)settings_struct.number_of_regions, settings_struct.FFTLines, (uint8_t)settings_struct.keep_first, regionSize, (uint8_t)settings_struct.Vfreq);
			break;
		}
		case area_mode:
			status = DLLSetupArea(drvno, settings_struct.lines_binning, (uint8_t)settings_struct.Vfreq);
			break;
#endif
		default:
			return es_parameter_out_of_range;
		}
	}
	else *useSWTrig = false;
	if (status != es_no_error) return status;
	//allocate Buffer
	status = SetMeasurementParameters(drvno, settings_struct.nos, settings_struct.nob);
	if (status != es_no_error) return status;
	status = CloseShutter(drvno); //set cooling  off
	if (status != es_no_error) return status;
	//set mshut
	if (settings_struct.mshut)
	{
		status = SetSEC(drvno, settings_struct.ShutterExpTimeIn10ns);
		if (status != es_no_error) return status;
		status = SetTORReg(drvno, TOR_SSHUT);
		if (status != es_no_error) return status;
	}
	else
	{
		status = SetSEC(drvno, 0);
		if (status != es_no_error) return status;
		status = SetTORReg(drvno, (uint8_t)settings_struct.TORmodus);
		if (status != es_no_error) return status;
	}
	//SSlope
	SetSSlope(drvno, settings_struct.sslope);
	if (status != es_no_error) return status;
	//BSlope
	status = SetBSlope(drvno, settings_struct.bslope);
	if (status != es_no_error) return status;
	//SetTimer
	status = SetSTI(drvno, (uint8_t)settings_struct.sti_mode);
	if (status != es_no_error) return status;
	status = SetBTI(drvno, (uint8_t)settings_struct.bti_mode);
	if (status != es_no_error) return status;
	status = SetSTimer(drvno, settings_struct.stime_in_microsec);
	if (status != es_no_error) return status;
	status = SetBTimer(drvno, settings_struct.btime_in_microsec);
	if (status != es_no_error) return status;
	if (settings_struct.enable_gpx) status = InitGPX(drvno, settings_struct.gpx_offset);
	if (status != es_no_error) return status;
	//Delay after Trigger
	status = SetSDAT(drvno, settings_struct.sdat_in_10ns);
	if (status != es_no_error) return status;
	status = SetBDAT(drvno, settings_struct.bdat_in_10ns);
	if (status != es_no_error) return status;
	//init Camera
	status = InitCameraGeneral(drvno, settings_struct.pixel, settings_struct.trigger_mode_cc, settings_struct.sensor_type, 0, 0, settings_struct.led_off);
	if (status != es_no_error) return status;
	switch (settings_struct.camera_system)
	{
	case camera_system_3001:
		InitCamera3001(drvno, settings_struct.gain_switch);
		break;
	case camera_system_3010:
		InitCamera3010(drvno, settings_struct.ADC_Mode, settings_struct.ADC_custom_pattern, settings_struct.gain_switch);
		break;
	case camera_system_3030:
		InitCamera3030(drvno, settings_struct.ADC_Mode, settings_struct.ADC_custom_pattern, settings_struct.gain_3030, settings_struct.dac, settings_struct.dac_output[drvno-1], settings_struct.isIr);
		break;
	default:
		return es_parameter_out_of_range;
	}
	if (status != es_no_error) return status;
	//set gain switch
	status = SendFLCAM(drvno, maddr_cam, 0, (uint16_t)settings_struct.gain_switch);
	if (status != es_no_error) return status;
	//for cooled Cam
	status = SetTemp(drvno, (uint8_t)settings_struct.Temp_level);
	if (status != es_no_error) return status;
	//DMA
	status = SetupDma(drvno);
	if (status != es_no_error) return status;
	status = SetDmaStartMode(drvno, HWDREQ_EN);
	if (status != es_no_error) return status;
	if (INTR_EN) status = enableInterrupt(drvno);
	if (status != es_no_error) return status;
	status = SetDmaRegister(drvno, settings_struct.pixel);
	if (status != es_no_error) return status;
	continiousPause = settings_struct.cont_pause;
	status = SetBEC(drvno, settings_struct.bec_in_10ns);
	if (status != es_no_error) return status;
	status = SetXckdelay(drvno, settings_struct.xckdelay_in_10ns);
	if (status != es_no_error) return status;
	status = FindCam(drvno);
	if (status != es_no_error) return status;
	status = SetHardwareTimerStopMode(drvno, true);
	return status;
}

/**
 * \brief Set pixel count
 *
 * \param drvno PCIe board identifier
 * \param pixelcount pixel count
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetPixelCount(uint32_t drvno, uint16_t pixelcount)
{
	ES_LOG("Set pixel count: %u\n", pixelcount);
	aPIXEL[drvno] = pixelcount;
	return writeBitsS0_32(drvno, pixelcount, 0xFFFF, S0Addr_PIXREGlow);
}

/**
 * \brief Clears DAT and EC.
 * 
 * \param drv PCIe board identifier.
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
 * \param drv PCIe board identifier.
 * \return es_status_codes:
 *		-es_no_error
 *		-es_register_read_failed
 *		-es_register_write_failed
 */
es_status_codes AbortMeasurement( uint32_t drv )
{
	ES_LOG("Abort Measurement\n");
	abortMeasurementFlag = true;
	es_status_codes status = StopSTimer( drv );
	if (status != es_no_error) return status;
	status = resetBlockOn(drv);
	if (status != es_no_error) return status;
	status = resetMeasureOn(drv);
	if (status != es_no_error) return status;
	return ResetDma( drv );
}

/**
 * \brief Sets BlockOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes setBlockOn( uint32_t drvno )
{
	notifyBlockStart();
	return setBitS0_32( drvno, PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS );
}

/**
 * \brief Sets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
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
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes resetBlockOn( uint32_t drvno )
{
	notifyBlockDone();
	return resetBitS0_32( drvno, PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS );
}

/**
 * \brief Resets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
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
 * \param drvno board number (=1 if one PCI board).
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
	// DCSR: reset the Iniator Reset 
	RegisterValues = 0x0;
	status = writeBitsDma_32(drvno, RegisterValues, BitMask, DmaAddr_DCSR);
	if (status != es_no_error)
		ES_LOG("switch off the Initiator Reset for the DMA failed\n");
	return status;
}

/**
 * \brief Set cam count
 *
 * \param drvno PCIe board identifier
 * \param camcount camera count
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetCamCount(uint32_t drvno, uint16_t camcount)
{
	ES_LOG("Set cam count: %u\n", camcount);
	aCAMCNT[drvno] = camcount;
	return writeBitsS0_32(drvno, camcount, 0xF, S0Addr_CAMCNT);
};

/**
 * \brief Sets sensor type (Reg TOR:D25 -> manual).
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param sensor_type Determines sensor type.
 * 		- 0: PDA (line sensor)
 * 		- 1: FFT (area sensor)
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes SetSensorType( uint32_t drvno, uint8_t sensor_type )
{
	ES_LOG("Setting sensor type: %u\n", sensor_type);
	es_status_codes status = es_no_error;
	switch (sensor_type)
	{
	case 0: //PDA
		status = resetBitS0_8(drvno, TOR_MSB_bitindex_ISFFT, S0Addr_TOR_MSB);
		if (status != es_no_error) return status;
		status = setBitS0_8(drvno, TOR_MSB_bitindex_SENDRS, S0Addr_TOR_MSB);
		if (status != es_no_error) return status;
		status = setBitS0_8(drvno, TOR_MSB_bitindex_SHORTRS, S0Addr_TOR_MSB); //if 3030
		if (status != es_no_error) return status;
		status = OpenShutter(drvno);
		break;
	case 1: //FFT
		status = setBitS0_8(drvno, TOR_MSB_bitindex_ISFFT, S0Addr_TOR_MSB);
		if (status != es_no_error) return status;
		status = resetBitS0_8(drvno, TOR_MSB_bitindex_SENDRS, S0Addr_TOR_MSB);
		break;
	default:
		return es_parameter_out_of_range;
	}
	return status;
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 * 
 * @param Data 
 * @param Bitmask 
 * @param Address 
 * @param drvno PCIe board identifier.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes writeBitsS0_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint16_t address  )
{
	return writeBitsDma_32(drvno, data, bitmask, address + S0_SPACE_OFFSET);
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 * 
 * @param Data 
 * @param Bitmask 
 * @param Address 
 * @param drvno PCIe board identifier.
 * @return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
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
 * 		- es_register_write_failed
 */
es_status_codes setBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32(drvno, 0xFFFFFFFF, bitmask, address);
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
 * 		- es_register_write_failed
 */
es_status_codes setBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
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
 * 		- es_register_write_failed
 */
es_status_codes resetBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32(drvno, 0x0, bitmask, address);
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
 * 		- es_register_write_failed
 */
es_status_codes resetBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_8(drvno, 0x0, bitmask, address);
}

es_status_codes writeRegisterS0_32( uint32_t drvno, uint32_t data, uint16_t address )
{
	return writeRegister_32(drvno, data, address + S0_SPACE_OFFSET);
}

es_status_codes writeRegisterS0_32twoBoards(uint32_t data1, uint32_t data2, uint16_t address)
{
	return writeRegister_32twoBoards(data1, data2, address + S0_SPACE_OFFSET);
}

es_status_codes writeRegisterS0_16( uint32_t drvno, uint16_t data, uint16_t address )
{
	return writeRegister_16(drvno, data, address + S0_SPACE_OFFSET);
}

es_status_codes writeRegisterS0_8( uint32_t drvno, uint8_t data, uint16_t address )
{
	return writeRegister_8(drvno, data, address + S0_SPACE_OFFSET);
}

es_status_codes writeRegisterS0_8twoBoards(uint8_t data1, uint8_t data2, uint16_t address)
{
	return writeRegister_8twoBoards(data1, data2, address + S0_SPACE_OFFSET);
}

es_status_codes readRegisterS0_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
	return readRegister_32(drvno, data, address + S0_SPACE_OFFSET);
}

es_status_codes readRegisterS0_16( uint32_t drvno, uint16_t* data, uint16_t address )
{
	return readRegister_16(drvno, data, address + S0_SPACE_OFFSET);
}

es_status_codes readRegisterS0_8( uint32_t drvno, uint8_t* data, uint16_t address )
{
	return readRegister_8(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * \brief Open shutter for sensors with EC (exposure control) / sets IFC signal = high.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OpenShutter( uint32_t drvno )
{
	ES_LOG("Open shutter\n");
	return setBitS0_8(drvno, CTRLB_bitindex_SHON, S0Addr_CTRLB);
};

/**
 * \brief For FFTs: Setup full binning.
 * 
 * \param drvno PCIe board identifier.
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
 * \param drvno board number (=1 if one PCI board)
 * \param lines number of vertical lines
 * \param vfreq vertical clk frequency
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
 * \brief sets Vertical Partial Binning in registers R10,R11 and and R12. Only for FFT sensors.
 *
 * \param drvno PCIe board identifier
 * \param range specifies R 1..5
 * \param lines number of vertical clks for next read
 * \param keep TRUE if scan should be written to FIFO
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetupVPB(uint32_t drvno, uint32_t range, uint32_t lines, bool keep)
{
	ES_LOG("SetupVPB, range: 0x%x, lines: 0x%x, keep: %x\n", range, lines, keep);
	uint32_t adr = 0;
	lines *= 2; //vclks=lines*2
	switch (range)
	{
	case 1:
		adr = 0x68;//0x40;
		break;
	case 2:
		adr = 0x6A;//0x42;
		break;
	case 3:
		adr = 0x6C;//0x44;
		break;
	case 4:
		adr = 0x6E;//0x46;
		break;
	case 5:
		adr = 0x70;//0x48;
		break;
	case 6:
		adr = 0x72;//0x4A;
		break;
	case 7:
		adr = 0x74;//0x4C;
		break;
	case 8:
		adr = 0x76;//0x4E;
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
	es_status_codes status = writeRegisterS0_32(drvno, number_of_regions, S0Addr_ARREG);
	if (status != es_no_error) return status;
	return setBitS0_32(drvno, 15, S0Addr_ARREG);//this turns ARREG on and therefore partial binning too
}

/**
 * \brief Turns ARREG off and therefore partial binning too.
 * 
 * \param drvno PCIe board identifier.
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
 * \brief Setup measurement parameters.
 * 
 * Sets registers in PCIe card and allocates resources.
 * Call this func once as it takes time to allocate the resources.
 * But be aware: the buffer size and nos is set here and may not be changed later.
 * \param drvno PCIe board identifier.
 * \param nos number of samples
 * \param nob number of blocks
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 *		- es_allocating_memory_failed
 *		- es_not_enough_ram
 */
es_status_codes SetMeasurementParameters( uint32_t drvno, uint32_t nos, uint32_t nob )
{
	Nob = nob;
	*Nospb = nos;
	ES_LOG( "Set measurement parameters: drv: %i nos: %i and nob: %i\n", drvno, nos, nob );
	//stop all and clear FIFO
	es_status_codes status = StopSTimer( drvno );
	if (status != es_no_error) return status;
	status = RSFifo( drvno );
	if (status != es_no_error) return status;
	status = allocateUserMemory(drvno);
	if (status != es_no_error) return status;
	//set hardware regs
	status = SetDMABufRegs(drvno);
	if (status != es_no_error) return status;
	uint32_t dmaBufferPartSizeInScans = DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS; //500
	if (BOARD_SEL > 2)
		numberOfInterrupts = Nob * (*Nospb) * aCAMCNT[drvno] * number_of_boards / dmaBufferPartSizeInScans - 2;//- 2 because intr counter starts with 0
	else
		numberOfInterrupts = Nob * (*Nospb) * aCAMCNT[drvno] / dmaBufferPartSizeInScans - 1;//- 1 because intr counter starts with 0
	ES_LOG("Number of interrupts: 0x%x \n", numberOfInterrupts);
	return status;
}

/**
 * \brief Stop S Timer.
 * 
 * Clear Bit30 of XCK-Reg: 0= timer off
 * \param drvno PCIe board identifier.
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
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes RSFifo( uint32_t drvno )
{
	ES_LOG("Reset Fifo\n");
	es_status_codes status = setBitS0_8(drvno, BTRIGREG_bitindex_RSFIFO, S0Addr_BTRIGREG);
	if (status != es_no_error) return status;
	return resetBitS0_8(drvno, BTRIGREG_bitindex_RSFIFO, S0Addr_BTRIGREG);
}

/**
 * Allocate user memory.
 * 
 * \param drvno PCIe board identifier.
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
	int64_t memory_free_mb = memory_free / (1024 * 1024);
	int64_t needed_mem = (int64_t)aCAMCNT[drvno] * (int64_t)Nob * (int64_t)(*Nospb) * (int64_t)aPIXEL[drvno] * (int64_t)sizeof( uint16_t );
	int64_t needed_mem_mb = needed_mem / (1024 * 1024);
	ES_LOG( "Allocate user memory, available memory:%ld MB, memory needed: %ld MB (%ld)\n", memory_free_mb, needed_mem_mb, needed_mem );
	//check if enough space is available in the physical ram
	if (memory_free > (uint64_t)needed_mem)
	{
		uint16_t* userBufferTemp = (uint16_t*) malloc( needed_mem );
		ES_LOG( "user buffer space: %p - %p\n", userBufferTemp, userBufferTemp + needed_mem );
		if (userBufferTemp)
		{
			userBuffer[drvno] = userBufferTemp;
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
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes SetDMABufRegs( uint32_t drvno )
{
	ES_LOG("Set DMA buffer registers, ");
	//DMABufSizeInScans - use 1 block
	es_status_codes status = writeBitsS0_32(drvno, DMA_BUFFER_SIZE_IN_SCANS, 0xffffffff, S0Addr_DmaBufSizeInScans);
	if (status != es_no_error) return status;
	//scans per intr must be 2x per DMA_BUFFER_SIZE_IN_SCANS to copy hi/lo part
	//aCAMCNT: double the INTR if 2 cams
	status = writeBitsS0_32(drvno, DMA_DMASPERINTR, 0xffffffff, S0Addr_DMAsPerIntr);
	if (status != es_no_error) return status;
	ES_LOG( "scansPerInterrupt/camcnt: 0x%x \n", DMA_DMASPERINTR / aCAMCNT[drvno] );
	status = writeBitsS0_32(drvno, *Nospb, 0xffffffff, S0Addr_NOS);
	if (status != es_no_error) return status;
	status = writeBitsS0_32(drvno, Nob, 0xffffffff, S0Addr_NOB);
	if (status != es_no_error) return status;
	return writeBitsS0_32(drvno, aCAMCNT[drvno], 0xffffffff, S0Addr_CAMCNT);
}

/**
 * \brief Sets the IFC Bit of Interface for sensors with shutter function. IFC=low
 * 
 * \param drvno board number (=1 if one PCI board)
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
 * \param drvno PCIe board identifier
 * \param ecin10ns Time in 10 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetSEC( uint32_t drvno, uint32_t ecin10ns )
{
	ES_LOG("Set SEC. EC in 10 ns: %u\n", ecin10ns);
	es_status_codes status = es_no_error;
	if (ecin10ns <= 1)
		status = writeRegisterS0_32(drvno, 0, S0Addr_SEC);
	else
	{
		ecin10ns |= 0x80000000; // enable delay
		status = writeRegisterS0_32(drvno, ecin10ns, S0Addr_SEC);
	}
	return status;
};

/**
 * \brief Set signal of output port of PCIe card.
 * 
 * \param drvno PCIe board identifier
 * \param fkt select output signal
 * 	- 0  XCK
 * 	- 1  REG -> OutTrig
 * 	- 2  VON
 * 	- 3  DMA_ACT
 * 	- 4  ASLS
 * 	- 5  STIMER
 * 	- 6  BTIMER
 * 	- 7  ISR_ACT
 * 	- 8  S1
 * 	- 9  S2
 * 	- 10 BON
 * 	- 11 MEASUREON
 * 	- 12 SDAT
 * 	- 13 BDAT
 * 	- 14 SSHUT
 * 	- 15 BSHUT
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetTORReg( uint32_t drvno, uint8_t tor )
{
	ES_LOG("Set TOR: %u\n", tor);
	uint8_t read_val = 0;
	es_status_codes status = readRegisterS0_8( drvno, &read_val, S0Addr_TOR_MSB);
	if (status != es_no_error) return status;
	read_val &= 0x0f; //dont disturb lower bits
	tor = tor << 4;
	tor |= read_val;
	return writeRegisterS0_8( drvno, tor, S0Addr_TOR_MSB);
}

/**
 * \brief Set the external trigger slope for scan trigger (PCI Reg CrtlA:D5 -> manual).
 *
 * \param drvno board number (=1 if one PCI board)
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
};

/**
 * \brief Sets slope for block trigger.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param slope 1 for positive slope
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
* \param drvno PCIe board identifier.
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
* \param drvno PCIe board identifier.
* \param bti_mode Defines the input mode for BTI.
* 	- 0: I
* 	- 1: S1
* 	- 2: S2
* 	- 3: S1&s2
* 	- 4: BTIMER
* \return es_status_codes
* 	- es_no_error
* 	- es_register_read_failed
* 	- es_register_write_failed
*/
es_status_codes SetBTI( uint32_t drvno, uint8_t bti_mode )
{
	ES_LOG("Set BTI: %u\n", bti_mode);
	return writeBitsS0_8( drvno, bti_mode << CTRLB_bitindex_BTI0, CTRLB_bit_BTI0 | CTRLB_bit_BTI1 | CTRLB_bit_BTI2, S0Addr_CTRLB );
}

/**
 * \brief Sets time for scan timer.
 * 
 * \param drvno board number (=1 if one PCI board)
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
 * \param drvno board number (=1 if one PCI board)
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
 * \param drvno PCIe board identifier.
 * \param delay GPX offset is used to increase accuracy. A counter value can be added, usually 1000.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes InitGPX( uint32_t drvno, uint32_t delay )
{
	ES_LOG("Init GPX, delay: %u\n", delay);
	uint32_t regData, err_cnt = 0;
	uint32_t mask = 0x3FFFF;
	delay &= mask;
	uint32_t regVal = 0x08200000 | delay;
	uint32_t RegData[12][2] = {
		{ 0, 0x000000AB },	// write to reg0: 0x80    disable inputs
	{ 1, 0x0620620 },	// write to reg1: 0x0620620 channel adjust
	{ 2, 0x00062FFC },	// write to reg2: 62E04  R-mode, disable all CH
	{ 3, 0x00000000 },	// write to reg3: 0 set to ecl
	{ 4, 0x02000000 },	// write to reg4: 0x02000000 EF flag=on
	{ 6, 0x08000001 },	// write to reg6: ecl + FILL=1
	{ 7, 0x00001FB4 },	// write to reg7: res= 27ps 
	{ 11, 0x07ff0000 },	// write to reg11: 7ff all error flags (layout flag is not connected)
	{ 12, 0x00000000 },	// write to reg12: no ir flags - is used anyway when 1 hit
	{ 14, 0x0 },
	//scharf setzen
	{ 4, 0x02400000 },	// write to reg4: master reset
	{ 2, 0x00062004 }	// write to reg2: 62E04  R-mode, en CH0..5 (3 werte
	};
	// setupo GPX chip for mode M
	//reset GPX  ´bit0 in GPXCTRL reg
	es_status_codes status = readRegisterS0_32( drvno, &regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	regData |= 0x01;
	status = writeRegisterS0_32( drvno, regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	regData &= 0xFFFFFFFE;
	status = writeRegisterS0_32( drvno, regData, S0Addr_TDCCtrl ); //reset bit
	if (status != es_no_error) return status;
	//setup R mode -> time between start and stop
	status = SetGPXCtrl( drvno, 5, regVal ); // write to reg5: 82000000 retrigger, disable after start-> reduce to 1 val
	if (status != es_no_error) return status;
	for (int write_reg = 0; write_reg < 12; write_reg++)
	{
		status = SetGPXCtrl( drvno, RegData[write_reg][0], RegData[write_reg][1] );//write
		if (status != es_no_error) return status;
		status = ReadGPXCtrl( drvno, RegData[write_reg][0], &regData );//read
		if (RegData[write_reg][1] != regData) err_cnt++;//compare write data with readdata
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
	uint32_t regData = 0,
		tempData = 0;
	//Read old data of ctrl gpx reg
	es_status_codes status = readRegisterS0_32(drvno, &regData, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	//shift gpx addr to the right place for the gpx ctrl reg
	tempData = GPXAddress << 28;
	//set CSexpand bit: reset CS Bit
	tempData &= 0xF0000000;
	//hold the other bits of the ctrl gpx reg
	regData &= 0x07FFFFFF;
	//combine the old ctrl bits with the new address
	regData |= tempData;
	//write to the gpxctrl reg
	status = writeRegisterS0_32(drvno, regData, S0Addr_TDCCtrl);
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
	uint32_t regData = 0,
		tempData = 0;
	//Read old data of ctrl gpx reg
	es_status_codes status = readRegisterS0_32( drvno, &regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	//shift gpx addr to the right place for the gpx ctrl reg
	tempData = GPXAddress << 28;
	//set CSexpand bit set CS Bit
	tempData |= 0x08000000;
	//hold the other bits of the ctrl gpx reg
	regData &= 0x07FFFFFF;
	//combine the old ctrl bits with the new address
	regData |= tempData;
	//write to the gpxctrl reg
	status = writeRegisterS0_32(drvno, regData, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return readRegisterS0_32( drvno, GPXData, S0Addr_TDCData );
}

/**
 * \brief Sets delay after trigger hardware register.
 * 
 * \param drvno PCIe board identifier.
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
};

/**
 * \brief Sets delay after trigger hardware register.
 * 
 * \param drvno PCIe board identifier.
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
};

/**
 * \brief General init routine for all Camera Systems.
 * 
 * 	Sets register in camera.
 * \param drvno selects PCIe board
 * \param pixel pixel count of camera
 * \param trigger_input for Camera Control (CC): selects CC trigger input. 0 - XCK, 1 - EXTTRIG connector, 2 - DAT
 * \param is_fft =1 vclk on, =0 vclk off
 * \param is_area =1 area mode on, =0 area mode off
 * \param IS_COOLED =1 disables PCIe FIFO when cool cam transmits cool status
 * \param led_off 1 led off, 0 led on
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes InitCameraGeneral( uint32_t drvno, uint16_t pixel, uint16_t cc_trigger_input, uint8_t is_fft, uint8_t is_area, uint8_t IS_COOLED, uint16_t led_off)
{
	ES_LOG("Init camera general, pixel: %u, cc_trigger: %u, fft: %u, area: %u, cooled: %u\n", pixel, cc_trigger_input, is_fft, is_area, IS_COOLED);
	es_status_codes status = es_no_error;
	// when TRUE: disables PCIe FIFO when cool cam transmits cool status
	if (IS_COOLED)
		status = Use_ENFFW_protection( drvno, true );
	else
		status = Use_ENFFW_protection( drvno, false );
	if (status != es_no_error) return status;
	//set camera pixel register
	status = SendFLCAM( drvno, maddr_cam, cam_adaddr_pixel, pixel );
	if (status != es_no_error) return status;
	//set trigger input of CamControl
	status = SendFLCAM( drvno, maddr_cam, cam_adaddr_trig_in, cc_trigger_input );
	if (status != es_no_error) return status;
	//set led off
	SendFLCAM(drvno, maddr_cam, cam_adaddr_LEDoff, led_off );
	//select vclk and Area mode on
	if (is_area>0)	{	is_area = (uint8_t)0x8000; }
	else { is_area = 0x0000; }
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_vclk, (uint16_t) (is_fft | is_area) );
}

/**
 * \brief Protects ENFFW from cool cam status transmission. Enable with cool cam, disable with HS > 50 kHz.
 * 
 * 	RX_VALID usually triggers ENFFW. This must be disabled when cool cams transmit their cooling status.
 * 	RX_VALID_EN is enabled with XCKI and disabled with ~CAMFFXCK_ALL, after all frame data is collected.
 * 	If RX_VALID raises again for cool status values, it doesn't effect ENFFW when RX_VALID_EN is low.
 * \param drvno selects PCIe board
 * \param use_EN enables or disables RX_VALID write protection
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes Use_ENFFW_protection( uint32_t drvno, bool USE_ENFFW_PROTECT )
{
	if (USE_ENFFW_PROTECT)
		return setBitS0_32( drvno, 3, S0Addr_PCIEFLAGS );
	else
		return resetBitS0_32( drvno, 3, S0Addr_PCIEFLAGS );
}

/**
 * \brief Sends data via fibre link, e.g. used for sending data to ADC (ADS5294).
 * 
 * Send setup:
 * - d0:d15 = data for AD-Reg  ADS5294
 * - d16:d23 = ADR of  AD-Reg
 * - d24 = ADDR0		AD=1
 * - d25 = ADDR1		AD=0
 * - d26 makes load pulse
 * - all written to DB0 in Space0 = Long0
 * - for AD set maddr=01, adaddr address of reg
 * \param drvno board number (=1 if one PCI board)
 * \param maddr master address for specifying device (2 for ADC)
 * \param adaddr register address
 * \param data data
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SendFLCAM( uint32_t drvno, uint8_t maddr, uint8_t adaddr, uint16_t data )
{
	uint32_t ldata = 0;
	ldata = maddr;
	ldata = ldata << 8;
	ldata |= adaddr;
	ldata = ldata << 16;
	ldata |= data;
	es_status_codes status = writeRegisterS0_32( drvno, ldata, S0Addr_DBR );
	if (status != es_no_error) return status;
#ifdef __linux__
    usleep(500);
#endif
	//load val
	ldata |= 0x4000000;
	status = writeRegisterS0_32(drvno, ldata, S0Addr_DBR);
	if (status != es_no_error) return status;
#ifdef __linux__
    usleep(500);
#endif
	//rs load
	ldata = 0;
	return writeRegisterS0_32(drvno, ldata, S0Addr_DBR);
}

/**
 * \brief Init routine for Camera System 3001.
 * 
 * 	Sets register in camera.
 * \param drvno selects PCIe board
 * \param gain_switch 1 gain hi, 0 gain lo (for IR sensors)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes InitCamera3001( uint32_t drvno, uint8_t gain_switch )
{
	ES_LOG("Init camera 3001, gain switch: %u\n", gain_switch);
	es_status_codes status = Use_ENFFW_protection( drvno, true );
	if (status != es_no_error) return status;
	//set gain switch (mostly for IR sensors)
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_gain, gain_switch);
}

/**
 * \brief Init routine for Camera System 3010.
 * 
 * 	Sets registers in camera and ADC LTC2271.
 * 	FL3010 is intended for sensor S12198 !
 * 	with frame rate 8kHz = min. 125µs exp time
 * \param drvno selects PCIe board
 * \param pixel pixel amount of camera
 * \param adc_mode 0: normal mode, 2: custom pattern
 * \param custom_pattern fixed output for testmode, ignored when testmode FALSE
 * \param gain_switch 1 gain hi, 0 gain lo (for IR sensors)
 * \return void
 */
es_status_codes InitCamera3010( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern, uint16_t gain_switch )
{
	ES_LOG("Init camera 3010, adc_mode: %u, custom_pattern: %u\n", adc_mode, custom_pattern);
	es_status_codes status = Cam3010_ADC_reset( drvno );
	if (status != es_no_error) return status;
	status = Cam3010_ADC_setOutputMode(drvno, adc_mode, custom_pattern);
	if (status != es_no_error) return status;
	//set gain switch (mostly for IR sensors)
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_gain, gain_switch);
}

/**
 * \brief ADC reset routine for Camera System 3010.
 * 
 * 	ADC LTC2271 neets a reset via SPI first. Bit D7
 * 	of the resetregister A0 with address 00h is set to 1.
 * 	D6:D0 are don't care. So address is 00h and data is
 * 	80h = 10000000b for e.g.
 * 	This has to be done after every startup.
 * 	Then the ADC can be programmed further via SPI in the next frames.
 * 	Called by InitCamera3010
 * \param drvno selects PCIe board
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes Cam3010_ADC_reset( uint32_t drvno )
{
	ES_LOG("Camera 3010 ADC reset\n");
	return SendFLCAM(drvno, maddr_adc, adc_ltc2271_regaddr_reset, adc_ltc2271_msg_reset );
}

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
 * \param adc_mode 0: normal mode, 1: ramp, 2: custom pattern
 * \param custom_pattern only used when adc_mode = 2, lower 14 bits are used as output of ADC
 * \param gain in ADC
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes InitCamera3030( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern, uint8_t gain, bool useDac, uint32_t* dac_output, bool isIr )
{
	ES_LOG("Init camera 3030, adc_mode: %u, custom_pattern: %u, gain: %u, use dac: %u, isIr: %u\n", adc_mode, custom_pattern, gain, useDac, isIr);
	es_status_codes status = Cam3030_ADC_reset( drvno );
	if (status != es_no_error) return status;
	//two wire mode output interface for pal versions P209_2 and above
	status = Cam3030_ADC_twoWireModeEN( drvno );
	if (status != es_no_error) return status;
	status = Cam3030_ADC_SetGain( drvno, gain );
	if (status != es_no_error) return status;
	if (adc_mode)
		status = Cam3030_ADC_RampOrPattern( drvno, adc_mode, custom_pattern );
	//DAC
	int dac_channel_count = 8;
	if (useDac)
	{
		SendFLCAM_DAC(drvno, dac_channel_count, 0, 0, 1);
		DAC_setAllOutputs(drvno, dac_output, isIr);
	}
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
 * 	Sets gain of ADC ADS5294 0...15 by callig SetADGain() subroutine.
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
 * \param drvno PCIe board identifier.
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
	data = data << 4;
	data |= e;
	data = data << 4;
	data |= b;
	data = data << 4;
	data |= f;
	es_status_codes status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_gain_1_to_4, data );	//gain1..4
	if (status != es_no_error) return status;
	data = h;
	data = data << 4;
	data |= d;
	data = data << 4;
	data |= g;
	data = data << 4;
	data |= c;
	return SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_gain_5_to_8, data );	//gain7..8
}

/**
 * \brief ADC debug mode for Camera System 3030.
 * 
 * 	Lets ADC send a ramp or a custom pattern (value) instead of ADC sample data.
 * 	Called by InitCamera3030 when adc_mode > 0.
 * \param drvno selects PCIe board
 * \param adc_mode 1: ramp, 2: custom pattern
 * \param custom_pattern (only used when adc_mode = 2)
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
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
		//at addr 0x25 (mode and higher bits): 0b00000000000100dd
		status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_mode, adc_ads5294_msg_custompattern | ((custom_pattern >> 12) & 0x3) );
		//at addr 0x26 (lower bits): 0bdddddddddddd0000
		status = SendFLCAM( drvno, maddr_adc, adc_ads5294_regaddr_custompattern, custom_pattern << 4 );
		break;
	default:
		break;
	}
	return status;
}

/**
 * \brief Set temperature level for cooled cameras.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param level level 0..7 / 0=off, 7=min -> see cooling manual
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetTemp( uint32_t drvno, uint8_t level )
{
	if (level >= 8) level = 0;
	ES_LOG("Set temperature level: %u\n", level);
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_coolTemp, level );
}

/**
 * \brief Sends data via fibre link to DAC8568. Mapping of bits in DAC8568: 4 prefix, 4 control, 4 address, 16 data, 4 feature.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param ctrl 4 control bits
 * \param addr 4 address bits
 * \param data 16 data bits
 * \param feature 4 feature bits
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes SendFLCAM_DAC( uint32_t drvno, uint8_t ctrl, uint8_t addr, uint16_t data, uint8_t feature )
{
	uint16_t	hi_bytes = 0,
		lo_bytes = 0;
	uint8_t	maddr_DAC = 0b11,
		hi_byte_addr = 0x01,
		lo_byte_addr = 0x02;

	if (ctrl & 0x10) //4 ctrl bits => only lower 4 bits allowed
	{
		ES_LOG( "SendFLCAM_DAC: Only values between 0 and 15 are allowed for control bits." );
		return es_parameter_out_of_range;
	}
	if (addr & 0x10) //4 addr bits => only lower 4 bits allowed
	{
		ES_LOG( "SendFLCAM_DAC: Only values between 0 and 15 are allowed for address bits." );
		return es_parameter_out_of_range;
	}
	if (feature & 0x10) //4 ctrl bits => only lower 4 bits allowed
	{
		ES_LOG( "SendFLCAM_DAC: Only values between 0 and 15 are allowed for feature bits." );
		return es_parameter_out_of_range;
	}
	hi_bytes |= 0x0;	//4 prefix bits, first bit always 0 (0xxx)
	hi_bytes <<= 4;
	hi_bytes |= ctrl & 0x0F;	//4 control bits
	hi_bytes <<= 4;
	hi_bytes |= addr & 0x0F;	//4 address bits
	hi_bytes <<= 4;
	hi_bytes |= data >> 12; //4 data bits (upper 4 bits of 16 bits data)
	lo_bytes |= data & 0x0FFF; //12 data bits (lower 12 bits of 16 bit data)
	lo_bytes <<= 4;
	lo_bytes |= feature;	//4 feature bits
	es_status_codes status = SendFLCAM( drvno, maddr_DAC, hi_byte_addr, hi_bytes );
	if (status != es_no_error) return status;
	return SendFLCAM( drvno, maddr_DAC, lo_byte_addr, lo_bytes );
}

/**
 * \brief Sets all outputs of the DAC8568 on PCB 2189-7.
 * 
 * Use this function to set the outputs, because it is resorting the channel numeration correctly.
 * \param drvno pcie board identifier
 * \param output all output values that will be converted to analog voltage (0 ... 0xFFFF)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes DAC_setAllOutputs(uint32_t drvno, uint32_t* output, bool isIR)
{
	es_status_codes status = es_no_error;
	int* reorder_ch;
	if (isIR)
	{
		int tmp[8] = { 0,1,2,3,4,5,6,7 }; //IR
		reorder_ch = tmp;
	}
	else
	{
		int tmp[8] = { 3, 4, 0, 5, 1, 6, 2, 7 }; //vis
		reorder_ch = tmp;
	}
	for (uint8_t channel = 0; channel < 8; channel++)
	{
		status = DAC_setOutput(drvno, channel, output[reorder_ch[channel]]);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Sets the output of the DAC8568 on PCB 2189-7.
 * 
 * \param drvno pcie board identifier
 * \param channel select one of eight output channel (0 ... 7)
 * \param output output value that will be converted to analog voltage (0 ... 0xFFFF)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
es_status_codes DAC_setOutput( uint32_t drvno, uint8_t channel, uint16_t output )
{
	//ctrl 3: write and update DAC register
	ES_LOG("Set DAC output ch%u = %u\n", channel, output);
	return SendFLCAM_DAC( drvno, 3, channel, output, 0 );
}

/**
 * \brief Exposure control (EC) signal is used for mechanical shutter or sensors with EC function.
 * 
 * Starts after delay after trigger (DAT) signal and is active for ecin10ns.
 * Resets additional delay after trigger with ecin10ns = 0.
 * \param drvno PCIe board identifier
 * \param ecin10ns Time in 10 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetBEC( uint32_t drvno, uint32_t ecin10ns )
{
	ES_LOG("Set BEC in 10 ns: %u\n", ecin10ns);
	es_status_codes status = es_no_error;
	if (ecin10ns)
	{
		ecin10ns |= 0x80000000; // enable delay
		status = writeRegisterS0_32(drvno, ecin10ns, S0Addr_BEC);
	}
	else
		status = writeRegisterS0_32(drvno, 0, S0Addr_BEC);
	return status;
};

/**
 * \brief Set XCK delay.
 * 
 * \param drvno PCIe board identifier.
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

es_status_codes SetDmaRegister( uint32_t drvno, uint32_t pixel )
{		
	ES_LOG("Set DMA register: drv: %u, pixel: %u\n", drvno, pixel);
	/*Pixelsize with matching TLP Count (TLPC).
	Pixelsize = TLPS * TLPC - 1*TLPS
	(TLPS TLP size = 64)
	TLPC 0x Pixelsize
		2	64
		3	128
		4	192
		5	256
		6	320
		7	384
		8	448
		9	512
		a	576
		b	640
		c	704
		d	768
		e	832
		f	896
		10	960
		11	1024
		12	1088
		13	1152
		14	1216
		15	1280
		16	1344
		17	1408
		18	1472
		19	1536
		1a	1600
		1b	1664
		1c	1728
		1d	1792
		1e	1856
		1f	1920
		20	1984
		21	2048
		22	2112
		23  2176
		...
		41  4096
		42  4160
		...
		82  8256
		*/
	uint32_t no_tlps = 0;
	switch (pixel)
	{
	case 128:
		no_tlps = 0x2;//3
		break;
	case 192:
		no_tlps = 0x3;//4
		break;
	case 320:
		no_tlps = 0x5;//6
		break;
	case 576:
		no_tlps = 0x9;//a
		break;
	case 1088:
		no_tlps = 0x11;
		break;
	case 2112:
		no_tlps = 0x21;//22
		break;
	case 4160:
		no_tlps = 0x41;//42
		break;
	case 8256:
		no_tlps = 0x81;//82
		break;
	default:
		if (!MANUAL_OVERRIDE_TLP)
		{
			ES_LOG("Could not choose TLP size, no valid pixel count.\n");
			return es_invalid_pixel_count;
		}
	}
	if (LEGACY_202_14_TLPCNT) no_tlps = no_tlps + 1;
	uint32_t data = 0;
	es_status_codes status = readConfig_32(drvno, &data, PCIeAddr_devCap);
	if (status != es_no_error) return status;
	int tlpmode = data & 0x7;//0xE0 ;
	if (_FORCETLPS128) tlpmode = 0;
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
		//WriteLongIOPort( drvno, BData, PCIeAddr_devStatCtrl );
		//NO_TLPS setup now in setboardvars
		tlp_size = 0x20;
		break;
	case 1:
		data |= 0x20;//set to 256 bytes = 64 DWORDS 
		//BData |= 0x00000020;//set to 256 
		//WriteLongIOPort( drvno, BData, PCIeAddr_devStatCtrl );
		//NO_TLPS = 0x9;//x9 was before. 0x10 is calculated in aboutlp and 0x7 is working;
		tlp_size = 0x40;
		break;
	case 2:
		data |= 0x40;//set to 512 bytes = 128 DWORDS 
		//BData |= 0x00000020;//set to 512 
		//WriteLongIOPort( drvno, BData, PCIeAddr_devStatCtrl );
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
	//TODO: dev->tlp_size = TLPSIZE
	status = writeConfig_32(drvno, data, PCIeAddr_devStatCtrl);
	if (status != es_no_error) return status;
	uint64_t dma_physical_address = getDmaAddress(drvno);
	// WDMATLPA (Reg name): write the lower part (bit 02:31) of the DMA adress to the DMA controller
	ES_LOG("Set WDMATLPA to physical address of dma buffer 0x%016lx\n", dma_physical_address);
	status = writeBitsDma_32(drvno, dma_physical_address, 0xFFFFFFFC, DmaAddr_WDMATLPA);
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
 * @param Data 
 * @param Bitmask 
 * @param Address 
 * @param drvno PCIe board identifier.
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
	//step 1: save Data as setbitmask for making this part humanreadable
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
 * @param Data 
 * @param Bitmask 
 * @param Address 
 * @param drvno PCIe board identifier.
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
	//step 1: save Data as setbitmask for making this part humanreadable
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

es_status_codes writeRegisterDma_32( uint32_t drvno, uint32_t data, uint16_t address )
{
	return writeRegister_32(drvno, data, address);
}

es_status_codes writeRegisterDma_8( uint32_t drvno, uint8_t data, uint16_t address )
{
	return writeRegister_8(drvno, data, address);
}

es_status_codes readRegisterDma_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
	return readRegister_32(drvno, data, address);
}

es_status_codes readRegisterDma_8( uint32_t drvno, uint8_t* data, uint16_t address )
{
	return readRegister_8(drvno, data, address);
}

/**
 * @brief Set Dma Start Mode 
 * 
 * @param drvno PCIe board identifier
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
 * \brief Const burst loop with DMA initiated by hardware DREQ. Read nos lines from FIFO. this is the main loop
 * 
 * \param board_sel sets interface board
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 *		- es_camera_not_found
 *		- es_register_write_failed
 */
es_status_codes StartMeasurement()
{
	ES_LOG("\n*** Start Measurement ***\n");
	abortMeasurementFlag = false;
	es_status_codes status = es_no_error;
	do
	{
		//reset the internal block counter and ScanIndex before START
		//set to hw stop of timer hwstop=TRUE
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
		{
			status = ResetHardwareCounter(1);
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
		{
			status = ResetHardwareCounter(2);
			if (status != es_no_error) return status;
		}
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
			ResetBufferWritePos(1);
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
			ResetBufferWritePos(2);
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
		{
			status = StartCopyDataToUserBufferThread(1);
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
		{
			status = StartCopyDataToUserBufferThread(2);
			if (status != es_no_error) return status;
		}
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
		{
			status = setMeasureOn(1);
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
		{
			status = setMeasureOn(2);
			if (status != es_no_error) return status;
		}
		//block read function
		for (uint32_t blk_cnt = 0; blk_cnt < Nob; blk_cnt++)
		{
			if(BOARD_SEL == 2)
				waitForBlockTrigger(2);
			else
				waitForBlockTrigger(1);
			if (status == es_abortion)
				return AbortMeasurement(BOARD_SEL);
			else if (status != es_no_error) return status;
			ES_LOG("Block %u triggered\n", blk_cnt);
			if (BOARD_SEL == 1 || BOARD_SEL == 3)
			{
				status = countBlocksByHardware(1);
				if (status != es_no_error) return status;
				if (BOARD_SEL != 3)
				{
					status = setBlockOn(1);
					if (status != es_no_error) return status;
					status = StartSTimer(1);
					if (status != es_no_error) return status;
					//start scan for first read if area or ROI
					if (*useSWTrig) status = DoSoftwareTrigger(1);
					if (status != es_no_error) return status;
				}
			}
			if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
			{
				status = countBlocksByHardware(2);
				if (status != es_no_error) return status;
				if (BOARD_SEL != 3)
				{
					status = setBlockOn(2);
					if (status != es_no_error) return status;
					status = StartSTimer(2);
					if (status != es_no_error) return status;
					//start scan for first read
					if (*useSWTrig) status = DoSoftwareTrigger(2);
					if (status != es_no_error) return status;
				}
			}
			//for synchronising both cams
			if (BOARD_SEL == 3)
			{
				status = setBlockOnTwoBoards();
				if (status != es_no_error) return status;
				status = StartSTimerTwoBoards();
				if (status != es_no_error) return status;
				//start scan for first read
				if (*useSWTrig) status = DoSoftwareTriggerTwoBoards();
				if (status != es_no_error) return status;
			}
			//main read loop - wait here until nos is reached or ESC key
			//if nos is reached the flag RegXCKMSB:b30 = TimerOn is reset by hardware if flag HWDREQ_EN is TRUE
			//extended to Timer_routine for all variants of one and  two boards
			if (BOARD_SEL == 1)
			{
				bool timerOneOn = true;
				while (timerOneOn)
				{
					if ((FindCam(1) != es_no_error) | abortMeasurementFlag)
						return AbortMeasurement(1);
					abortMeasurementFlag = checkEscapeKeyState();
					status = IsTimerOn(1, &timerOneOn);
					if (status != es_no_error) return status;
				}
			}
			if (number_of_boards == 2 && BOARD_SEL == 2)
			{
				bool timerTwoOn = true;
				while (timerTwoOn)
				{
					if ((FindCam(2) != es_no_error) | abortMeasurementFlag)
						return AbortMeasurement(2);
					abortMeasurementFlag = checkEscapeKeyState();
					status = IsTimerOn(2, &timerTwoOn);
					if (status != es_no_error) return status;
				}
			}
			if (number_of_boards == 2 && BOARD_SEL == 3)
			{
				bool timerOneOn = true,
					timerTwoOn = true;

				while (timerOneOn || timerTwoOn)
				{
					bool return_flag_1 = false;
					bool return_flag_2 = false;

					if (!return_flag_1)
					{
						if ((FindCam(1) != es_no_error) | abortMeasurementFlag)
						{
							status = AbortMeasurement(1);
							if (status != es_no_error) return status;
							return_flag_1 = false;
						}
					}
					if (!return_flag_2)
					{
						//stop if ESC was pressed
						if ((FindCam(2) != es_no_error) | abortMeasurementFlag)
						{
							status = AbortMeasurement(2);
							if (status != es_no_error) return status;
							return_flag_2 = true;
						}
					}
					if (return_flag_1 && return_flag_2) return status;
					status = IsTimerOn(1, &timerOneOn);
					if (status != es_no_error) return status;
					status = IsTimerOn(2, &timerTwoOn);
					if (status != es_no_error) return status;
					abortMeasurementFlag = checkEscapeKeyState();
				}
			}
			if (BOARD_SEL == 1 || BOARD_SEL == 3)
			{
				status = resetBlockOn(1);
				if (status != es_no_error) return status;
			}
			if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
			{
				status = resetBlockOn(2);
				if (status != es_no_error) return status;
			}
		}//block cnt read function
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
		{
			status = StopSTimer(1);
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
		{
			status = StopSTimer(2);
			if (status != es_no_error) return status;
		}
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
		{
			status = GetLastBufPart(1);
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
		{
			status = GetLastBufPart(2);
			if (status != es_no_error) return status;
		}
		// This sleep is here to prevent the measurement beeing interrupted too early. When operating with 2 cameras the last scan could be cut off without the sleep. This is only a workaround. The problem is that the software is waiting for RSTIMER beeing reset by the hardware before setting measure on and block on to low, but the last DMA is done after RSTIMER beeing reset. BLOCKON and MEASUREON should be reset after all DMAs are done.
		// RSTIMER --------________
		// DMAWRACT _______-----___
		// BLOCKON ---------_______
		// MEASUREON ---------_____
#ifdef WIN32
		WaitforTelapsed(100);
#endif
#ifdef __linux__
        if (BOARD_SEL == 1 || BOARD_SEL == 3)
        {
            pthread_mutex_lock(&mutex[0]);
            pthread_mutex_unlock(&mutex[0]);
        }
        if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
        {
            pthread_mutex_lock(&mutex[1]);
            pthread_mutex_unlock(&mutex[1]);
        }
#endif
		if (BOARD_SEL == 1 || BOARD_SEL == 3)
		{
			status = resetMeasureOn(1);
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
		{
			status = resetMeasureOn(2);
			if (status != es_no_error) return status;
		}
		if (checkSpaceKeyState())
			continiousMeasurementFlag = false;
		abortMeasurementFlag = checkEscapeKeyState();
	} while (continiousMeasurementFlag && !abortMeasurementFlag);
	ES_LOG("*** Measurement done ***\n\n");
	return status;
}

/**
 * \brief Test if SFP module is there and fiber is linked up.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes FindCam( uint32_t drvno )
{
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32( drvno, &data, S0Addr_PCIEFLAGS );
	if (status != es_no_error) return status;
	if ((data & 0x80000000) > 0)
	{
		//SFP error
		ES_LOG( "Fiber or Camera error\n" );
		return es_camera_not_found;
	}
	return status;
}

/**
 * \brief Reset the internal intr collect counter.
 * 
 * \param drv board number
 * \param hwstop timer is stopped by hardware if nos is reached
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
 * \param drv board number
 * \param reset_by_hardware true: timer is stopped by hardware if nos is reached
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
 * @param drvno PCIe board identifier. May only work for 1
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
		abortMeasurementFlag = checkEscapeKeyState();
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
 * \param drvno PCIe board identifier
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes countBlocksByHardware( uint32_t drvno )
{
	es_status_codes status =  pulseBitS0_32(drvno, PCIEFLAGS_bitindex_BLOCKTRIG, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	//reset scan counter
	return pulseBitS0_32(drvno, ScanIndex_bitindex_counter_reset, S0Addr_ScanIndex);
}

/**
 * \brief Sets Scan Timer on.
 * 
 * \param drvno board number (=1 if one PCI board)
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
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes DoSoftwareTrigger( uint32_t drvno )
{
	ES_LOG("Do software trigger\n");
	es_status_codes status = setBitS0_8(drvno, BTRIGREG_bitindex_SWTRIG, S0Addr_BTRIGREG);
	if (status != es_no_error) return status;
	return resetBitS0_8(drvno, BTRIGREG_bitindex_SWTRIG, S0Addr_BTRIGREG);
}

/**
 * \brief Checks if timer is active (Bit30 of XCK-Reg).
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes IsTimerOn( uint32_t drvno, bool* on )
{
	uint8_t data = 0;
	es_status_codes status = readRegisterS0_8( drvno, &data, S0Addr_XCKMSB );
	if (status != es_no_error) return status;
	data &= 0x40;
	if (data != 0) *on = true;
	else *on = false;
	return status;
}

/**
 * \brief For the rest part of the buffer.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes GetLastBufPart( uint32_t drvno )
{
	ES_LOG( "Get the last buffer part\n" );
	//get the rest if buffer is not multiple of 500 (BUFSIZEINSCANS/2)
	//also if nos is < BUFSIZEINSCANS/2 - here: no intr occurs
	uint32_t spi = 0;
	es_status_codes status = readRegisterS0_32( drvno, &spi, S0Addr_DMAsPerIntr ); //get scans per intr
	if (status != es_no_error) return status;
	//halfbufize is 500 with default values
	uint32_t dmaHalfBufferSize = DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS;
	uint32_t scans_all_cams = (*Nospb) * Nob * aCAMCNT[drvno];
	uint32_t rest_overall = scans_all_cams % dmaHalfBufferSize;
	size_t rest_in_bytes = rest_overall * aPIXEL[drvno] * sizeof( uint16_t );
	ES_LOG( "nos: 0x%x, nob: 0x%x, scansPerInterrupt: 0x%x, camcnt: 0x%x\n", (*Nospb), Nob, spi, aCAMCNT[drvno]);
	ES_LOG( "scans_all_cams: 0x%x \n", scans_all_cams );
	ES_LOG( "rest_overall: 0x%x, rest_in_bytes: 0x%zx\n", rest_overall, rest_in_bytes );
	if (rest_overall)
		copyRestData(drvno, rest_in_bytes);
	return status;
}

/**
 * \brief Initializes PCIe board.
 * 
 * \param board_sel Select PCIe boards.
 *		- 1: board one
 *		- 2: board two
 *		- 3: both boards
 * \return es_status_codes:
 *		- es_no_error
 *		- es_invalid_driver_number
 *		- es_getting_device_info_failed
 *		- es_open_device_failed
 */
es_status_codes InitBoard()
{
	ES_LOG("\n*** Init board ***\n");
	ES_LOG("Number of boards: %u\n", number_of_boards);
	if (number_of_boards < 1) return es_open_device_failed;
	es_status_codes status = _InitBoard(1);
	if (status != es_no_error) return status;
	if (number_of_boards > 1)
		status = _InitBoard(2);
    for(uint32_t drvno=1; drvno <= number_of_boards; drvno++)
        InitMutex(drvno);
	ES_LOG("*** Init board done***\n\n");
	return status;
}

es_status_codes InitDriver()
{
	ES_LOG("\n*** Init driver ***\n");
	es_status_codes status = _InitDriver();
	ES_LOG("*** Init driver done***\n\n");
	return status;
}

/**
 * \brief Exit driver.
 * 
 * \param board_sel:
 *		- 1: board one
 *		- 2: board two
 *		- 3: both boards
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_unlocking_dma_failed
 *		- es_parameter_out_of_range
 */
es_status_codes ExitDriver(uint32_t board_sel)
{
	ES_LOG("\n*** Exit driver ***\n");
	es_status_codes status = es_no_error;
	switch (board_sel)
	{
	case 1:
		status = _ExitDriver(1);
		break;
	case 2:
		status = _ExitDriver(2);
		break;
	case 3:
		status = _ExitDriver(1);
		if (status != es_no_error) return status;
		status = _ExitDriver(2);
		break;
	default:
		return es_parameter_out_of_range;
	}

	ES_LOG("*** Exit driver done***\n\n");
	return status;
}

es_status_codes ReturnFrame(uint32_t drv, uint32_t curr_nos, uint32_t curr_nob, uint16_t curr_cam, uint16_t* pdest, uint32_t length)
{
	//ES_LOG( "Return frame: drvno: %u, curr_nos: %u, curr_nob: %u, curr_cam: %u, pdest %p, length: %u\n", drv, curr_nos, curr_nob, curr_cam, pdest, length );
	es_status_codes status = checkDriverHandle(drv);
	if (status != es_no_error) return status;
	uint16_t* pframe = NULL;
	status = GetAddressOfPixel( drv, 0, curr_nos, curr_nob, curr_cam, &pframe);
	if (status != es_no_error) return status;
	//ES_LOG("pframe %p\n", pframe);
	//ES_LOG("userBuffer %p\n", userBuffer[drv]);
	memcpy( pdest, pframe, length * sizeof( uint16_t ) );
	return status;
}


/**
 * \brief Returns the index of a pixel located in userBuffer.
 * 
 * \param drvno indentifier of PCIe card
 * \param pixel position in one scan (0...(PIXEL-1))
 * \param sample position in samples (0...(nos-1))
 * \param block position in blocks (0...(nob-1))
 * \param CAM position in camera count (0...(CAMCNT-1)
 * \param pIndex Pointer to index of pixel.
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetIndexOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t CAM, uint64_t* pIndex )
{
	if (pixel >= aPIXEL[drvno] || sample >= *Nospb || block >= Nob || CAM >= aCAMCNT[drvno])
		return es_parameter_out_of_range;
	//init index with base position of pixel
	uint64_t index = pixel;
	//position of index at CAM position
	index += (uint64_t)CAM *((uint64_t)aPIXEL[drvno] + 4);  //GS! offset of 4 pixel via pipelining from CAM1 to CAM2
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
 * \param drvno indentifier of PCIe card
 * \param pixel position in one scan (0...(PIXEL-1))
 * \param sample position in samples (0...(nos-1))
 * \param block position in blocks (0...(nob-1))
 * \param CAM position in camera count (0...(CAMCNT-1))
 * \param Pointer to get address
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes GetAddressOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t CAM, uint16_t** address )
{
	uint64_t index = 0;
	es_status_codes status = GetIndexOfPixel(drvno, pixel, sample, block, CAM, &index);
	if (status != es_no_error) return status;
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
	ES_LOG("Calc ram usage in MB, nos: %u:, nob: %u\n", nos, nob);
	double ramUsage = 0;
	for (int i = 0; i < number_of_boards; i++)
		ramUsage += (uint64_t)nos * (uint64_t)nob * (uint64_t)aPIXEL[i + 1] * (uint64_t)aCAMCNT[i + 1] * sizeof(uint16_t);
	ramUsage = ramUsage / 1048576;
	ES_LOG("Ram usage: %f\n", ramUsage);
	return ramUsage;
}

/**
 * \brief Online calc TRMS noise val of pix.
 *
 * Calculates RMS of TRMS_pixel in the range of samples from firstSample to lastSample. Only calculates RMS from one block.
 * \param drvno indentifier of PCIe card
 * \param firstSample start sample to calculate RMS. 0...(nos-2). Typical value: 10, to skip overexposed first samples
 * \param lastSample last sample to calculate RMS. firstSample+1...(nos-1).
 * \param TRMS_pixel pixel for calculating noise (0...(PIXEL-1))
 * \param CAMpos index for camcount (0...(CAMCNT-1))
 * \param mwf pointer for mean value
 * \param trms pointer for noise
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_allocating_memory_failed
 */
es_status_codes CalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms)
{
	if (firstSample >= lastSample || lastSample > *Nospb)
	{
		//error: firstSample must be smaller than lastSample
		ES_LOG("Calc Trms failed. lastSample must be greater than firstSample and both in bounderies of nos, drvno: %u, firstSample: %u, lastSample: %u, TRMS_pixel: %u, CAMpos: %u, Nospb: %u\n", drvno, firstSample, lastSample, TRMS_pixel, CAMpos, *Nospb);
		*mwf = -1;
		*trms = -1;
		return es_parameter_out_of_range;
	}
	uint32_t samples = lastSample - firstSample;
	uint16_t *TRMS_pixels = calloc(samples, sizeof(uint16_t));
	if (!TRMS_pixels) return es_allocating_memory_failed;
	//storing the values of one pix for the rms analysis
	for (uint32_t scan = 0; scan < samples; scan++)
	{
		uint64_t TRMSpix_of_current_scan = 0;
		es_status_codes status = GetIndexOfPixel(drvno, TRMS_pixel, scan + firstSample, 0, CAMpos, &TRMSpix_of_current_scan);
		if (status != es_no_error) return status;
		TRMS_pixels[scan] = userBuffer[drvno][TRMSpix_of_current_scan];
	}
	//rms analysis
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
		*mwf += TRMSVals[i];//for C-Noobs: this is the same like *(TRMSVals+1)
	}
	*mwf /= nos;
	for (uint32_t i = 0; i < nos; i++)
	{// get varianz
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
 * \param drvno board number (=1 if one PCI board)
 * \param valid Is true (not 0) if FIFO keeps >= 1 complete lines (linecounter>0).
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes checkFifoFlags(uint32_t drvno, bool* valid)
{	// not empty & XCK = low -> true
	ES_LOG("checkFifoFlags\n");
	uint8_t data = 0;
	es_status_codes status = readRegisterS0_8(drvno, &data, S0Addr_FF_FLAGS);
	data &= 0x80;
	if (data > 0) *valid = true;
	else *valid = false;
	return status;
}

/**
 * \brief Check ovl flag (overflow of FIFO).
 *
 * If occured stays active until a call of FFRS.
 * \param drvno board number (=1 if one PCI board)
 * \return Is true (not 0) if overflow occured (linecounter>0).
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes checkFifoOverflow(uint32_t drvno, bool* overflow)
{
	ES_LOG("checkFifoOverflow\n");
	uint8_t data = 0;
	es_status_codes status = readRegisterS0_8(drvno, &data, S0Addr_FF_FLAGS);
	data &= 0x08; //0x20; if not saved
	if (data > 0) *overflow = true; //empty
	else *overflow = false;
	return status;
}

/**
 * \brief Check if blockon bit is set.
 *
 * \param drvno PCIe board identifier.
 * \param blockOn True when blockon bit is set.
 *  \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes isBlockOn(uint32_t drvno, bool* blockOn)
{
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	//Check for measure on bit
	if (PCIEFLAGS_bit_BLOCKON & data)
		*blockOn = true;
	else
		*blockOn = false;
	return status;
}

/**
 * \brief Check if measure on bit is set.
 *
 * \param drvno PCIe board identifier.
 * \param measureOn True when measureon bit is set.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes isMeasureOn(uint32_t drvno, bool* measureOn)
{
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	//Check for measure on bit
	if (PCIEFLAGS_bit_MEASUREON & data)
		*measureOn = true;
	else
		*measureOn = false;
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
 */
es_status_codes LedOff(uint32_t drvno, uint8_t LED_OFF)
{
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_LEDoff, (uint16_t)LED_OFF);
}

/**
 * \brief Reset trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * Functions is not optimized for 2 cams.
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes:
 * 		- es_no_error
 */
es_status_codes OutTrigLow(uint32_t drvno)
{
	return resetBitS0_32(drvno, CTRLA_bitindex_TRIG_OUT, S0Addr_CTRLA);
};

/**
 * \brief Set trigger out(Reg CtrlA:D3) of PCIe board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * Functions is not optimized for 2 cams.
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OutTrigHigh(uint32_t drvno)
{
	return setBitS0_32(drvno, CTRLA_bitindex_TRIG_OUT, S0Addr_CTRLA);
} //OutTrigHigh

/**
 * \brief Pulses trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual
 * \param drvno board number (=1 if one PCI board)
 * \param PulseWidth duration of pulse in ms
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OutTrigPulse(uint32_t drvno, uint32_t PulseWidth)
{
	es_status_codes status = OutTrigHigh(drvno);
	if (status != es_no_error) return status;
#ifdef WIN32
    Sleep(PulseWidth);
#endif
	return OutTrigLow(drvno);
}

/**
 * \brief Reads the binary state of an ext. trigger input.
 *
 * Direct read of inputs for polling.
 * \param drv board number
 * \param btrig_ch specify input channel
 * 			- btrig_ch=0 not used
 * 			- btrig_ch=1 is pcie trig in I
 * 			- btrig_ch=2 is S1
 * 			- btrig_ch=3 is S2
 * 			- btrig_ch=4 is S1&S2
 * 			- btrig_ch=5 is TSTART (GTI - DAT - EC)
 * \param state false when low, otherwise true
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readBlockTriggerState(uint32_t drv, uint8_t btrig_ch, bool* state)
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
		status = readRegisterS0_8(drv, &val, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		if ((val & CTRLA_bit_DIR_TRIGIN) > 0) *state = true;
		break;
	case 2: //S1
		status = readRegisterS0_8(drv, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x02) > 0) *state = true;
		break;
	case 3: //S2
		status = readRegisterS0_8(drv, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x04) > 0) *state = true;
		break;
	case 4: // S1&S2
		status = readRegisterS0_8(drv, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x02) == 0) *state = false;
		status = readRegisterS0_8(drv, &val, S0Addr_CTRLC);
		if (status != es_no_error) return status;
		if ((val & 0x04) == 0) *state = false;
		*state = true;
		break;
	case 5: // TSTART
		status = readRegisterS0_8(drv, &val, S0Addr_CTRLA);
		if (status != es_no_error) return status;
		if ((val & CTRLA_bit_TSTART) > 0) *state = true;
		break;
	}
	return status;
}

/**
 * \brief Sets the camera gain register.
 *
 * 	Sets corresponding camera register: maddr = 0, adadr = 0
 * 	Currently a one bit value (bit0 = 1 -> gain high), but can be used as a numerical value 0...3 in future.
 * 	Legacy cameras will only look for bit0.
 * \param drvno selects PCIe board
 * \param gain_value 1 -> gain high, 0 -> gain low
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetGain(uint32_t drvno, uint16_t gain_value)
{
	return SendFLCAM(drvno, maddr_cam, cam_adaddr_gain, gain_value);
}

/**
 * \brief Returns when block on bit is 0.
 *
 * \param drvno PCIe board identifier.
 *  \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes waitForBlockReady(uint32_t drvno)
{
	bool blockOn = true;
	es_status_codes status = es_no_error;
	while (blockOn)
	{
		status = isBlockOn(drvno, &blockOn);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Returns when measure on bit is 0.
 *
 * \param drvno PCIe board identifier.
 *  \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes waitForMeasureReady(uint32_t drvno)
{
	bool measureOn = true;
	es_status_codes status = es_no_error;
	while (measureOn)
	{
		status = isMeasureOn(drvno, &measureOn);
		if (status != es_no_error) return status;
	}
	return status;
}

es_status_codes dumpS0Registers(uint32_t drvno, char** stringPtr)
{
	enum N
	{ 
		number_of_registers = 43,
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
		"R14 nc"DLLTAB,
		"R15 nc"DLLTAB,
		"R16 BTimer",
		"R17 BDAT",
		"R18 BEC"DLLTAB,
		"R19 BFLAGS",
		"R20 ADSC1",
		"R21 LDSC1",
		"R22 ADSC2",
		"R23 LDSC2",
		"R24 ADSC3",
		"R25 LDSC3",
		"R26 DSCCTRL"
	}; //Look-Up-Table for the S0 Registers
	uint32_t data = 0;
	//allocate string buffer buffer
	*stringPtr = (char*) calloc(number_of_registers * bufferLength, sizeof(char));
	unsigned int len = 0; 
	es_status_codes status = es_no_error;
	for (int i = 0; i <= number_of_registers - 1; i++)
	{
		//read register
		status = readRegisterS0_32(drvno, &data, i * 4);
		if (status != es_no_error)
		{
			//write error to buffer
			len += sprintf(*stringPtr + len, "\nerror while reading register %s", register_names[i]);
			return status;
		}
		//write register name and value to buffer
		len += sprintf(*stringPtr + len, "%s\t0x%x\n", register_names[i], data);
	}
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
	//allocate string buffer buffer
	*stringPtr = (char*)calloc(number_of_registers * bufferLength, sizeof(char));
	unsigned int len = 0;
	es_status_codes status = es_no_error;
	for (int i = 0; i < number_of_registers; i++)
	{
		es_status_codes status = readRegisterDma_32(drvno, &data, i * 4);
		if (status != es_no_error)
		{
			//write error to buffer
			len += sprintf(*stringPtr + len, "\nerror while reading register %s", register_names[i]);
			return status;
		}
		//write register name and value to buffer
		len += sprintf(*stringPtr + len, "%s\t0x%x\n", register_names[i], data);
	}
	return status;
}

es_status_codes dumpTlpRegisters(uint32_t drvno, char** stringPtr)
{
	uint32_t data = 0;
	unsigned int len = 0;
	//allocate string buffer buffer
	*stringPtr = (char*)calloc(500, sizeof(char));
	len += sprintf(*stringPtr + len, "PAY_LOAD values:\t"DLLTAB"0 = 128 bytes\n\t"DLLTAB DLLTAB"1 = 256 bytes\n\t"DLLTAB DLLTAB"2 = 512 bytes\n");
	es_status_codes status = readConfig_32(drvno, &data, PCIeAddr_devCap);
	if (status != es_no_error)
	{
		len += sprintf(*stringPtr + len, "\nerror while reading register\n");
		return status;
	}
	data &= 0x7;
	len += sprintf(*stringPtr + len, "PAY_LOAD Supported:\t0x%x\n", data);
	status = readConfig_32(drvno, &data, PCIeAddr_devCap);
	if (status != es_no_error)
	{
		len += sprintf(*stringPtr + len, "\nerror while reading register\n");
		return status;
	}
	uint32_t actpayload = (data >> 5) & 0x07;
	len += sprintf(*stringPtr + len, "PAY_LOAD:\t"DLLTAB"0x%x\n", actpayload);
	data >>= 12;
	data &= 0x7;
	len += sprintf(*stringPtr + len, "MAX_READ_REQUEST_SIZE:\t0x%x\n", data);
	uint32_t pixel = 0;
	status = readRegisterS0_32(drvno, &pixel, S0Addr_PIXREGlow);
	pixel &= 0xFFFF;
	len += sprintf(*stringPtr + len, "Number of pixels:\t"DLLTAB"%u\n", pixel);
	switch (actpayload)
	{
	case 0: data = 0x20;  break;
	case 1: data = 0x40;  break;
	case 2: data = 0x80;  break;
	case 3: data = 0x100; break;
	}
	len += sprintf(*stringPtr + len, "TLP_SIZE is:\t"DLLTAB"%u DWORDs\n\t"DLLTAB DLLTAB"=%u BYTEs\n", data, data*4);
	status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPS);
	if (status != es_no_error)
	{
		len += sprintf(*stringPtr + len, "\nerror while reading register\n");
		return status;
	}
	len += sprintf(*stringPtr + len, "TLPS in DMAReg is:\t%u\n", data);
	if (data)
		data = (pixel - 1) / (data * 2) + 1;
	len += sprintf(*stringPtr + len, "number of TLPs should be:\t%u\n", data);
	status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPC);
	if (status != es_no_error)
	{
		len += sprintf(*stringPtr + len, "\nerror while reading register\n");
		return status;
	}
	len += sprintf(*stringPtr + len, "number of TLPs is:\t"DLLTAB"%u\n", data);
	return status;
}

es_status_codes dumpSettings(char** stringPtr)
{
	unsigned int len = 0;
	//allocate string buffer buffer
	*stringPtr = (char*)calloc(1000, sizeof(char));
	len += sprintf(*stringPtr + len,
		"unused\t%u\n"
		"nos\t%u\n"
		"nob\t%u\n"
		"sti_mode\t%u\n"
		"bti_mode\t%u\n"
		"stime in microseconds\t%u\n"
		"btime in microseconds\t%u\n"
		"sdat in 10 ns\t%u\n"
		"bdat in 10 ns\t%u\n"
		"sslope\t%u\n"
		"bslope\t%u\n"
		"xckdelay_in_10ns\t%u\n"
		"shutterExpTimeIn10ns\t%u\n"
		"trigger mode cc\t%u\n"
		"board sel\t%u\n"
		"sensor type\t%u\n"
		"camera system\t%u\n"
		"camcnt\t%u\n"
		"pixel\t%u\n"
		"mshut\t%u\n"
		"led off\t%u\n"
		"gain switch\t%u\n"
		"gain 3030\t%u\n"
		"temp level\t%u\n"
		"dac\t%u\n"
		"enable gpx\t%u\n"
		"gpx offset\t%u\n"
		"fftlines\t%u\n"
		"vfreq\t%u\n"
		"ffmode\t%u\n"
		"lines binning\t%u\n"
		"number of regions\t%u\n"
		"keep first\t%u\n",
		settings_struct.unused,
		settings_struct.nos,
		settings_struct.nob,
		settings_struct.sti_mode,
		settings_struct.bti_mode,
		settings_struct.stime_in_microsec,
		settings_struct.btime_in_microsec,
		settings_struct.sdat_in_10ns,
		settings_struct.bdat_in_10ns,
		settings_struct.sslope,
		settings_struct.bslope,
		settings_struct.xckdelay_in_10ns,
		settings_struct.ShutterExpTimeIn10ns,
		settings_struct.trigger_mode_cc,
		settings_struct.board_sel,
		settings_struct.sensor_type,
		settings_struct.camera_system,
		settings_struct.camcnt,
		settings_struct.pixel,
		settings_struct.mshut,
		settings_struct.led_off,
		settings_struct.gain_switch,
		settings_struct.gain_3030,
		settings_struct.Temp_level,
		settings_struct.dac,
		settings_struct.enable_gpx,
		settings_struct.gpx_offset,
		settings_struct.FFTLines,
		settings_struct.Vfreq,
		settings_struct.FFTMode,
		settings_struct.lines_binning,
		settings_struct.number_of_regions,
		settings_struct.keep_first);
	len += sprintf(*stringPtr + len, "region size\t");
	for (int i = 0; i < 8; i++)
		len += sprintf(*stringPtr + len, "%u ", settings_struct.region_size[i]);
	for (int drvno = 0; drvno < MAXPCIECARDS; drvno++)
	{
		len += sprintf(*stringPtr + len, "\ndac output board %i\t ", drvno);
		for (int i = 0; i < 8; i++)
			len += sprintf(*stringPtr + len, "%u ", settings_struct.dac_output[drvno][i]);
	}
	len += sprintf(*stringPtr + len,
		"\ntor modus\t%u\n"
		"adc mode\t%u\n"
		"adc custom pattern\t%u\n"
		"bec_in_10ns\t%u\n"
		"isIr\t%u\n",
		settings_struct.TORmodus,
		settings_struct.ADC_Mode,
		settings_struct.ADC_custom_pattern,
		settings_struct.bec_in_10ns,
		settings_struct.isIr);
	return es_no_error;
}

/**
 * \brief Sets BlockOn bit in PCIEFLAGS and notifies UI about it. Two board sync version
 *
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes setBlockOnTwoBoards()
{
	notifyBlockStart();

	uint32_t data1 = 0;
	uint32_t data2 = 0;

	es_status_codes status = readRegisterS0_32(1, &data1, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	data1 |= PCIEFLAGS_bit_BLOCKON;

	status = readRegisterS0_32(2, &data2, S0Addr_PCIEFLAGS);
	if (status != es_no_error) return status;
	data2 |= PCIEFLAGS_bit_BLOCKON;

	return writeRegisterS0_32twoBoards(data1, data2, S0Addr_PCIEFLAGS);
}

/**
 * \brief Sets Scan Timer on. Two board sync version.
 *
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes StartSTimerTwoBoards()
{
	ES_LOG("Start S Timer\n");

	uint8_t data1 = 0;
	uint8_t data2 = 0;

	es_status_codes status = readRegisterS0_8(1, &data1, S0Addr_XCKMSB);
	if (status != es_no_error) return status;
	data1 |= XCKMSB_bit_stimer_on;

	status = readRegisterS0_8(2, &data2, S0Addr_XCKMSB);
	if (status != es_no_error) return status;
	data2 |= XCKMSB_bit_stimer_on;

	return writeRegisterS0_8twoBoards(data1, data2, S0Addr_XCKMSB);
}

/**
 * \brief Triggers one camera read by calling this function.
 *
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes DoSoftwareTriggerTwoBoards()
{
	ES_LOG("Do software trigger\n");

	uint8_t data1 = 0;
	uint8_t data2 = 0;

	es_status_codes status = readRegisterS0_8(1, &data1, S0Addr_BTRIGREG);
	if (status != es_no_error) return status;
	data1 |= BTRIGREG_bit_SWTRIG;

	status = readRegisterS0_8(2, &data2, S0Addr_BTRIGREG);
	if (status != es_no_error) return status;
	data2 |= BTRIGREG_bit_SWTRIG;

	status = writeRegisterS0_8twoBoards(data1, data2, S0Addr_BTRIGREG);
	if (status != es_no_error) return status;
	data1 &= ~BTRIGREG_bit_SWTRIG;
	data2 &= ~BTRIGREG_bit_SWTRIG;
	return writeRegisterS0_8twoBoards(data1, data2, S0Addr_BTRIGREG);
}


/**
 * @brief reset Delay Stage Counter
 *
 * @param drvno PCIe board identifier
 * @param DSCNumber 1: DSC 1; 2: DSC 2; 3: DSC 3
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
	case 3: data = 0x10000; break;
	}
	//for reset you have to set a 1 to the reg and then a zero to allw a new start again
	status = writeBitsS0_32( drvno, data, data, S0Addr_DSCCtrl );
	if (status != es_no_error) return status;
	return writeBitsS0_32( drvno, 0, data, S0Addr_DSCCtrl );
}

/**
 * @brief set direction of Delay Stage Counter
 *
 * @param drvno PCIe board identifier
 * @param DSCNumber 1: DSC 1; 2: DSC 2; 3: DSC 3
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
	case 3: data = 0x20000; break;
	}

	if (dir)
		return writeBitsS0_32( drvno, data, data, S0Addr_DSCCtrl );
	else
		return writeBitsS0_32( drvno, 0, data, S0Addr_DSCCtrl );

}

/**
 * @brief return all values of Delay Stage Counter
 *
 * @param drvno PCIe board identifier
 * @param DSCNumber 1: DSC 1; 2: DSC 2; 3: DSC 3
 * @param ADSC actual DSC
 * @param LDSC last DSC
 * @return es_status_codes
 */
es_status_codes GetDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC )
{
	es_status_codes status;
	ES_LOG( "get DSC %u\n", DSCNumber );
	uint16_t addrADSC, addrLDSC;
	switch (DSCNumber)
	{
	case 1: addrADSC = S0Addr_A1DSC; addrLDSC = S0Addr_L1DSC; break;
	case 2: addrADSC = S0Addr_A2DSC; addrLDSC = S0Addr_L2DSC; break;
	case 3: addrADSC = S0Addr_A3DSC; addrLDSC = S0Addr_L3DSC; break;
	}

	status = readRegisterS0_32( drvno, ADSC, addrADSC );
	if (status != es_no_error) return status;
	return readRegisterS0_32( drvno, LDSC, addrLDSC );

}
