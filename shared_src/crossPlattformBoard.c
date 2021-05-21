#include "crossPlattformBoard.h"
#include "enum.h"
#include "es_status_codes.h"
#include <stdlib.h>

uint32_t tmp_aPIXEL[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
uint32_t* aPIXEL = tmp_aPIXEL;
uint32_t tmp_aCAMCNT[MAXPCIECARDS] = { 1, 1, 1, 1, 1 };	// cameras parallel
uint32_t* aCAMCNT = tmp_aCAMCNT;	// cameras parallel
bool useSWTrig_temp = false;
bool* useSWTrig = &useSWTrig_temp;
uint16_t* temp_userBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
uint16_t** userBuffer= temp_userBuffer;
uint32_t BOARD_SEL = 1;
uint32_t numberOfInterrupts = 0;
uint8_t number_of_boards = 0;
uint32_t Nob = 1;
uint32_t tmp_Nosbp = 1000;
uint32_t* Nospb = &tmp_Nosbp;
bool abortMeasurementFlag = false;

/**
 * \brief Initialize Measurement.
 * 
 * \param settings struct global_settings
 * \return es_status_codes
 *		- 
 */
es_status_codes InitMeasurement(struct global_settings settings)
{
	ES_LOG("\n*** Init Measurement ***\n");
	abortMeasurementFlag = false;
	ES_LOG("struct global_settings: ");
	for (int i = 0; i < sizeof(settings)/4; i++)
		ES_LOG("%x ", *(&settings.drvno + i));
	ES_LOG("\n");
	es_status_codes status = checkDriverHandle(settings.drvno);
	if (status != es_no_error) return status;
	BOARD_SEL = settings.board_sel;
	status = ClearAllUserRegs(settings.drvno);
	if (status != es_no_error) return status;
	status = SetPixelCount(settings.drvno, settings.pixel);
	if (status != es_no_error) return status;
	status = SetCamCount(settings.drvno, settings.camcnt);
	if (status != es_no_error) return status;
	//set PDA and FFT
	status = SetSensorType(settings.drvno, (uint8_t)settings.sensor_type);
	if (status != es_no_error) return status;
	if (settings.sensor_type == FFTsensor)
	{
		switch (settings.FFTMode)
		{
			case full_binning:
				status = SetupFullBinning(settings.drvno, settings.FFTLines, (uint8_t)settings.Vfreq);
				break;
			// TODO: implement partial binning and area
			/*
			case partial_binning:
			{
				UINT8 regionSize[8];
				for (int i = 0; i < 8; i++) regionSize[i] = settings.region_size[i];
				status = DLLSetupROI(settings.drvno, (UINT16)settings.number_of_regions, settings.FFTLines, (UINT8)settings.keep_first, regionSize, (UINT8)settings.Vfreq);
				break;
			}
			case area_mode:
				status = DLLSetupArea(settings.drvno, settings.lines_binning, (UINT8)settings.Vfreq);
				break;
			*/
			default:
				return es_parameter_out_of_range;
		}
	}
	else *useSWTrig = false;
	if (status != es_no_error) return status;
	//allocate Buffer
	status = SetMeasurementParameters(settings.drvno, settings.nos, settings.nob);
	if (status != es_no_error) return status;
	status = CloseShutter(settings.drvno); //set cooling  off
	if (status != es_no_error) return status;
	//set mshut
	if (settings.mshut)
	{
		status = SetSEC(settings.drvno, settings.ShutterExpTime * 100);
		if (status != es_no_error) return status;
		status = SetTORReg(settings.drvno, TOR_SSHUT);
		if (status != es_no_error) return status;
	}
	else
	{
		status = SetSEC(settings.drvno, 0);
		if (status != es_no_error) return status;
		status = SetTORReg(settings.drvno, (uint8_t)settings.TORmodus);
		if (status != es_no_error) return status;
	}
	//SSlope
	SetSSlope(settings.drvno, settings.sslope);
	if (status != es_no_error) return status;
	//BSlope
	status = SetBSlope(settings.drvno, settings.bslope);
	if (status != es_no_error) return status;
	//SetTimer
	status = SetSTI(settings.drvno, (uint8_t)settings.sti_mode);
	if (status != es_no_error) return status;
	status = SetBTI(settings.drvno, (uint8_t)settings.bti_mode);
	if (status != es_no_error) return status;
	status = SetSTimer(settings.drvno, settings.stime_in_microsec);
	if (status != es_no_error) return status;
	status = SetBTimer(settings.drvno, settings.btime_in_microsec);
	if (status != es_no_error) return status;
	if (settings.enable_gpx) status = InitGPX(settings.drvno, settings.gpx_offset);
	if (status != es_no_error) return status;
	//Delay after Trigger
	status = SetSDAT(settings.drvno, settings.sdat_in_100ns);
	if (status != es_no_error) return status;
	status = SetBDAT(settings.drvno, settings.bdat_in_100ns);
	if (status != es_no_error) return status;
	//init Camera
	status = InitCameraGeneral(settings.drvno, settings.pixel, settings.trigger_mode_cc, settings.sensor_type, 0, 0, settings.led_off);
	if (status != es_no_error) return status;
	switch (settings.camera_system)
	{
	case camera_system_3001:
		InitCamera3001(settings.drvno, settings.gain_switch);
		break;
	case camera_system_3010:
		InitCamera3010(settings.drvno, settings.ADC_Mode, settings.ADC_custom_pattern, settings.gain_switch);
		break;
	case camera_system_3030:
		InitCamera3030(settings.drvno, settings.ADC_Mode, settings.ADC_custom_pattern, settings.gain_3030);
		//TODO use DAC...maybe extra function or sendflcam_dac
		break;
	default:
		return es_parameter_out_of_range;
	}
	if (status != es_no_error) return status;
	//set gain switch
	status = SendFLCAM(settings.drvno, maddr_cam, 0, (uint16_t)settings.gain_switch);
	if (status != es_no_error) return status;
	//for cooled Cam
	status = SetTemp(settings.drvno, (uint8_t)settings.Temp_level);
	if (status != es_no_error) return status;
	//DAC
	//TODO: Move DAC to CAM 3030
	int dac_channel_count = 8;
	if (settings.dac) {
		SendFLCAM_DAC(settings.drvno, dac_channel_count, 0, 0, 1);
		int reorder_ch[8] = { 3, 4, 0, 5, 1, 6, 2, 7 };
		for (uint8_t channel = 0; channel < dac_channel_count; channel++)
		{
			status = DAC_setOutput(settings.drvno, channel, (uint16_t)settings.dac_output[reorder_ch[channel]]);
			if (status != es_no_error) return status;
		}
	}
	//DMA
	status = SetupDma(settings.drvno);
	if (status != es_no_error) return status;
	status = SetDmaStartMode(settings.drvno, HWDREQ_EN);
	if (status != es_no_error) return status;
	if(INTR_EN) status = enableInterrupt(settings.drvno);
	if (status != es_no_error) return status;
	status = SetDmaRegister(settings.drvno, settings.pixel);
	if (status != es_no_error) return status;
	//TODO set cont FF mode with DLL style(CONTFFLOOP = activate;//0 or 1;CONTPAUSE = pause;) or CCDExamp style(check it out)
	status = SetBEC( settings.drvno, settings.bec );
	if (status != es_no_error) return status;
	status = SetXckdelay(settings.drvno, settings.xckdelay);
	ES_LOG("*** Init Measurement done ***\n\n");
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
};

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
es_status_codes abortMeasurement( uint32_t drv )
{
	ES_LOG("Abort Measurement\n");
	abortMeasurementFlag = true;
	es_status_codes status = StopSTimer( drv );
	if (status != es_no_error) return status;
	status = resetBlockOn(drv);
	if (status != es_no_error) return status;
	status = resetMeasureOn(drv);
	if (status != es_no_error) return status;
	return SetDMAReset( drv );
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
	//notifyBlockDone( drvno );
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
	//notifyMeasureDone( drvno );
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
es_status_codes SetDMAReset( uint32_t drvno )
{
	/*
	ULONG BitMask = 0x1;
	ULONG RegisterValues = 0x1;
	es_status_codes status = SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, drvno);
	if (status != es_no_error)
	{
		ErrorMsg("switch on the Initiator Reset for the DMA failed");
		return status;
	}
	// DCSR: reset the Iniator Reset 
	RegisterValues = 0x0;
	status = SetDMAReg(RegisterValues, BitMask, DmaAddr_DCSR, drvno);
	if (status != es_no_error)
		ErrorMsg("switch off the Initiator Reset for the DMA failed");
	return status;
	*/
	return es_no_error;
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
	case 0:
        status = resetBitS0_8(drvno, TOR_MSB_bitindex_ISFFT, S0Addr_TOR_MSB);
		if (status != es_no_error) return status;
        status = setBitS0_8(drvno, TOR_MSB_bitindex_SENDRS, S0Addr_TOR_MSB);
		if (status != es_no_error) return status;
		status = OpenShutter(drvno);
		break;
	case 1:
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

es_status_codes writeRegisterS0_16( uint32_t drvno, uint16_t data, uint16_t address )
{
    return writeRegister_16(drvno, data, address + S0_SPACE_OFFSET);
}

es_status_codes writeRegisterS0_8( uint32_t drvno, uint8_t data, uint16_t address )
{
    return writeRegister_8(drvno, data, address + S0_SPACE_OFFSET);
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
	ES_LOG("Stop S Timer, drv: %u\n", drvno);
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
	int64_t needed_mem = (int64_t)aCAMCNT[drvno] * (int64_t)Nob * (int64_t)*Nospb * (int64_t)aPIXEL[drvno] * (int64_t)sizeof( uint16_t );
	int64_t needed_mem_mb = needed_mem / (1024 * 1024);
	ES_LOG( "Allocate user memory, available memory:%ld MB, memory needed: %ld MB\n", memory_free_mb, needed_mem_mb );
	//check if enough space is available in the physical ram
	if (memory_free > (uint64_t)needed_mem)
	{
		// sometimes it makes one ISR more, so better to allocate nos+1 thaT IN THIS CASE THE ADDRESS pDMAIndex is valid
		//B! "2 *" because the buffer is just 2/3 of the needed size. +1 oder *2 weil sonst absturz im continuous mode
		uint16_t* userBufferTemp = calloc( (uint64_t)aCAMCNT[drvno] * (uint64_t)(*Nospb) * (uint64_t)Nob * (uint64_t)aPIXEL[drvno], sizeof( uint16_t ) );
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
};

/**
 * \brief Exposure control (EC) signal is used for mechanical shutter or sensors with EC function.
 * 
 * Starts after delay after trigger (DAT) signal and is active for ecin100ns.
 * Resets additional delay after trigger with ecin100ns = 0.
 * \param drvno PCIe board identifier
 * \param ecin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetSEC( uint32_t drvno, uint32_t ecin100ns )
{
	ES_LOG("Set SEC. EC in 100 ns: %u\n", ecin100ns);
	es_status_codes status = es_no_error;
	if (ecin100ns <= 1)
		status = writeRegisterS0_32(drvno, 0, S0Addr_SEC);
	else
	{
		ecin100ns |= 0x80000000; // enable delay
		status = writeRegisterS0_32(drvno, ecin100ns, S0Addr_SEC);
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
	es_status_codes status = readRegisterS0_32( drvno, &data, S0Addr_XCKLL ); //reset
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
 * \param datin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetSDAT( uint32_t drvno, uint32_t datin100ns )
{
	ES_LOG("Set SDAT in 100ns: %u\n", datin100ns);
	if (datin100ns)
	{
		datin100ns |= 0x80000000; // enable delay
		return writeRegisterS0_32(drvno, datin100ns, S0Addr_SDAT);
	}
	else return writeRegisterS0_32(drvno, 0, S0Addr_SDAT);
};

/**
 * \brief Sets delay after trigger hardware register.
 * 
 * \param drvno PCIe board identifier.
 * \param datin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetBDAT( uint32_t drvno, uint32_t datin100ns )
{
	ES_LOG("Set BDAT in 100ns: %u\n", datin100ns);
	if (datin100ns)
	{
		datin100ns |= 0x80000000; // enable delay
		return writeRegisterS0_32(drvno, datin100ns, S0Addr_BDAT);
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
	//load val
	ldata |= 0x4000000;
	status = writeRegisterS0_32(drvno, ldata, S0Addr_DBR);
	if (status != es_no_error) return status;
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
	return SendFLCAM( drvno, maddr_adc, adc_ltc2271_regaddr_reset, adc_ltc2271_msg_reset );
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
es_status_codes InitCamera3030( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern, uint8_t gain )
{
	ES_LOG("Init camera 3030, adc_mode: %u, custom_pattern: %u, gain: %u\n", adc_mode, custom_pattern, gain);
	es_status_codes status = Cam3030_ADC_reset( drvno );
	if (status != es_no_error) return status;
	//two wire mode output interface for pal versions P209_2 and above
	status = Cam3030_ADC_twoWireModeEN( drvno );
	if (status != es_no_error) return status;
	status = Cam3030_ADC_SetGain( drvno, gain );
	if (status != es_no_error) return status;
	if (adc_mode)
		status = Cam3030_ADC_RampOrPattern( drvno, adc_mode, custom_pattern );
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
 * Starts after delay after trigger (DAT) signal and is active for ecin100ns.
 * Resets additional delay after trigger with ecin100ns = 0.
 * \param drvno PCIe board identifier
 * \param ecin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetBEC( uint32_t drvno, uint32_t ecin100ns )
{
	ES_LOG("Set BEC in 100 ns: %u\n", ecin100ns);
	es_status_codes status = es_no_error;
	if (ecin100ns)
	{
		ecin100ns |= 0x80000000; // enable delay
		status = writeRegister_32(drvno, ecin100ns, S0Addr_BEC);
	}
	else
		status = writeRegister_32(drvno, 0, S0Addr_BEC);
	return status;
};

/**
 * \brief Set XCK delay.
 * 
 * \param drvno PCIe board identifier.
 * \param xckdelay XCK delay
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetXckdelay(uint32_t drvno, uint32_t xckdelay)
{
	ES_LOG("Set XCK delay: %u\n", xckdelay);
	es_status_codes status = es_no_error;
	if (xckdelay)
	{
		xckdelay |= 0x80000000;
		status = writeRegister_32(drvno, xckdelay, S0Addr_XCKDLY);
	}
	else
		status = writeRegister_32(drvno, 0, S0Addr_XCKDLY);
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
	ES_LOG("Set WDMATLPA to physical address of dma buffer 0x%016lx\n", dma_physical_address)
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