/*****************************************************************//**
 * @file   Board.c
 * @copydoc Board.h
 *********************************************************************/

#include "Board.h"
#include "UIAbstractionLayer.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <math.h> // for sqrt()
#include <stdio.h>
#include <inttypes.h>
#ifdef __linux__
#define sprintf_s snprintf
#include <stdlib.h>
#include <string.h>
#endif
#include "../shared_src/default_settings.h"
#include "../version.h"

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
 * @brief Set global settings struct.
 *
 * Call this every time you changed settings before InitMeasurement.
 * @param[in] settings struct measurement_settings
 */
void SetGlobalSettings(struct measurement_settings settings)
{
	settings_struct = settings;
}

/**
 * @brief Initialize measurement (using board select).
 *
 * Call this every time you changed settings before starting the measurement. When you didn't change any settings, you can start the next measurement without calling InitMeasurement every time.
 * @return @ref es_status_codes
 */
es_status_codes InitMeasurement()
{
	ES_LOG("\n*** Init Measurement ***\n");
	ES_TRACE("struct global_settings: ");
	for (uint32_t i = 0; i < sizeof(settings_struct) / 4; i++)
		ES_TRACE("%"PRIu32" ", *(&settings_struct.board_sel + i));
	ES_TRACE("\n");
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

void SetVirtualCamcnt(uint32_t drvno)
{
	if (settings_struct.camera_settings[drvno].camcnt)
		virtualCamcnt[drvno] = settings_struct.camera_settings[drvno].camcnt;
	else
		// if camcnt = 0, treat as camcnt = 1, but write 0 to register
		virtualCamcnt[drvno] = 1;
	ES_LOG("Set virtual camcnt to %"PRIu32"\n", virtualCamcnt[drvno]);
	return;
}

es_status_codes InitSoftware(uint32_t drvno)
{
	ES_LOG("\nInit software for board %"PRIu32"\n", drvno);
	if (settings_struct.nos < 2 || settings_struct.nob < 1) return es_parameter_out_of_range;
	SetVirtualCamcnt(drvno);
	abortMeasurementFlag = false;
	es_status_codes status = allocateUserMemory(drvno);
	if (status != es_no_error) return status;
	if(settings_struct.camera_settings[drvno].dma_buffer_size_in_scans == 0)
		settings_struct.camera_settings[drvno].dma_buffer_size_in_scans = settingDmaBufferSizeInScansDefault;
	uint32_t dmaBufferPartSizeInScans = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS; //500
	if (dmaBufferPartSizeInScans)
		numberOfInterrupts[drvno] = (settings_struct.nob * settings_struct.nos * virtualCamcnt[drvno]) / dmaBufferPartSizeInScans;
	ES_LOG("Number of interrupts: %"PRIu32" \n", numberOfInterrupts[drvno]);
	if (testModeOn) return es_device_not_found;
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
	ES_LOG("\nInit hardware board %"PRIu32"\n", drvno);
	es_status_codes status = GetPcieCardVersion(drvno, &pcieCardMajorVersion[drvno], &pcieCardMinorVersion[drvno]);
	if (status != es_no_error) return status;
	status = DisarmScanTrigger(drvno);
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
			uint8_t regionSize[MAX_NUMBER_OF_REGIONS];
			for (int i = 0; i < MAX_NUMBER_OF_REGIONS; i++) regionSize[i] = (uint8_t)settings_struct.camera_settings[drvno].region_size[i];
			status = SetupROI(drvno, (uint16_t)settings_struct.camera_settings[drvno].number_of_regions, settings_struct.camera_settings[drvno].fft_lines, regionSize, (uint8_t)settings_struct.camera_settings[drvno].vfreq);
			break;
		}
		case area_mode:
			status = SetupArea(drvno, settings_struct.camera_settings[drvno].lines_binning, (uint8_t)settings_struct.camera_settings[drvno].vfreq);
			break;
		default:
			return es_parameter_out_of_range;
		}
	}
	if (status != es_no_error) return status;
	status = SetSSlope(drvno, settings_struct.camera_settings[drvno].sslope);
	if (status != es_no_error) return status;
	status = SetBSlope(drvno, settings_struct.camera_settings[drvno].bslope);
	if (status != es_no_error) return status;
	status = SetSTI(drvno, (uint8_t)settings_struct.camera_settings[drvno].sti_mode);
	if (status != es_no_error) return status;
	status = SetBTI(drvno, (uint8_t)settings_struct.camera_settings[drvno].bti_mode);
	if (status != es_no_error) return status;
	status = SetTimerResolution(drvno, (uint8_t)settings_struct.camera_settings[drvno].timer_resolution_mode);
	if (status != es_no_error) return status;
	status = SetSTimer(drvno, settings_struct.camera_settings[drvno].stime);
	if (status != es_no_error) return status;
	status = SetBTimer(drvno, settings_struct.camera_settings[drvno].btime);
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
	status = SetSticnt(drvno, (uint8_t)settings_struct.camera_settings[drvno].sticnt);
	if (status != es_no_error) return status;
	status = SetBticnt(drvno, (uint8_t)settings_struct.camera_settings[drvno].bticnt);
	if (status != es_no_error) return status;
	status = SetTocnt(drvno, (uint8_t)settings_struct.camera_settings[drvno].tocnt);
	if (status != es_no_error) return status;
	status = SetSEC(drvno, settings_struct.camera_settings[drvno].sec_in_10ns);
	if (status != es_no_error) return status;
	status = SetTORReg(drvno, (uint8_t)settings_struct.camera_settings[drvno].tor);
	if (status != es_no_error) return status;
	status = SetS1S2ReadDelay(drvno);
	if (status != es_no_error) return status;
	status = SetShiftS1S2ToNextScan(drvno);
	// when cooled camera legacy mode: disable PCIe FIFO when cool cam transmits cool status
	if (settings_struct.camera_settings[drvno].is_cooled_camera_legacy_mode)
		status = Use_ENFFW_protection(drvno, true);
	else
		status = Use_ENFFW_protection(drvno, false);
	return status;
}

/**
 * @brief Initialize Measurement (using drvno).
 *
 * @return @ref es_status_codes
 */
es_status_codes _InitMeasurement(uint32_t drvno)
{
	es_status_codes status = InitSoftware(drvno);
	if (status != es_no_error) return status;
	status = InitPcieBoard(drvno);
	if (status != es_no_error) return status;
	if (settings_struct.camera_settings[drvno].camcnt > 0)
		status = Cam_Init(drvno);
	return status;
}

/**
 * @brief Set pixel count
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes SetPixelCountRegister(uint32_t drvno)
{
	ES_LOG("Set pixel count: %"PRIu32"\n", settings_struct.camera_settings[drvno].pixel);
	return writeBitsS0_32(drvno, settings_struct.camera_settings[drvno].pixel, PIXREG_bits_pixel, S0Addr_PIXREG_FFCTRL_FFFLAGS);
}

/**
 * @brief Clears DAT and EC.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ClearAllUserRegs(uint32_t drvno)
{
	ES_LOG("Clear all user registers.\n");
	es_status_codes status = writeRegisterS0_32(drvno, 0, S0Addr_BDAT);
	if (status != es_no_error) return status;
	status = writeRegisterS0_32(drvno, 0, S0Addr_BEC);
	if (status != es_no_error) return status;
	status = writeRegisterS0_32(drvno, 0, S0Addr_SDAT);
	if (status != es_no_error) return status;
	return writeRegisterS0_32(drvno, 0, S0Addr_SEC);
}

/**
 * @brief Use this function to abort measurement.
 *
 * @return @ref es_status_codes
 */
es_status_codes AbortMeasurement()
{
	ES_LOG("Abort Measurement\n");
	abortMeasurementFlag = true;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		status = DisarmScanTrigger(drvno);
		if (status != es_no_error) return status;
		status = resetBlockEn(drvno);
		if (status != es_no_error) return status;
		status = resetMeasureOn(drvno);
		if (status != es_no_error) return status;
		status = ResetDma(drvno);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * @brief Sets abortMeasurementFlag to true.
 *
 * Use this function if the measurement is running and you want to stop it.
 *
 * @return @ref es_status_codes
 */
es_status_codes SetAbortMeasurementFlag()
{
	ES_LOG("Set abort measurement flag to true\n");
	abortMeasurementFlag = true;
	return es_no_error;
}

/**
 * @brief Sets BlockOn bit in PCIEFLAGS and notifies UI about it.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes setBlockEn(uint32_t drvno)
{
	ES_LOG("Set BLOCK_EN\n");
	notifyBlockStart();
	if(blockStartHook)
		blockStartHook();
	return setBitS0_32(drvno, PCIEFLAGS_bitindex_BLOCK_EN, S0Addr_PCIEFLAGS);
}

/**
 * @brief Sets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes setMeasureOn(uint32_t drvno)
{
	ES_LOG("Set measure on\n");
	notifyMeasureStart();
	if (measureStartHook)
		measureStartHook();
	return setBitS0_32(drvno, PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS);
}

/**
 * @brief Resets BlockOn bit in PCIEFLAGS and notifies UI about it.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes resetBlockEn(uint32_t drvno)
{
	ES_LOG("Reset BLOCK_EN\n");
	notifyBlockDone();
	if (blockDoneHook)
		blockDoneHook();
	return resetBitS0_32(drvno, PCIEFLAGS_bitindex_BLOCK_EN, S0Addr_PCIEFLAGS);
}

/**
 * @brief Resets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes resetMeasureOn(uint32_t drvno)
{
	ES_LOG("Reset measure on\n");
	notifyMeasureDone();
	if (measureDoneHook)
		measureDoneHook();
	return resetBitS0_32(drvno, PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS);
}

/**
 * @brief
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return @ref es_status_codes
 */
es_status_codes ResetDma(uint32_t drvno)
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
 * @brief Set cam count
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes SetCamCountRegister(uint32_t drvno)
{
	ES_LOG("Set cam count: %"PRIu32"\n", settings_struct.camera_settings[drvno].camcnt);
	return writeBitsS0_32(drvno, settings_struct.camera_settings[drvno].camcnt, CAMCNT_bits, S0Addr_CAMCNT);
}

/**
 * @brief Sets sensor type bits in register camera type
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param sensor_type Determines sensor type. See enum @ref sensor_type_t in enum_settings.h for options.
 * @return @ref es_status_codes
 */
es_status_codes SetSensorType(uint32_t drvno, uint16_t sensor_type)
{
	ES_LOG("Setting sensor type: %"PRIu16"\n", sensor_type);
	es_status_codes status;
	if (sensor_type == sensor_type_fft && settings_struct.camera_settings[drvno].is_fft_legacy)
		status = setBitS0_32(drvno, TOR_bitindex_ISFFT_LEGACY, S0Addr_TOR_STICNT_TOCNT);
	else
		status = resetBitS0_32(drvno, TOR_bitindex_ISFFT_LEGACY, S0Addr_TOR_STICNT_TOCNT);
	if (status != es_no_error) return status;
	uint32_t data = sensor_type << camera_type_sensor_type_bit_index;
	return writeBitsS0_32(drvno, data, camera_type_sensor_type_bits, S0Addr_CAMERA_TYPE);
}

/**
 * @brief Sets camera system bits in register camera type
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param camera_system Determines the camera system. See enum @ref camera_system_t in enum_settings.h for options.
 * @return @ref es_status_codes
 */
es_status_codes SetCameraSystem(uint32_t drvno, uint16_t camera_system)
{
	ES_LOG("Setting camera system: %"PRIu16"\n", camera_system);
	uint32_t data = camera_system << camera_type_camera_system_bit_index;
	return writeBitsS0_32(drvno, data, (uint32_t)camera_type_camera_system_bits, S0Addr_CAMERA_TYPE);
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 *
 * @param data 4 bytes (32 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFFFFFFFF - all bits 32 bits are written, 0 - no bits are written.
 * @param address Address of the register in S0 space.
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return @ref es_status_codes
 */
es_status_codes writeBitsS0_32(uint32_t drvno, uint32_t data, uint32_t bitmask, uint32_t address)
{
	return writeBitsDma_32(drvno, data, bitmask, address + S0_SPACE_OFFSET);
}

/**
 * @brief Set specified bits to 1 in S0 register at memory address.
 *
 * @param data 4 bytes (32 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFFFFFFFF - all bits 32 bits are written, 0 - no bits are written.
 * @param address Address of the register in S0 space.
 * @return @ref es_status_codes
 */
es_status_codes writeBitsS0_32_allBoards(uint32_t data, uint32_t bitmask, uint32_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
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
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return @ref es_status_codes
 */
es_status_codes writeBitsS0_8(uint32_t drvno, uint8_t data, uint8_t bitmask, uint32_t address)
{
	return writeBitsDma_8(drvno, data, bitmask, address + S0_SPACE_OFFSET);
}

/**
 * @brief Set bit to 1 in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return @ref es_status_codes
 */
es_status_codes setBitS0_32(uint32_t drvno, uint32_t bitnumber, uint32_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32(drvno, 0xFFFFFFFF, bitmask, address);
}

/**
 * @brief Set bit to 1 in S0 register at memory address.
 *
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return @ref es_status_codes
 */
es_status_codes setBitS0_32_allBoards(uint32_t bitnumber, uint32_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32_allBoards(0xFFFFFFFF, bitmask, address);
}

/**
 * @brief Set bit to 1 in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...7, 0 is LSB, 7 MSB
 * @param address register address. 1 byte steps are valid.
 * @return @ref es_status_codes
 */
es_status_codes setBitS0_8(uint32_t drvno, uint32_t bitnumber, uint32_t address)
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
 * @return @ref es_status_codes
 */
es_status_codes resetBitS0_32(uint32_t drvno, uint32_t bitnumber, uint32_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32(drvno, 0x0, bitmask, address);
}

/**
 * @brief Set bit to 0 in register at memory address.
 *
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return @ref es_status_codes
 */
es_status_codes resetBitS0_32_allBoards(uint32_t bitnumber, uint32_t address)
{
	uint32_t bitmask = 0x1 << bitnumber;
	return writeBitsS0_32_allBoards(0x0, bitmask, address);
}

/**
 * @brief Set bit to 0 in register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...7, 0 is LSB, 7 MSB
 * @param address register address. 1 byte steps are valid.
 * @return @ref es_status_codes
 */
es_status_codes resetBitS0_8(uint32_t drvno, uint32_t bitnumber, uint32_t address)
{
	uint8_t bitmask = (uint8_t)(0x1 << bitnumber);
	return writeBitsS0_8(drvno, 0x0, bitmask, address);
}

/**
 * @brief Write 4 byte of a register in S0 space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Data to write.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterS0_32(uint32_t drvno, uint32_t data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = writeRegister_32(drvno, data, address + S0_SPACE_OFFSET);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Write 4 bytes of a register in S0 space.
 *
 * @param data Data to write.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterS0_32_allBoards(uint32_t data, uint32_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = writeRegisterS0_32(drvno, data, address);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * @brief Write 2 bytes of a register in S0 space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Data to write.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterS0_16(uint32_t drvno, uint16_t data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = writeRegister_16(drvno, data, address + S0_SPACE_OFFSET);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Write 1 byte of a register in S0 space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Data to write.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterS0_8(uint32_t drvno, uint8_t data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = writeRegister_8(drvno, data, address + S0_SPACE_OFFSET);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Write the same 1 byte to a register in S0 space of all boards.
 *
 * @param data Data to write.
 * @param address Address of the register to write.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterS0_8_allBoards(uint8_t data, uint32_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = writeRegisterS0_8(drvno, data, address);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * @brief Read 4 bytes of a register in S0 space.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] data Read buffer.
 * @param[in] address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes readRegisterS0_32(uint32_t drvno, uint32_t* data, uint32_t address)
{
	return readRegister_32(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * @brief Read 4 bytes of a register in S0 space of all boards.
 *
 * @param data Read buffer.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes readRegisterS0_32_allBoards(uint32_t** data, uint32_t address)
{
	return readRegister_32_allBoards(data, address + S0_SPACE_OFFSET);
}

/**
 * @brief Reads 4 bytes on DMA area of all PCIe boards.
 *
 * @param data buffer array for data
 * @param address Offset from BaseAdress - in Bytes ! 0..3= Regs of Board.
 * @return @ref es_status_codes
 */
es_status_codes readRegister_32_allBoards(uint32_t** data, uint32_t address)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = readRegister_32(drvno, *(data + drvno), address);
			if (status != es_no_error) return status;
		}
	}
	return status;
};

/**
 * @brief Read 2 bytes of a register in S0 space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Read buffer.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes readRegisterS0_16(uint32_t drvno, uint16_t* data, uint32_t address)
{
	return readRegister_16(drvno, data, address + S0_SPACE_OFFSET);
}

/**
 * @brief Read 1 byte of a register in S0 space.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] data Read buffer.
 * @param[in] address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes readRegisterS0_8(uint32_t drvno, uint8_t* data, uint32_t address)
{
	return readRegister_8(drvno, data, address + S0_SPACE_OFFSET);
}
/**
 * @brief Read 1 bit of a 4 byte s0 register.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] address Address of the register to read.
 * @param[in] bitnumber Address of the bit to read.
 * @param[out] isBitHigh Tells if bit is high or low.
 * @return @ref es_status_codes
 */
es_status_codes ReadBitS0_32(uint32_t drvno, uint32_t address, uint8_t bitnumber, bool* isBitHigh)
{
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, address);
	if (data & (1 << bitnumber)) *isBitHigh = true;
	else *isBitHigh = false;

	return status;
}

/**
 * @brief Read 1 bit of 1 byte of a s0 register.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] address Address of the register to read.
 * @param[in] bitnumber Address of the bit to read.
 * @param[out] isBitHigh Tells if bit is high or low.
 * @return @ref es_status_codes
 */
es_status_codes ReadBitS0_8(uint32_t drvno, uint32_t address, uint8_t bitnumber, bool* isBitHigh)
{
	uint8_t data = 0;
	es_status_codes status = readRegisterS0_8(drvno, &data, address);

	if (data & (1 << bitnumber)) *isBitHigh = true;
	else *isBitHigh = false;

	return status;
}

/**
 * @brief Open shutter for sensors with EC (exposure control) / sets IFC signal = high.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes OpenShutter(uint32_t drvno)
{
	ES_LOG("Open shutter MSHUT, drvno %"PRIu32"\n", drvno);
	return setBitS0_32(drvno, CTRL_bitindex_SHON, S0Addr_CTRL);
}

/**
 * @brief For FFTs: Setup full binning.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param lines Lines in camera.
 * @param vfreq Frequency for vertical clock.
 * @return @ref es_status_codes
 */
es_status_codes SetupFullBinning(uint32_t drvno, uint32_t lines, uint8_t vfreq)
{
	ES_LOG("Setup full binning\n");
	es_status_codes status = SetupVCLKReg(drvno, lines, vfreq);
	if (status != es_no_error) return status;
	return ResetPartialBinning(drvno);
}

/**
 * @brief Set REG VCLKCTRL for FFT sensors.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param lines number of vertical lines
 * @param vfreq vertical clock frequency
 * @return @ref es_status_codes
 */
es_status_codes SetupVCLKReg(uint32_t drvno, uint32_t lines, uint8_t vfreq)
{
	ES_LOG("Setup VCLK register. drvno: %"PRIu32", lines: %"PRIu32", vfreq: %"PRIu8"\n", drvno, lines, vfreq);
	return writeRegisterS0_32(drvno, (((uint32_t)vfreq) << VCLKFREQ_bitindex) & lines * 2, S0Addr_VCLKCTRL_VCLKFREQ); // write no of vclks=2*lines
}

/**
 * @brief sets Vertical Partial Binning in registers R10,R11 and R12. Only for FFT sensors.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param range Determines the range, for which the value lines will be written.
 *		* min = 1
 *		* step = 1
 *		* max = 5
 * @param lines number of vertical clocks for next read
 *		* min = 1
 *		* step = 1
 *		* max = 2047
 * @return @ref es_status_codes
 */
es_status_codes SetupVPB(uint32_t drvno, uint32_t range, uint32_t lines)
{
	ES_LOG("SetupVPB, range: 0x%x, lines: 0x%x\n", range, lines);
	uint16_t adr = 0;
	int shift = 0;
	uint32_t bits = 0;
	uint32_t keep = 0;
	lines *= 2; //vclks=lines*2
	switch (range)
	{
	case 1:
		adr = S0Addr_ROI0;
		shift = ROI0_bitindex_range1;
		bits = ROI0_bits_range1;
		keep = ROI0_bit_range1_keep;
		break;
	case 2:
		adr = S0Addr_ROI0;
		shift = ROI0_bitindex_range2;
		bits = ROI0_bits_range2;
		keep = (uint32_t)ROI0_bit_range2_keep;
		break;
	case 3:
		adr = S0Addr_ROI1;
		shift = ROI1_bitindex_range3;
		bits = ROI1_bits_range3;
		keep = ROI1_bit_range3_keep;
		break;
	case 4:
		adr = S0Addr_ROI1;
		shift = ROI1_bitindex_range4;
		bits = ROI1_bits_range4;
		keep = (uint32_t)ROI1_bit_range4_keep;
		break;
	case 5:
		adr = S0Addr_ROI2;
		shift = ROI2_bitindex_range5;
		bits = ROI2_bits_range5;
		keep = ROI2_bit_range5_keep;
		break;
	default:
		return es_parameter_out_of_range;
	}
	// keep is a deprecated setting. The idea was to get faster by throwing away data. This is in fact not possible, because the data has be read out anyway. Thus throwing away data has no advantage. The keep bit is always set to 1.
	uint32_t data = ((lines << shift) & bits) | keep;
	return writeBitsS0_32(drvno, data, bits | keep, adr);
}

/**
 * @brief Turn partial binning on.
 *
 * @param drvno  PCIe board identifier.
 * @param number_of_regions number of regions for partial binning
 * @return @ref es_status_codes
 */
es_status_codes SetPartialBinning(uint32_t drvno, uint16_t number_of_regions)
{
	ES_LOG("Set partial binning\n");
	es_status_codes status = writeRegisterS0_32(drvno, number_of_regions, S0Addr_ARREG);
	if (status != es_no_error) return status;
	return setBitS0_32(drvno, 15, S0Addr_ARREG);//this turns ARREG on and therefore partial binning too
}

/**
 * @brief Turns
 * ARREG off and therefore partial binning too.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ResetPartialBinning(uint32_t drvno)
{
	ES_LOG("Reset partial binning\n");
	return resetBitS0_32(drvno, 15, S0Addr_ARREG);
}

/**
 * @brief Disarm scan trigger.
 *
 * Clear Bit30 of XCK-Reg: 0= timer off
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes DisarmScanTrigger(uint32_t drvno)
{
	if (PcieCardVersionIsSmallerThan(drvno, 0x222, 0x18))
	{
		ES_LOG("Disarm scan trigger\n");
		return resetBitS0_32(drvno, XCK_bitindex_arm_scan_trigger, S0Addr_XCK);
	}
	else
		// Since version 222_18 XCK_bitindex_arm_scan_trigger is read only.
		return es_no_error;
}

/**
 * @brief reset FIFO and FFcounter
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes RSFifo(uint32_t drvno)
{
	ES_LOG("Reset FIFO\n");
	es_status_codes status = setBitS0_32(drvno, FFCTRL_bitindex_RSFIFO, S0Addr_PIXREG_FFCTRL_FFFLAGS);
	if (status != es_no_error) return status;
	return resetBitS0_32(drvno, FFCTRL_bitindex_RSFIFO, S0Addr_PIXREG_FFCTRL_FFFLAGS);
}

/**
 * Allocate user memory.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes allocateUserMemory(uint32_t drvno)
{
	//free old memory before allocating new one
	free(userBuffer[drvno]);
	userBuffer[drvno] = NULL;
	uint64_t memory_all = 0;
	uint64_t memory_free = 0;
	FreeMemInfo(&memory_all, &memory_free);
	uint64_t memory_free_mb = memory_free / (1024 * 1024);
	uint64_t needed_mem = (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.nob * (uint64_t)settings_struct.nos * (uint64_t)settings_struct.camera_settings[drvno].pixel * (uint64_t)sizeof(uint16_t);
	uint64_t needed_mem_mb = needed_mem / (1024 * 1024);
	ES_LOG("Allocate user memory, available memory:%"PRIu64" MB, memory needed: %"PRIu64" MB (%"PRIu64")\n", memory_free_mb, needed_mem_mb, needed_mem);
	// turn off warning that variables are not used
	(void)needed_mem_mb;
	(void)memory_free_mb;
	//check if enough space is available in the physical ram
	if (memory_free > needed_mem)
	{
		uint16_t* userBufferTemp = (uint16_t*)calloc(needed_mem, 1);
		if (userBufferTemp)
		{
			userBufferEndPtr[drvno] = userBufferTemp + needed_mem / sizeof(uint16_t);
			userBuffer[drvno] = userBufferTemp;
			userBufferWritePos[drvno] = userBufferTemp;
			userBufferWritePos_last[drvno] = userBufferTemp;
			ES_TRACE("user buffer space: %p - %p\n", userBuffer[drvno], userBufferEndPtr[drvno]);
			return es_no_error;
		}
		else
		{
			ES_LOG("Allocating user memory failed.\n");
			return es_allocating_memory_failed;
		}
	}
	else
	{
		ES_LOG("ERROR for buffer %"PRIu8": available memory: %"PRIu64" MB \n \tmemory needed: %"PRIu64" MB\n", number_of_boards, memory_free_mb, needed_mem_mb);
		return es_not_enough_ram;
	}
}

/**
 * @brief Set DMA register
 *
 * Sets DMA_BUFFER_SIZE_IN_SCANS, DMA_DMASPERINTR, NOS, NOB, CAMCNT
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes SetDMABufRegs(uint32_t drvno)
{
	ES_LOG("Set DMA buffer registers, ");
	//DMABufSizeInScans - use 1 block
	es_status_codes status = writeRegisterS0_32(drvno, settings_struct.camera_settings[drvno].dma_buffer_size_in_scans, S0Addr_DmaBufSizeInScans);
	if (status != es_no_error) return status;
	//scans per interrupt must be 2x per DMA_BUFFER_SIZE_IN_SCANS to copy hi/lo part
	//virtualCamcnt: double the INTR if 2 cams
	uint32_t dmasPerInterrupt = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS;
	status = writeRegisterS0_32(drvno, dmasPerInterrupt, S0Addr_DMAsPerIntr);
	if (status != es_no_error) return status;
	ES_LOG("scansPerInterrupt/camcnt: %"PRIu32" \n", dmasPerInterrupt / virtualCamcnt[drvno]);
	return status;
}

es_status_codes SetNosRegister(uint32_t drvno)
{
	ES_LOG("Set NOS register to %"PRIu32"\n", settings_struct.nos);
	return writeRegisterS0_32(drvno, settings_struct.nos, S0Addr_NOS);
}

es_status_codes SetNobRegister(uint32_t drvno)
{
	ES_LOG("Set NOB register to %"PRIu32"\n", settings_struct.nob);
	return writeRegisterS0_32(drvno, settings_struct.nob, S0Addr_NOB);
}

/**
 * @brief Sets the IFC bit of interface for sensors with shutter function. IFC=low
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes CloseShutter(uint32_t drvno)
{
	ES_LOG("Close shutter MSHUT, drvno %"PRIu32"\n", drvno);;
	return resetBitS0_32(drvno, CTRL_bitindex_SHON, S0Addr_CTRL);
}

/**
 * @brief Exposure control (EC) signal is used for mechanical shutter or sensors with EC function.
 *
 * Starts after delay after trigger (DAT) signal and is active for ecin10ns.
 * Resets additional delay after trigger with ecin10ns = 0.
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param ecin10ns Time in 10 ns steps.
 * @return @ref es_status_codes
 */
es_status_codes SetSEC(uint32_t drvno, uint32_t ecin10ns)
{
	ES_LOG("Set SEC. EC in 10 ns: %"PRIu32"\n", ecin10ns);
	return writeRegisterS0_32(drvno, ecin10ns, S0Addr_SEC);
}

/**
 * @brief Set signal of output port of PCIe card.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] tor select output signal. See enum tor_out in enum.h for options.
 * @return @ref es_status_codes
 */
es_status_codes SetTORReg(uint32_t drvno, uint8_t tor)
{
	ES_LOG("Set TOR: %"PRIu8"\n", tor);
	// TOR register layout:
	// bit		31	30	29	28	27
	// meaning	TO3	TO2	TO1	TO0	TOSELG
	// use lower 4 bits of input tor for the upper nibble TO0 - TO3
	uint32_t tor_upper_nibble = tor << TOR_bitindex_TO0;
	// use bit 5 of input tor for bit TOSELG
	uint32_t toselg = tor & 0x10;
	toselg = toselg >> 4;
	// shift the bit to the correct position
	toselg = toselg << TOR_bitindex_TOSEL;
	uint32_t data = tor_upper_nibble | toselg;
	return writeBitsS0_32(drvno, data, (uint32_t)TOR_BITS_TO, S0Addr_TOR_STICNT_TOCNT);
}

/**
 * @brief Set the external trigger slope for scan trigger (PCI Reg CrtlA:D5 -> manual).
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param sslope Choose slope:
 *		- 0: high slope
 *		- 1: low slope
 *		- 2: both slope
 * @return @ref es_status_codes
 */
es_status_codes SetSSlope(uint32_t drvno, uint32_t sslope)
{
	ES_LOG("Set scan slope, %"PRIu32"\n", sslope);
	es_status_codes status = es_no_error;
	switch (sslope)
	{
		// high slope
	case sslope_pos:
		status = setBitS0_32(drvno, CTRL_bitindex_SLOPE, S0Addr_CTRL);
		if (status != es_no_error) return status;
		status = resetBitS0_32(drvno, CTRL_bitindex_BOTH_SLOPE, S0Addr_CTRL);
		break;
		// low slope
	case sslope_neg:
		status = resetBitS0_32(drvno, CTRL_bitindex_SLOPE, S0Addr_CTRL);
		if (status != es_no_error) return status;
		status = resetBitS0_32(drvno, CTRL_bitindex_BOTH_SLOPE, S0Addr_CTRL);
		break;
		// both slope
	case sslope_both:
		status = setBitS0_32(drvno, CTRL_bitindex_SLOPE, S0Addr_CTRL);
		if (status != es_no_error) return status;
		status = setBitS0_32(drvno, CTRL_bitindex_BOTH_SLOPE, S0Addr_CTRL);
		break;
	default:
		return es_parameter_out_of_range;
	}
	return status;
}

/**
 * @brief Sets slope for block trigger.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param slope:
 *		- 0: negative slope
 *		- 1: positive slope
 *		- 2: both
 * @return @ref es_status_codes
 */
es_status_codes SetBSlope(uint32_t drvno, uint32_t slope)
{
	ES_LOG("Set BSlope: %"PRIu32"\n", slope);
	return writeRegisterS0_32(drvno, slope, S0Addr_BSLOPE);
}

/**
* @brief Chooses trigger input for scan trigger input (STI)
* @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
* @param sti_mode Defines the input mode for STI.
* 	- 0: I
* 	- 1: S1
* 	- 2: S2
* 	- 3: unused
* 	- 4: S Timer
* 	- 5: ASL
* @return @ref es_status_codes
*/
es_status_codes SetSTI(uint32_t drvno, uint8_t sti_mode)
{
	ES_LOG("Set STI: %"PRIu8"\n", sti_mode);
	return writeBitsS0_32(drvno, ((uint32_t)sti_mode) << CTRL_bitindex_STI0, CTRL_bit_STI0 | CTRL_bit_STI1 | CTRL_bit_STI2, S0Addr_CTRL);
}

/**
* @brief Chooses trigger input for block trigger input (BTI)
* @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
* @param bti_mode Defines the input mode for BTI.
* 	- 0: I
* 	- 1: S1
* 	- 2: S2
* 	- 3: S1&s2
* 	- 4: BTIMER
*	- 5: S1 chopper
*	- 6: S2 chopper
*	- 7: S1&S2 chopper
* @return @ref es_status_codes
*/
es_status_codes SetBTI(uint32_t drvno, uint8_t bti_mode)
{
	ES_LOG("Set BTI: %"PRIu8"\n", bti_mode);
	return writeBitsS0_32(drvno, ((uint32_t)bti_mode) << CTRL_bitindex_BTI0, CTRL_bit_BTI0 | CTRL_bit_BTI1 | CTRL_bit_BTI2, S0Addr_CTRL);
}

/**
 * @brief Set timer resolution.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param resolution_mode Resolution of the time. See @ref timer_resolution_t in enum.h for options.
 * @return @ref es_status_codes
 */
es_status_codes SetTimerResolution(uint32_t drvno, uint8_t resolution_mode)
{
	es_status_codes status = es_no_error;
	switch (resolution_mode)
	{
	case timer_resolution_1us:
		status = resetBitS0_32(drvno, XCK_bitindex_res_ns, S0Addr_XCK);
		if (status != es_no_error) return status;
		status = resetBitS0_32(drvno, XCK_bitindex_res_ms, S0Addr_XCK);
		if (status != es_no_error) return status;
		break;
	case timer_resolution_100us:
		status = setBitS0_32(drvno, XCK_bitindex_res_ns, S0Addr_XCK);
		if (status != es_no_error) return status;
		status = setBitS0_32(drvno, XCK_bitindex_res_ms, S0Addr_XCK);
		if (status != es_no_error) return status;
		break;
	case timer_resolution_1ms:
		status = resetBitS0_32(drvno, XCK_bitindex_res_ns, S0Addr_XCK);
		if (status != es_no_error) return status;
		status = setBitS0_32(drvno, XCK_bitindex_res_ms, S0Addr_XCK);
		if (status != es_no_error) return status;
		break;
	case timer_resolution_100ns:
		status = setBitS0_32(drvno, XCK_bitindex_res_ns, S0Addr_XCK);
		if (status != es_no_error) return status;
		status = resetBitS0_32(drvno, XCK_bitindex_res_ms, S0Addr_XCK);
		if (status != es_no_error) return status;
		break;
	default:
		return es_parameter_out_of_range;
	}
	return status;
}

/**
 * @brief Sets time for scan timer.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] stime Trigger time. The resolution of the time depends on the resolution mode set by SetTimerResolution(). 28 bit.
 * @return @ref es_status_codes
 */
es_status_codes SetSTimer(uint32_t drvno, uint32_t stime)
{
	ES_LOG("Set stime in microseconds: %"PRIu32"\n", stime);
	return writeBitsS0_32(drvno, stime, XCK_bits_stimer, S0Addr_XCK);
}

/**
 * @brief Sets time for block timer.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] btime Block time. The resolution of the time depends on the resolution mode set by SetTimerResolution(). 28 bit.
 * @return @ref es_status_codes
 */
es_status_codes SetBTimer(uint32_t drvno, uint32_t btime)
{
	ES_LOG("Set btime in microseconds: %"PRIu32"\n", btime);
	if (btime)
	{
		uint32_t data = btime | 0x80000000;
		return writeRegisterS0_32(drvno, data, S0Addr_BTIMER);
	}
	else
		return writeRegisterS0_32(drvno, 0, S0Addr_BTIMER);
}

/**
 * @brief Initialize the TDC-GPX chip. TDC: time delay counter option.
 *
 * More information: https://www.sciosense.com/wp-content/uploads/2023/12/TDC-GPX-Datasheet.pdf
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param delay GPX offset is used to increase accuracy. A counter value can be added, usually 1000. 18 bit.
 * @return @ref es_status_codes
 */
es_status_codes InitGPX(uint32_t drvno, uint32_t delay)
{
	ES_LOG("Init GPX, delay: %"PRIu32"\n", delay);
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
	es_status_codes status = readRegisterS0_32(drvno, &regData, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	regData |= TDCCtrl_bit_reset;
	status = writeRegisterS0_32(drvno, regData, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	regData &= ~TDCCtrl_bit_reset;
	status = writeRegisterS0_32(drvno, regData, S0Addr_TDCCtrl); //reset bit
	if (status != es_no_error) return status;
	//setup R mode -> time between start and stop
	status = SetGPXCtrl(drvno, 5, regVal); // write to reg5: 82000000 retrigger, disable after start-> reduce to 1 val
	if (status != es_no_error) return status;
	for (int write_reg = 0; write_reg < 12; write_reg++)
	{
		status = SetGPXCtrl(drvno, (uint8_t)RegData[write_reg][0], RegData[write_reg][1]);//write
		if (status != es_no_error) return status;
		status = ReadGPXCtrl(drvno, (uint8_t)RegData[write_reg][0], &regData);//read
		if (RegData[write_reg][1] != regData) err_cnt++;//compare write data with read data
	}
	return status;
}

/**
 * @brief Set GPXCtrl register.
 *
 * @param drvno select PCIe board
 * @param GPXAddress address to access
 * @param GPXData data to write
 * @return @ref es_status_codes
 */
es_status_codes SetGPXCtrl(uint32_t drvno, uint8_t GPXAddress, uint32_t GPXData)
{
	uint32_t data = 0;
	//shift gpx addr to the right place for the gpx ctrl reg
	data = (uint32_t)(GPXAddress << TDCCtrl_bitindex_adr0);
	uint32_t bitmask = TDCCtrl_bit_cs | TDCCtrl_bit_adr0 | TDCCtrl_bit_adr1 | TDCCtrl_bit_adr2 | (uint32_t)TDCCtrl_bit_adr3;
	es_status_codes status = writeBitsS0_32(drvno, data, bitmask, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return writeRegisterS0_32(drvno, GPXData, S0Addr_TDCData);
}

/**
 * @brief Read GPXCtrl register.
 *
 * @param drvno select PCIe board
 * @param GPXAddress address to access
 * @param GPXData pointer where read data is written to
 * @return @ref es_status_codes
 */
es_status_codes ReadGPXCtrl(uint32_t drvno, uint8_t GPXAddress, uint32_t* GPXData)
{
	uint32_t data = 0;
	//shift gpx addr to the right place for the gpx ctrl reg
	data = (uint32_t)GPXAddress << TDCCtrl_bitindex_adr0;
	//set CSexpand bit set CS Bit
	data |= TDCCtrl_bit_cs;
	uint32_t bitmask = TDCCtrl_bit_cs | TDCCtrl_bit_adr0 | TDCCtrl_bit_adr1 | TDCCtrl_bit_adr2 | (uint32_t)TDCCtrl_bit_adr3;
	es_status_codes status = writeBitsS0_32(drvno, data, bitmask, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return readRegisterS0_32(drvno, GPXData, S0Addr_TDCData);
}

/**
 * @brief Sets delay after trigger hardware register.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param datin10ns Time in 10 ns steps. 31 bit.
 *		* disable: 0
 *		* min: 1 * 10 ns = 10 ns.
 *		* max: 2.147.483.647 * 10 ns = 21.474.836.470 ns = 21,474836470 s
 * @return @ref es_status_codes
 */
es_status_codes SetSDAT(uint32_t drvno, uint32_t datin10ns)
{
	ES_LOG("Set SDAT in 10ns: %"PRIu32"\n", datin10ns);
	if (datin10ns)
	{
		datin10ns |= SDAT_bit_enable; // enable delay
		return writeRegisterS0_32(drvno, datin10ns, S0Addr_SDAT);
	}
	else return writeRegisterS0_32(drvno, 0, S0Addr_SDAT);
}

/**
 * @brief Sets delay after trigger hardware register.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param datin10ns Time in 10 ns steps. 31 bit.
 *		* disable: 0
 *		* min: 1 * 10 ns = 10 ns.
 *		* max: 2.147.483.647 * 10 ns = 21.474.836.470 ns = 21,474836470 s
 * @return @ref es_status_codes
 */
es_status_codes SetBDAT(uint32_t drvno, uint32_t datin10ns)
{
	ES_LOG("Set BDAT in 10ns: %"PRIu32"\n", datin10ns);
	if (datin10ns)
	{
		datin10ns |= 0x80000000; // enable delay
		return writeRegisterS0_32(drvno, datin10ns, S0Addr_BDAT);
	}
	else return writeRegisterS0_32(drvno, 0, S0Addr_BDAT);
}

/**
 * @brief Protects ENFFW from cool cam status transmission. Enable with cool cam, disable with HS > 50 kHz.
 *
 * Legacy code for old cameras.
 * RX_VALID usually triggers ENFFW. This must be disabled when cool cams transmit their cooling status.
 * RX_VALID_EN is enabled with XCKI and disabled with ~CAMFFXCK_ALL, after all frame data is collected.
 * If RX_VALID raises again for cool status values, it doesn't effect ENFFW when RX_VALID_EN is low.
 * @param drvno selects PCIe board
 * @param USE_ENFFW_PROTECT enables or disables RX_VALID write protection
 * @return @ref es_status_codes
 */
es_status_codes Use_ENFFW_protection(uint32_t drvno, bool USE_ENFFW_PROTECT)
{
	if (USE_ENFFW_PROTECT)
		return setBitS0_32(drvno, PCIEFLAGS_bitindex_USE_ENFFW_PROTECT, S0Addr_PCIEFLAGS);
	else
		return resetBitS0_32(drvno, PCIEFLAGS_bitindex_USE_ENFFW_PROTECT, S0Addr_PCIEFLAGS);
}

/**
 * @brief Sends data to DAC8568.
 *
 * Mapping of bits in DAC8568: 4 prefix, 4 control, 4 address, 16 data, 4 feature.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param location Switch for the different locations of DAC85689. See enum @ref DAC8568_location_t in enum_settings.h for details.
 * @param cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * @param ctrlBits 4 control bits
 * @param addrBits 4 address bits
 * @param dataBits 16 data bits
 * @param featureBits 4 feature bits
 * @return @ref es_status_codes
 */
es_status_codes DAC8568_sendData(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t ctrlBits, uint8_t addrBits, uint16_t dataBits, uint8_t featureBits)
{
	uint32_t data = 0;
	es_status_codes status;
	if (ctrlBits & 0xF0) //4 ctrl bits => only lower 4 bits allowed
	{
		ES_LOG("DAC8568_sendData: Only values between 0 and 15 are allowed for control bits.");
		return es_parameter_out_of_range;
	}
	if (addrBits & 0xF0) //4 addr bits => only lower 4 bits allowed
	{
		ES_LOG("DAC8568_sendData: Only values between 0 and 15 are allowed for address bits.");
		return es_parameter_out_of_range;
	}
	if (featureBits & 0xF0) //4 ctrl bits => only lower 4 bits allowed
	{
		ES_LOG("DAC8568_sendData: Only values between 0 and 15 are allowed for feature bits.");
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
		status = Cam_DAC8568_sendData(drvno, data, cameraPosition);
		if (status != es_no_error) return status;
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
 * @brief Sets all outputs of the DAC8568 in camera 3030 or on PCIe board.
 *
 * Use this function to set the outputs, because it is resorting the channel numeration correctly.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] location Switch for the different locations of DAC85689. See enum @ref DAC8568_location_t in enum.h for details.
 * @param[in] cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * @param[in] output all output values that will be converted to analog voltage (0 ... 0xFFFF)
 * @param[in] reorder_channels used to reorder DAC channels for high speed camera
 * @return @ref es_status_codes
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
 * @brief Sets the output of the DAC8568 on PCB 2189-7.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] location Switch for the different locations of DAC85689. See enum @ref DAC8568_location_t in enum_settings.h for details.
 * @param[in] cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * @param[in] channel select one of eight output channel (0 ... 7)
 * @param[in] output output value that will be converted to analog voltage (0 ... 0xFFFF)
 * @return @ref es_status_codes
 */
es_status_codes DAC8568_setOutput(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output)
{
	//ctrl bits 3: write and update DAC register
	ES_LOG("Set DAC: board %"PRIu32", location %"PRIu8", cameraPosition %"PRIu8", output ch%"PRIu8" = %"PRIu16"\n", drvno, location, cameraPosition, channel, output);
	return DAC8568_sendData(drvno, location, cameraPosition, 3, channel, output, 0);
}

/**
 * @brief Enable the internal reference in static mode.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param location Switch for the different locations of DAC85689. See enum @ref DAC8568_location_t in enum_settings.h for details.
 * @param cameraPosition This is describing the camera position when there are multiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * @return @ref es_status_codes
 */
es_status_codes DAC8568_enableInternalReference(uint32_t drvno, uint8_t location, uint8_t cameraPosition)
{
	ES_LOG("DAC %"PRIu8": enable internal reference\n", location);
	return DAC8568_sendData(drvno, location, cameraPosition, 8, 0, 0, 1);
}

/**
 * @brief This function sets the register BEC.
 *
 * The Block Exposure control (BEC) signal can be used to open and close a mechanical shutter.
 * BEC starts after the block delay after trigger (BDAT) signal and is active for bec_in_10ns.
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param bec_in_10ns:
 *	- =0 no BEC
 *	- >0 Time in 10 ns steps. Min: 1 * 10 ns, Max: 4294967295 * 10ns = 42949672950ns = 42,94967295s
 * @return @ref es_status_codes
 */
es_status_codes SetBEC(uint32_t drvno, uint32_t bec_in_10ns)
{
	ES_LOG("Set BEC in 10 ns: %"PRIu32"\n", bec_in_10ns);
	return writeRegisterS0_32(drvno, bec_in_10ns, S0Addr_BEC);
}

/**
 * @brief DEPRECATED. Set XCK delay.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param xckdelay_in_10ns XCK delay, 31 bit. xckdelay = 500ns + xckdelay_in_10ns * 10ns
 *		* disable: 0
 *		* min 1: 500 ns + 1 * 10ns = 510 ns
 *		* max 2.147.483.647: 500 ns + 2.147.483.647 * 10 ns = 21.474.836.970 ns = 21,474.836.970 s
 * @return @ref es_status_codes
 */
es_status_codes SetXckdelay(uint32_t drvno, uint32_t xckdelay_in_10ns)
{
	ES_LOG("Set XCK delay: %"PRIu32"\n", xckdelay_in_10ns);
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
 * @brief Set DMA related registers like TLP mode and DMA addresses.
 *
 * @param drvno
 * @param pixel
 * @return
 */
es_status_codes SetDmaRegister(uint32_t drvno, uint32_t pixel)
{
	ES_LOG("Set DMA register: drv: %"PRIu32", pixel: %"PRIu32"\n", drvno, pixel);
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
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return @ref es_status_codes
 */
es_status_codes writeBitsDma_32(uint32_t drvno, uint32_t data, uint32_t bitmask, uint32_t address)
{
	uint32_t OldRegisterValues = 0;
	LockHighLevelMutex(drvno);
	//read the old Register Values in the S0 Address Reg
	es_status_codes status = readRegister_32(drvno, &OldRegisterValues, address);
	if (status != es_no_error)
	{
		UnlockHighLevelMutex(drvno);
		return status;
	}
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
	status = writeRegister_32(drvno, NewRegisterValues, address);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Set specified bits to 1 in DMA register at memory address.
 *
 * @param data 1 bytes (8 bits) data to write
 * @param bitmask Bitmask to select specific bits, which should be written. 0xFF - all bits 8 bits are written, 0 - no bits are written.
 * @param address Address of the register in DMA space.
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0.
 * @return @ref es_status_codes
 */
es_status_codes writeBitsDma_8(uint32_t drvno, uint8_t data, uint8_t bitmask, uint32_t address)
{
	uint8_t OldRegisterValues = 0;
	LockHighLevelMutex(drvno);
	//read the old Register Values in the S0 Address Reg
	es_status_codes status = readRegister_8(drvno, &OldRegisterValues, address);
	if (status != es_no_error)
	{
		UnlockHighLevelMutex(drvno);
		return status;
	}
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
	status = writeRegister_8(drvno, NewRegisterValues, address);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Write 4 bytes to a register in DMA space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Read buffer.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterDma_32(uint32_t drvno, uint32_t data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = writeRegister_32(drvno, data, address);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Write 1 byte to a register in DMA space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Read buffer.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes writeRegisterDma_8(uint32_t drvno, uint8_t data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = writeRegister_8(drvno, data, address);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Read 4 bytes of a register in DMA space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Read buffer.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes readRegisterDma_32(uint32_t drvno, uint32_t* data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = readRegister_32(drvno, data, address);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Read 1 byte of a register in DMA space.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param data Read buffer.
 * @param address Address of the register to read.
 * @return @ref es_status_codes
 */
es_status_codes readRegisterDma_8(uint32_t drvno, uint8_t* data, uint32_t address)
{
	LockHighLevelMutex(drvno);
	es_status_codes status = readRegister_8(drvno, data, address);
	UnlockHighLevelMutex(drvno);
	return status;
}

/**
 * @brief Set DMA Start Mode
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param start_by_hardware true: every XCK h->l starts DMA by hardware, false: by software
 * @return @ref es_status_codes
 */
es_status_codes SetDmaStartMode(uint32_t drvno, bool start_by_hardware)
{
	if (PcieCardVersionIsSmallerThan(drvno, 0x222, 0x18))
	{
		ES_LOG("Set DMA start mode: %d\n", start_by_hardware);
		uint32_t data = 0;
		if (start_by_hardware)
			data = IRQREG_bit_HWDREQ_EN;
		return writeBitsS0_32(drvno, data, IRQREG_bit_HWDREQ_EN, S0Addr_IRQREG);
	}
	else
		// IRQREG_bit_HWDREQ_EN is always 1 since version 222_18, so it is not necessary / possible to set it
		return es_no_error;
}

/**
 * @brief  This function is starting the measurement and returns when the measurement is done.
 *
 * When there are multiple boards, all boards are starting the measurement. Create a new thread for calling this function, when you don't want to have a blocking call.
 *
 * @return @ref es_status_codes
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
	// Clear old events of ESC and SPACE to make sure that only buttons pressed during the measurement are counted.
	clearKeyStates();
#ifndef MINIMAL_BUILD
	setTimestamp();
#endif
	measurement_cnt = 0;
#ifdef WIN32
	for (uint32_t i = 0; i < MAXPCIECARDS; i++)
	{
		data_available[i] = 0;
	}
#endif
	continuousMeasurementFlag = (bool)settings_struct.continuous_measurement;//0 or 1
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
		ES_LOG("measurement count: %"PRIu64"\n", measurement_cnt);
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
		for (uint32_t blk_cnt = 0; blk_cnt < settings_struct.nob; blk_cnt++)
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
					// legacy since P222_14: only for backwards compatibility with older versions
					if (PcieCardVersionIsSmallerThan(drvno, 0x222, 0x14))
					{
						status = waitForBlockTrigger(drvno);
						if (status == es_abortion)
						{
							status = AbortMeasurement();
							return ReturnStartMeasurement(status);
						}
						else if (status != es_no_error) return ReturnStartMeasurement(status);
					}
					// Only wait for the block trigger of one board
					break;
				}
			}
			ES_LOG("Block %"PRIu32" triggered\n", blk_cnt);
			// setBlockEn, ArmScanTrigger and DoSoftwareTrigger are starting the measurement.
			// timer must be started in each block as the scan counter stops it by hardware at end of block
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
			{
				// Check if the drvno'th bit is set
				if ((settings_struct.board_sel >> drvno) & 1)
				{
					status = ArmScanTrigger(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					status = setBlockEn(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					status = WaitForBlockOn(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					//start scan for first read if STI = ASL
					if (settings_struct.camera_settings[drvno].sti_mode == sti_ASL) status = DoSoftwareTrigger(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
					timerOn[drvno] = true;
				}
				else
					timerOn[drvno] = false;
			}
			// Main read loop. The software waits here until the flag RegXCKMSB:b30 = TimerOn is reset by hardware,
			// if flag HWDREQ_EN is TRUE.
			// This is done when nos scans are counted by hardware. Pressing ESC can cancel this loop.
			ES_LOG("Wait for the end of block %"PRIu32".\n", blk_cnt);
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
						if (!abortMeasurementFlag && checkEscapeKeyState())
							abortMeasurementFlag = true;
						status = GetArmScanTriggerStatus(drvno, (bool*)&timerOn[drvno]);
						if (status != es_no_error) return ReturnStartMeasurement(status);
					}
				}
			}
			ES_LOG("Block %"PRIu32" done.\n", blk_cnt);
			// When the software reaches this point, all scans for the current block are done.
			// So blockOn is reset here.
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
			{
				// Check if the drvno'th bit is set
				if ((settings_struct.board_sel >> drvno) & 1)
				{
					status = resetBlockEn(drvno);
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
				ES_TRACE("Wait for mutex %"PRIu32"\n", drvno);
				pthread_mutex_lock(&mutex[drvno]);
				pthread_mutex_unlock(&mutex[drvno]);
				ES_TRACE("Received unlocked mutex %"PRIu32"\n", drvno);
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
				status = DisarmScanTrigger(drvno);
				if (status != es_no_error) return ReturnStartMeasurement(status);
				if (!settings_struct.camera_settings[drvno].use_software_polling)
				{
					status = GetLastBufPart(drvno);
					if (status != es_no_error) return ReturnStartMeasurement(status);
				}
			}
		}
		notifyAllBlocksDone();
		if(allBlocksDoneHook)
			allBlocksDoneHook();
		// Maybe this is not needed anymore because of WaitForAllInterruptsDone
		// This sleep is here to prevent the measurement being interrupted too early. When operating with 2 cameras the last scan could be cut off without the sleep. This is only a workaround. The problem is that the software is waiting for RSTIMER being reset by the hardware before setting measure on and block on to low, but the last DMA is done after RSTIMER being reset. BLOCKON and MEASUREON should be reset after all DMAs are done.
		// RSTIMER --------________
		// DMAWRACT _______-----___
		// BLOCKON ---------_______
		// MEASUREON ---------_____
		//WaitforTelapsed(100);
		// When space key or ESC key was pressed, continuous measurement stops.
		if (continuousMeasurementFlag && checkSpaceKeyState())
			continuousMeasurementFlag = false;
		if (!abortMeasurementFlag && checkEscapeKeyState())
			abortMeasurementFlag = true;
		WaitforTelapsed(settings_struct.cont_pause_in_microseconds);
	} while (continuousMeasurementFlag && !abortMeasurementFlag);
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
 * @brief This is a helper function to return startMeasurement.
 *
 * This function sets isRunning = false and returns the given status.
 * @param status Status that will be returned.
 * @return Returns input parameter status.
 */
es_status_codes ReturnStartMeasurement(es_status_codes status)
{
	isRunning = false;
	return status;
}

/**
 * @brief Test if SFP module is there and fiber is linked up.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes FindCam(uint32_t drvno)
{
	if (settings_struct.camera_settings[drvno].camcnt == 0)
	{
		// Camcnt is 0. FindCam is returning without error
		return es_no_error;
	}
	bool linkUp = false;
	bool sfpError = false;
	es_status_codes status;
#ifdef WIN32
	bool firstFailure = true;
	int64_t timestampFirstFailureInMicroseconds = GetTimestampInMicroseconds();
	int64_t timeoutInMicroseconds = 10000;
	// Check the connection again while timout is not reached.
	// This loop is here to prevent checking during link down spikes
	while (GetTimestampInMicroseconds() - timestampFirstFailureInMicroseconds < timeoutInMicroseconds)
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
 * @brief Reset the hardware block counter and scan counter.
 *
 * @param drvno board number
 * @return @ref es_status_codes
*/
es_status_codes ResetHardwareCounter(uint32_t drvno)
{
	ES_LOG("Reset hardware counter\n");
	es_status_codes status = pulseBitS0_32(drvno, DMAsPerIntr_bitindex_counter_reset, S0Addr_DMAsPerIntr, 100);
	if (status != es_no_error) return status;
	status = pulseBitS0_32(drvno, DmaBufSizeInScans_bitindex_counter_reset, S0Addr_DmaBufSizeInScans, 100);
	if (status != es_no_error) return status;
	status = pulseBitS0_32(drvno, BLOCKINDEX_bitindex_counter_reset, S0Addr_BLOCKINDEX, 100);
	if (status != es_no_error) return status;
	return pulseBitS0_32(drvno, ScanIndex_bitindex_counter_reset, S0Addr_ScanIndex, 100);
}

/**
 * @brief Reset the internal intr collect counter.
 *
 * @param drvno board number
 * @param stop_by_hardware true: timer is stopped by hardware if nos is reached
 * @return @ref es_status_codes
*/
es_status_codes SetHardwareTimerStopMode(uint32_t drvno, bool stop_by_hardware)
{
	if (PcieCardVersionIsSmallerThan(drvno, 0x222, 0x18))
	{
		ES_LOG("Set hardware timer stop mode: %d\n", stop_by_hardware);
		es_status_codes status;
		if (stop_by_hardware)
			//when SCANINDEX reaches NOS, the timer is stopped by hardware.
			status = setBitS0_32(drvno, PCIEFLAGS_bitindex_ENRSTIMERHW, S0Addr_PCIEFLAGS);
		else
			//stop only with write to RS_Timer Reg
			status = resetBitS0_32(drvno, PCIEFLAGS_bitindex_ENRSTIMERHW, S0Addr_PCIEFLAGS);
		return status;
	}
	else
		// PCIEFLAGS_bitindex_ENRSTIMERHW is always 1 since version 222_18, so it is not necessary / possible to set it
		return es_no_error;
}

/**
 * @brief Pulse bit () -> 1 -> 0) in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @param duration_in_microseconds Duration of the bit beeing high in microseconds.
 * @return @ref es_status_codes
 */
es_status_codes pulseBitS0_32(uint32_t drvno, uint32_t bitnumber, uint32_t address, int64_t duration_in_microseconds)
{
	es_status_codes status = setBitS0_32(drvno, bitnumber, address);
	if (status != es_no_error) return status;
	WaitforTelapsed(duration_in_microseconds);
	return resetBitS0_32(drvno, bitnumber, address);
}

/**
 * @brief Pulse bit () -> 1 -> 0) in S0 register at memory address.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param bitnumber 0...7, 0 is LSB, 7 MSB
 * @param address register address. 1 byte steps are valid.
 * @param duration_in_microseconds Duration of the bit beeing high in microseconds.
 * @return @ref es_status_codes
 */
es_status_codes pulseBitS0_8(uint32_t drvno, uint32_t bitnumber, uint32_t address, int64_t duration_in_microseconds)
{
	es_status_codes status = setBitS0_8(drvno, bitnumber, address);
	if (status != es_no_error) return status;
	WaitforTelapsed(duration_in_microseconds);
	return resetBitS0_8(drvno, bitnumber, address);
}

/**
 * @brief Wait in loop until block trigger occurs.
 * 
 * Since P222_14 this is a legacy function only for backwards compatibility. P222_14 and newer versions will always return 1 at the block trigger bit.
 * If block trigger high: return
 * If block trigger low: wait for hi
 * Checks only PCIE board no 1
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0. May only work for 1
 * @return @ref es_status_codes
 */
es_status_codes waitForBlockTrigger(uint32_t drvno)
{
	ES_LOG("Wait for block trigger\n");
	bool blockTriggered = false;
	es_status_codes status;
	while (!abortMeasurementFlag)
	{
		if (!abortMeasurementFlag && checkEscapeKeyState())
			abortMeasurementFlag = true;
		status = ReadBitS0_32(drvno, S0Addr_CTRL, CTRL_bitindex_BSTART, &blockTriggered);
		if (status != es_no_error) return status;
		if (blockTriggered)
			return es_no_error;
	}
	return es_abortion;
}

/**
 * @brief Sends signal to hardware to count blocks
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes countBlocksByHardware(uint32_t drvno)
{
	ES_LOG("Increase hardware block counter\n");
	es_status_codes status = pulseBitS0_32(drvno, PCIEFLAGS_bitindex_BLOCKTRIG, S0Addr_PCIEFLAGS, 10);
	if (status != es_no_error) return status;
	//reset scan counter
	return pulseBitS0_32(drvno, ScanIndex_bitindex_counter_reset, S0Addr_ScanIndex, 10);
}

/**
 * @brief Sets Scan Timer on.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ArmScanTrigger(uint32_t drvno)
{
	if (PcieCardVersionIsSmallerThan(drvno, 0x222, 0x18))
	{
		ES_LOG("Arm scan trigger\n");
		return setBitS0_32(drvno, XCK_bitindex_arm_scan_trigger, S0Addr_XCK);
	}
	else
		// Since version 222_18 XCK_bitindex_arm_scan_trigger is read only.
		return es_no_error;
}

/**
 * @brief Triggers one camera read by calling this function.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes DoSoftwareTrigger(uint32_t drvno)
{
	ES_LOG("Do software trigger\n");
	es_status_codes status = setBitS0_32(drvno, FFCTRL_bitindex_SWTRIG, S0Addr_PIXREG_FFCTRL_FFFLAGS);
	if (status != es_no_error) return status;
	return resetBitS0_32(drvno, FFCTRL_bitindex_SWTRIG, S0Addr_PIXREG_FFCTRL_FFFLAGS);
}

/**
 * @brief Checks if timer is active (Bit30 of XCK-Reg).
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param on
 * @return @ref es_status_codes
 */
es_status_codes GetArmScanTriggerStatus(uint32_t drvno, bool* on)
{
	return ReadBitS0_32(drvno, S0Addr_XCK, XCK_bitindex_arm_scan_trigger, on);
}

/**
 * @brief For the rest part of the buffer.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes GetLastBufPart(uint32_t drvno)
{
	ES_TRACE("Get the last buffer part\n");
	// Get the rest if buffer is not multiple of 500 (BUFSIZEINSCANS/2)
	// Also if nos is < BUFSIZEINSCANS/2 there is data left in the DMA buffer, because no interrupt occurred after the last scans.
	uint32_t spi = 0;
	// Get scans per interrupt
	es_status_codes status = readRegisterS0_32(drvno, &spi, S0Addr_DMAsPerIntr);
	if (status != es_no_error) return status;
	// dmaHalfBufferSize is 500 with default values
	uint32_t dmaHalfBufferSize = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS;
	uint32_t scans_all_cams = settings_struct.nos * settings_struct.nob * virtualCamcnt[drvno];
	uint32_t rest_overall = scans_all_cams % dmaHalfBufferSize;
	size_t rest_in_bytes = rest_overall * settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t);
	ES_TRACE("nos: %"PRIu32", nob: %"PRIu32", scansPerInterrupt: %"PRIu32", camcnt: %"PRIu32"\n", settings_struct.nos, settings_struct.nob, spi, virtualCamcnt[drvno]);
	ES_TRACE("scans_all_cams: %"PRIu32" \n", scans_all_cams);
	ES_TRACE("rest_overall: %"PRIu32", rest_in_bytes: %"PRIu64"\n", rest_overall, rest_in_bytes);
	if (rest_overall)
		copyRestData(drvno, rest_in_bytes);
	return status;
}

/**
 * @brief Initializes the PCIe board.
 *
 * Is called by InitDriver. It is only needed to be called once.
 * @return @ref es_status_codes
 */
es_status_codes InitBoard()
{
	ES_LOG("\n*** Init board ***\n");
	// Initialize settings struct
	for (uint32_t drvno = 0; drvno < MAXPCIECARDS; drvno++)
		memcpy(&settings_struct.camera_settings[drvno], &camera_settings_default, sizeof(struct camera_settings));
	ES_LOG("Number of boards: %"PRIu8"\n", number_of_boards);
	if (number_of_boards < 1) return es_open_device_failed;
	es_status_codes status = es_no_error;
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
 * @brief Initialize the driver.
 *
 * Call this before any other action. It is only needed to be called once at startup.
 * @return @ref es_status_codes
 */
es_status_codes InitDriver()
{
	ES_LOG("\n*** Init driver ***\n");
	es_status_codes status = _InitDriver();
	initPerformanceCounter();
	ES_LOG("*** Init driver done***\n\n");
	InitBoard();
	return status;
}

/**
 * @brief Exit driver. Call this before exiting software for cleanup.
 *
 * @return @ref es_status_codes
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
 * @brief Copy the data of a single sample to pdest.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number ( 0...(nos - 1) )
 * @param[in] block block number ( 0...(nob - 1) )
 * @param[in] camera camera number ( 0...(CAMCNT - 1) )
 * @param[out] pdest Pointer where the data will be written to. Make sure that the size is >= sizeof(uint16_t) * pixel
 * @return @ref es_status_codes
 */
es_status_codes CopyOneSample(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest)
{
	//ES_TRACE( "Return frame: drvno: %"PRIu32", sample: %"PRIu32", block: %"PRIu32", camera: %"PRIu16", pdest %p\n", drvno, sample, block, camera, pdest );
	if (!pdest)
		return es_invalid_pointer;
	uint16_t* pSrc = NULL;
	es_status_codes status = GetOneSamplePointer(drvno, sample, block, camera, &pSrc, NULL);
	if (status != es_no_error) return status;
	//ES_LOG("pSrc %p\n", pSrc);
	//ES_LOG("userBuffer %p\n", userBuffer[drvno]);
	memcpy(pdest, pSrc, settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t));
	return status;
}

/**
 * @brief Copy the data of a single block to pdest.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] block block number ( 0...(nob - 1) )
 * @param[out] pdest Pointer where the data will be written to. Make sure that the size of the buffer is >= sizeof(uint16_t) * pixel * nos * camcnt
 * @return @ref es_status_codes
 */
es_status_codes CopyOneBlock(uint32_t drvno, uint16_t block, uint16_t* pdest)
{
	if (!pdest)
		return es_invalid_pointer;
	uint16_t* pSrc = NULL;
	es_status_codes	status = GetOneBlockPointer(drvno, block, &pSrc, NULL);
	if (status != es_no_error) return status;
	memcpy(pdest, pSrc, (uint64_t)settings_struct.nos * (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t));
	return status;
}

/**
 * @brief Copy the data of the complete measurement to pdest.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] pdest Pointer where the data will be written to. Make sure that the size of the buffer is >= sizeof(uint16_t) * pixel * nos * camcnt * nob
 * @return @ref es_status_codes
 */
es_status_codes CopyAllData(uint32_t drvno, uint16_t* pdest)
{
	if (!pdest)
		return es_invalid_pointer;
	uint16_t* pSrc = NULL;
	es_status_codes	status = GetAllDataPointer(drvno, &pSrc, NULL);
	if (status != es_no_error) return status;
	memcpy(pdest, pSrc, (uint64_t)settings_struct.nos * (uint64_t)settings_struct.nob * (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t));
	return status;
}

/**
 * @brief Copy the data of a custom length to pdest.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number ( 0...(nos - 1) )
 * @param[in] block block number ( 0...(nob - 1) )
 * @param[in] camera camera number ( 0...(CAMCNT - 1) )
 * @param[in] pixel position in one scan (0...(PIXEL-1))
 * @param[in] length_in_pixel Number of pixels to copy. When length_in_pixel exceeds the end of the data buffer the function returns es_parameter_out_of_range.
 * @param[out] pdest Pointer where the data will be written to. Make sure that the size of the buffer is >= sizeof(uint16_t) * length_in_pixel
 * @return @ref es_status_codes
 */
es_status_codes CopyDataArbitrary(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t pixel, size_t length_in_pixel, uint16_t* pdest)
{
	if (!pdest)
		return es_invalid_pointer;
	uint16_t* pSrc = NULL;
	size_t bytes_to_end_of_buffer = 0;
	es_status_codes	status = GetPixelPointer(drvno, (uint16_t)pixel, sample, block, camera, &pSrc, &bytes_to_end_of_buffer);
	if (status != es_no_error) return status;
	if (length_in_pixel * sizeof(uint16_t) > bytes_to_end_of_buffer) return es_parameter_out_of_range;
	memcpy(pdest, pSrc, length_in_pixel * sizeof(uint16_t));
	return status;

}

/**
 * @brief Returns the index of a pixel located in userBuffer.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param pixel position in one scan (0...(PIXEL-1))
 * @param sample position in samples (0...(nos-1))
 * @param block position in blocks (0...(nob-1))
 * @param camera position in camera count (0...(CAMCNT-1)
 * @param pIndex Pointer to index of pixel.
 * @return @ref es_status_codes
 */
es_status_codes GetIndexOfPixel(uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint64_t* pIndex)
{
	if (!pIndex)
		return es_invalid_pointer;
	if (pixel >= settings_struct.camera_settings[drvno].pixel || sample >= settings_struct.nos || block >= settings_struct.nob || camera >= virtualCamcnt[drvno])
		return es_parameter_out_of_range;
	es_status_codes status = checkDriverHandle(drvno);
	if (testModeOn && status != es_no_error) status = es_no_error;
	if (status != es_no_error) return status;
	//init index with base position of pixel
	uint64_t index = pixel;
	//position of index at camera position
	index += (uint64_t)camera * ((uint64_t)settings_struct.camera_settings[drvno].pixel); // AM! was +4 for PCIe version before P202_23 to shift scan counter in 2nd camera to the left
	//position of index at sample
	index += (uint64_t)sample * (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.camera_settings[drvno].pixel;
	//position of index at block
	index += (uint64_t)block * (uint64_t)settings_struct.nos * (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.camera_settings[drvno].pixel;
	*pIndex = index;
	return status;
}

/**
 * @brief Returns the address of a specific pixel. 
 * 
 * It is only safe to read as much data until the end of the buffer is reached. That is determined by bytes_to_end_of_buffer. It is always safe to read 2 bytes. The address you get is valid until the next call of @ref InitMeasurement. The data behind the address changes every measurement cycle when continuous mode is on or after each call of @ref StartMeasurement.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] pixel position in one scan (0...(PIXEL-1))
 * @param[in] sample position in samples (0...(nos-1))
 * @param[in] block position in blocks (0...(nob-1))
 * @param[in] camera position in camera count (0...(CAMCNT-1))
 * @param[out] pdest Pointer to get the pointer of the specific pixel. When NULL, this functions returns es_invalid_pointer.
 * @param[out] bytes_to_end_of_buffer Pointer to get the number of bytes to the end of the buffer. This is the size in bytes of the buffer from the position you are asking for and the end of the buffer. When NULL, this parameter is ignored.
 * @return @ref es_status_codes
 */
es_status_codes GetPixelPointer(uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	if (!userBuffer[drvno])
		return es_memory_not_initialized;
	if (!pdest)
		return es_invalid_pointer;
	uint64_t index = 0;
	es_status_codes status = GetIndexOfPixel(drvno, pixel, sample, block, camera, &index);
	if (status != es_no_error) return status;
	*pdest = &userBuffer[drvno][index];
	if (bytes_to_end_of_buffer)
	{
		uint64_t indexEnd = 0;
		status = GetIndexOfPixel(drvno, (uint16_t)settings_struct.camera_settings[drvno].pixel - 1, settings_struct.nos - 1, settings_struct.nob - 1, (uint16_t)settings_struct.camera_settings[drvno].camcnt - 1, &indexEnd);
		if (status != es_no_error) return status;
		*bytes_to_end_of_buffer = (indexEnd - index + 1) * sizeof(uint16_t);
	}
	return status;
}

/**
 * @brief Returns the address of the data buffer.
 *
 * It is only safe to read as much data until the end of the buffer is reached. That is determined by bytes_to_end_of_buffer. For this function bytes_to_end_of_buffer equals the size of the complete buffer (= nos * nob * pixel * camcnt * 2). The address you get is valid until the next call of @ref InitMeasurement. The data behind the address changes every measurement cycle when continuous mode is on or after each call of @ref StartMeasurement.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] pdest Pointer to get the pointer of the data buffer. When NULL, this functions returns es_invalid_pointer.
 * @param[out] bytes_to_end_of_buffer Pointer to get the number of bytes to the end of the buffer. This is the size in bytes of the whole measurement data memory for one board. When NULL, this parameter is ignored.
 * @return @ref es_status_codes
 */
es_status_codes GetAllDataPointer(uint32_t drvno, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	return GetPixelPointer(drvno, 0, 0, 0, 0, pdest, bytes_to_end_of_buffer);
}

/**
 * @brief Returns the address of a specific block.
 *
 * It is only safe to read as much data until the end of the buffer is reached. That is determined by bytes_to_end_of_buffer. For this function bytes_to_end_of_buffer is at least the size of one block (= nos * pixel * camcnt * 2). The address you get is valid until the next call of @ref InitMeasurement. The data behind the address changes every measurement cycle when continuous mode is on or after each call of @ref StartMeasurement.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] block position in blocks (0...(nob-1))
 * @param[out] pdest Pointer to get the pointer of the specific pixel. When NULL, this functions returns es_invalid_pointer.
 * @param[out] bytes_to_end_of_buffer Pointer to get the number of bytes to the end of the buffer. This is the size in bytes of the buffer from the position you are asking for and the end of the buffer. When NULL, this parameter is ignored.
 * @return @ref es_status_codes
 */
es_status_codes GetOneBlockPointer(uint32_t drvno, uint32_t block, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	return GetPixelPointer(drvno, 0, 0, block, 0, pdest, bytes_to_end_of_buffer);
}

/**
 * @brief Returns the address of a specific sample.
 *
 * It is only safe to read as much data until the end of the buffer is reached. That is determined by bytes_to_end_of_buffer. For this fuction bytes_to_end_of_buffer is a least the size of scan (= pixel * 2). The address you get is valid until the next call of @ref InitMeasurement. The data behind the address changes every measurement cycle when continuous mode is on or after each call of @ref StartMeasurement.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample position in samples (0...(nos-1))
 * @param[in] block position in blocks (0...(nob-1))
 * @param[in] camera position in camera count (0...(CAMCNT-1))
 * @param[out] pdest Pointer to get the pointer of the specific pixel. When NULL, this functions returns es_invalid_pointer.
 * @param[out] bytes_to_end_of_buffer Pointer to get the number of bytes to the end of the buffer. This is the size in bytes of the buffer from the position you are asking for and the end of the buffer. When NULL, this parameter is ignored.
 * @return @ref es_status_codes
 */
es_status_codes GetOneSamplePointer(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	return GetPixelPointer(drvno, 0, sample, block, camera, pdest, bytes_to_end_of_buffer);
}

/**
 * @brief Calculate the theoretical time needed for one measurement.
 *
 * nos * nob * exposure_time_in_ms / 1000
 * @param[in] nos number of samples
 * @param[in] nob number of blocks
 * @param[out] exposure_time_in_ms exposure time in ms
 * @return time in seconds
 */
double CalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms)
{
	double measureTime = (double)nos * (double)nob * exposure_time_in_ms / 1000;
	return measureTime;
}

/**
 * @brief Calculate needed RAM in MB for given nos and nob.
 *
 * @param[in] nos number of samples
 * @param[in] nob number of blocks
 * @return RAM in MB
 */
double CalcRamUsageInMB(uint32_t nos, uint32_t nob)
{
	ES_LOG("Calculate ram usage in MB, nos: %"PRIu32":, nob: %"PRIu32"\n", nos, nob);
	double ramUsage = 0;
	for (int i = 0; i < number_of_boards; i++)
		ramUsage += (uint64_t)nos * (uint64_t)nob * (uint64_t)settings_struct.camera_settings[i].pixel * (uint64_t)virtualCamcnt[i] * sizeof(uint16_t);
	ramUsage = ramUsage / 1048576;
	ES_LOG("Ram usage: %f\n", ramUsage);
	return ramUsage;
}

/**
 * @brief Calculate TRMS noise value of one pixel.
 *
 * Calculates RMS of TRMS_pixel in the range of samples from firstSample to lastSample. Only calculates RMS from one block.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] firstSample start sample to calculate RMS. 0...(nos-2). Typical value: 10, to skip overexposed first samples
 * @param[in] lastSample last sample to calculate RMS. firstSample+1...(nos-1).
 * @param[in] TRMS_pixel pixel for calculating noise (0...(PIXEL-1))
 * @param[in] CAMpos index for camcount (0...(CAMCNT-1))
 * @param[out] mwf pointer for mean value
 * @param[out] trms pointer for noise
 * @return @ref es_status_codes
 */
es_status_codes CalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double* mwf, double* trms)
{
	if (firstSample >= lastSample || lastSample > settings_struct.nos || userBuffer[drvno] == 0)
	{
		//error: firstSample must be smaller than lastSample
		ES_LOG("Calc Trms failed. lastSample must be greater than firstSample and both in boundaries of nos, drvno: %"PRIu32", firstSample: %"PRIu32", lastSample: %"PRIu32", TRMS_pixel: %"PRIu32", CAMpos: %"PRIu16", Nospb: %"PRIu32"\n", drvno, firstSample, lastSample, TRMS_pixel, CAMpos, settings_struct.nos);
		*mwf = -1;
		*trms = -1;
		return es_parameter_out_of_range;
	}
	uint32_t samples = lastSample - firstSample;
	uint16_t* TRMS_pixels = calloc(samples, sizeof(uint16_t));
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

void GetRmsVal(uint32_t nos, uint16_t* TRMSVals, double* mwf, double* trms)
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
 * @brief Checks content of FIFO.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] valid Is true (not 0) if FIFO keeps >= 1 complete lines (linecounter>0).
 * @return @ref es_status_codes
 */
es_status_codes CheckFifoValid(uint32_t drvno, bool* valid)
{	// not empty & XCK = low -> true
	ES_LOG("checkFifoFlags\n");
	return ReadBitS0_32(drvno, S0Addr_PIXREG_FFCTRL_FFFLAGS, FF_FLAGS_bitindex_valid, valid);
}

/**
 * @brief Check ovl flag (overflow of FIFO).
 *
 * If occurred stays active until a call of FFRS.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] overflow Is true (not 0) if overflow occurred (linecounter>0).
 * @return @ref es_status_codes
 */
es_status_codes CheckFifoOverflow(uint32_t drvno, bool* overflow)
{
	ES_LOG("checkFifoOverflow\n");
	return ReadBitS0_32(drvno, S0Addr_PIXREG_FFCTRL_FFFLAGS, FF_FLAGS_bitindex_overflow, overflow);
}

/**
 * @brief Check empty flag (FIFO empty).
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] empty TODO: missing doc
 * @return @ref es_status_codes
 */
es_status_codes CheckFifoEmpty(uint32_t drvno, bool* empty)
{
	ES_LOG("checkFifoEmpty\n");
	return ReadBitS0_32(drvno, S0Addr_PIXREG_FFCTRL_FFFLAGS, FF_FLAGS_bitindex_empty, empty);
}

/**
 * @brief Check full flag (FIFO full).
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] full TODO missing doc
 * @return @ref es_status_codes
 */
es_status_codes CheckFifoFull(uint32_t drvno, bool* full)
{
	ES_LOG("checkFifoFull\n");
	return ReadBitS0_32(drvno, S0Addr_PIXREG_FFCTRL_FFFLAGS, FF_FLAGS_bitindex_full, full);
}

/**
 * @brief Check if measure on bit is set.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] measureOn True when measureon bit is set.
 * @return @ref es_status_codes
 */
es_status_codes GetMeasureOn(uint32_t drvno, bool* measureOn)
{
	return ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_MEASUREON, measureOn);
}

/**
 * @brief Reset trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes OutTrigLow(uint32_t drvno)
{
	return resetBitS0_32(drvno, CTRL_bitindex_TRIG_OUT, S0Addr_CTRL);
}

/**
 * @brief Set trigger out(Reg CtrlA:D3) of PCIe board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes OutTrigHigh(uint32_t drvno)
{
	return setBitS0_32(drvno, CTRL_bitindex_TRIG_OUT, S0Addr_CTRL);
}

/**
 * @brief Pulses trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param pulseWidthInMicroseconds duration of pulse in us
 * @return @ref es_status_codes
 */
es_status_codes OutTrigPulse(uint32_t drvno, int64_t pulseWidthInMicroseconds)
{
	es_status_codes status = OutTrigHigh(drvno);
	if (status != es_no_error) return status;
	WaitforTelapsed(pulseWidthInMicroseconds);
	return OutTrigLow(drvno);
}

/**
 * @brief Reads the binary state of an ext. trigger input.
 *
 * Direct read of inputs for polling.
 * @param drvno board number
 * @param btrig_ch specify input channel
 * 			- btrig_ch=0 not used
 * 			- btrig_ch=1 is PCIe trig in I
 * 			- btrig_ch=2 is S1
 * 			- btrig_ch=3 is S2
 * 			- btrig_ch=4 is S1&S2
 * 			- btrig_ch=5 is TSTART (GTI - DAT - EC)
 * @param state false when low, otherwise true
 * @return @ref es_status_codes
 */
es_status_codes readBlockTriggerState(uint32_t drvno, uint8_t btrig_ch, bool* state)
{
	uint32_t val = 0;
	*state = false;
	es_status_codes status = es_no_error;
	switch (btrig_ch)
	{
	default:
	case 0:
		*state = true;
		break;
	case 1: //I
		status = readRegisterS0_32(drvno, &val, S0Addr_CTRL);
		if (status != es_no_error) return status;
		if ((val & CTRL_bit_STRIGIN) > 0) *state = true;
		break;
	case 2: //S1
		status = readRegisterS0_32(drvno, &val, S0Addr_CTRL);
		if (status != es_no_error) return status;
		if ((val & CTRL_bit_S1) > 0) *state = true;
		break;
	case 3: //S2
		status = readRegisterS0_32(drvno, &val, S0Addr_CTRL);
		if (status != es_no_error) return status;
		if ((val & CTRL_bit_S2) > 0) *state = true;
		break;
	case 4: // S1&S2
		status = readRegisterS0_32(drvno, &val, S0Addr_CTRL);
		if (status != es_no_error) return status;
		if ((val & CTRL_bit_S1) == 0) *state = false;
		status = readRegisterS0_32(drvno, &val, S0Addr_CTRL);
		if (status != es_no_error) return status;
		if ((val & CTRL_bit_S2) == 0) *state = false;
		*state = true;
		break;
	case 5: // TSTART
		status = readRegisterS0_32(drvno, &val, S0Addr_CTRL);
		if (status != es_no_error) return status;
		if ((val & CTRL_bit_BSTART) > 0) *state = true;
		break;
	}
	return status;
}

/**
 * @brief Returns when block on bit is 0.
 *
 * @return @ref es_status_codes
 */
es_status_codes WaitForBlockDone()
{
	bool blockOn[MAXPCIECARDS] = { false, false, false, false, false };
	es_status_codes status = es_no_error;
	do
	{
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
			{
				status = GetBlockOn(drvno, &blockOn[drvno]);
				if (status != es_no_error) return status;
			}
		}
	} while ((blockOn[0] || blockOn[1] || blockOn[2] || blockOn[3] || blockOn[4]) && !abortMeasurementFlag);
	return status;
}

/**
 * @brief Returns when measure on bit is 0.
 *
 * @return @ref es_status_codes
 */
es_status_codes WaitForMeasureDone()
{
	ES_LOG("WaitForMeasureDone\n");
	bool measureOn[MAXPCIECARDS] = { false, false, false, false, false };
	es_status_codes status = es_no_error;
	do
	{
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((settings_struct.board_sel >> drvno) & 1)
			{
				status = GetMeasureOn(drvno, &measureOn[drvno]);
				if (status != es_no_error) return status;
			}
		}
	} while ((measureOn[0] || measureOn[1] || measureOn[2] || measureOn[3] || measureOn[4]) && !abortMeasurementFlag);
	return status;
}

/**
 * @brief Read all S0 registers and write them to a string in hex.
 * 
 * @param[in] drvno PCIe board identifier
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
 */
es_status_codes dumpS0Registers(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		number_of_registers = 49,
		bufferLength = 40
	};
	char register_names[number_of_registers][bufferLength] = {
		"DBR",
		"CTRLA",
		"XCKLL",
		"XCKCNTLL",
		"PIXREG",
		"FIFOCNT",
		"VCLKCTRL",
		"'EBST'",
		"DAT",
		"EC",
		"TOR",
		"ARREG",
		"GIOREG",
		"XCKPERIOD",
		"IRQREG",
		"PCI board version",
		"R0 PCIEFLAGS",
		"R1 NOS",
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
		"R15 BTICNT",
		"R16 BTimer",
		"R17 BDAT",
		"R18 BEC",
		"R19 BFLAGS",
		"R20 A1DSC",
		"R21 L1DSC",
		"R22 A2DSC",
		"R23 L2DSC",
		"R24 ATDC2",
		"R25 LTDC2",
		"R26 DSCCtrl",
		"R27 DAC",
		"R28 XCKLEN",
		"R29 BONLEN",
		"R30 CAMERA TYPE",
		"R31 BON PERIOD",
		"R32 STATECTRL"
	}; //Look-Up-Table for the S0 Registers
	uint32_t data = 0;
	//allocate string buffer
	*stringPtr = (char*)calloc(number_of_registers * bufferLength, sizeof(char));
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
			len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register 0x%x %s", i * 4, register_names[i]);
			return status;
		}
		//write register name and value to buffer
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "0x%x  \t%s\t0x%x\n", i * 4, register_names[i], data);
	}
	return status;
}

/**
 * @brief Read all S0 registers and write them to a string in a human readable format.
 *
 * @param[in] drvno PCIe board identifier
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
 */
es_status_codes dumpHumanReadableS0Registers(uint32_t drvno, char** stringPtr)
{
	enum N
	{
		bufferSize = 4000
	};
	int len = 0;
	*stringPtr = (char*)calloc(bufferSize, sizeof(char));
	bool isBitHigh;
	uint32_t data32 = 0;

	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "address\tname\tbit\tname\tvalue\n");
	/*=======================================================================*/

	//CTRLA
	es_status_codes status = readRegisterS0_32(drvno, &data32, S0Addr_CTRL);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "0x04\tCTRLA\t0\tVON\t%"PRIu32"\n", (data32 & CTRL_bit_VONOFF) >> CTRL_bitindex_VONOFF);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tXCK\t %"PRIu32"\n", (data32 & CTRL_bit_XCK) >> CTRL_bitindex_XCK);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t3\tTRIG OUT\t%"PRIu32"\n", (data32 & CTRL_bit_TRIG_OUT) >> CTRL_bitindex_TRIG_OUT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t4\tBOTH SLOPE\t%"PRIu32"\n", (data32 & CTRL_bit_BOTH_SLOPE) >> CTRL_bitindex_BOTH_SLOPE);
	isBitHigh = (data32 & CTRL_bit_SLOPE) >> CTRL_bitindex_SLOPE;
	if (isBitHigh)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tSLOPE\t%d (pos)\n", isBitHigh);
	else
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tSLOPE\t%d (neg)\n", isBitHigh);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t6\tSTRIGIN\t%"PRIu32"\n", (data32 & CTRL_bit_STRIGIN) >> CTRL_bitindex_STRIGIN);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t7\tBSTART\t%"PRIu32"\n", (data32 & CTRL_bit_BSTART) >> CTRL_bitindex_BSTART);

	/*=======================================================================*/

	//CTRLB
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x05\tCTRLB\t0-2\tSTI\t");
	int combinedValueSTI = (data32 & CTRL_bits_STI) >> CTRL_bitindex_STI0;

	switch (combinedValueSTI) {
	case sti_I:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (I)\n", combinedValueSTI);
		break;
	case sti_S1:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S1)\n", combinedValueSTI);
		break;
	case sti_S2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S2)\n", combinedValueSTI);
		break;
	case sti_S2_enable_I:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S2ENI)\n", combinedValueSTI);
		break;
	case sti_STimer:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (stimer)\n", combinedValueSTI);
		break;
	case sti_ASL:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (ASL)\n", combinedValueSTI);
		break;
	default:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (invalid)\n", combinedValueSTI);
		break;

	}

	//CTRLB SHON
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t3\tSHON\t%"PRIu32"\n\t\t4-6\tBTI\t", (data32 & CTRL_bit_SHON) >> CTRL_bitindex_SHON);

	int combinedValueBTI = ((data32 & CTRL_bits_BTI)) >> CTRL_bitindex_BTI0;

	switch (combinedValueBTI)
	{
	case bti_I:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (I)\n", combinedValueBTI);
		break;
	case bti_S1:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S1)\n", combinedValueBTI);
		break;
	case bti_S2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S2)\n", combinedValueBTI);
		break;
	case bti_S1S2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S1&S2)\n", combinedValueBTI);
		break;
	case bti_BTimer:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (btimer)\n", combinedValueBTI);
		break;
	case bti_S1chopper:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S1 Chopper)\n", combinedValueBTI);
		break;
	case bti_S2chopper:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S2 Chopper)\n", combinedValueBTI);
		break;
	case bti_S1S2chopper:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (S1&S2 Chopper)\n", combinedValueBTI);
		break;
	default:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%d (invalid)\n", combinedValueSTI);
		break;
	}

	/*=======================================================================*/

	//CTRLC
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x06\tCTRLC\t0\tI\t%"PRIu32"\n", (data32 & CTRL_bit_I) >> CTRL_bitindex_I);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t1\tS1\t%"PRIu32"\n", (data32 & CTRL_bit_S1) >> CTRL_bitindex_S1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tS2\t%"PRIu32"\n", (data32 & CTRL_bit_S2) >> CTRL_bitindex_S2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t4\tEOI\t%"PRIu32"\n", (data32 & CTRL_bit_eoi) >> CTRL_bitindex_eoi);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tEOI-CHB\t%"PRIu32"\n", (data32 & CTRL_bit_eoi_chb) >> CTRL_bitindex_eoi_chb);

	/*=======================================================================*/

	//Register XCK
	status = readRegisterS0_32(drvno, &data32, S0Addr_XCK);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x08\tXCK\t0-27\tXCK STIMER\t%"PRIu32"\n", (data32 & XCK_bits_stimer) >> XCK_bitindex_stimer);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t28\tRes_ns\t%"PRIu32"\n", (data32 & XCK_bit_res_ns) >> XCK_bitindex_res_ns);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t29\tRes_ms\t%"PRIu32"\n", (data32 & XCK_bit_res_ms) >> XCK_bitindex_res_ms);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t30\tarm scan trigger\t%"PRIu32"\n", (data32 & XCK_bit_arm_scan_trigger) >> XCK_bitindex_arm_scan_trigger);

	/*=======================================================================*/

	//Register XCKCNT
	status = readRegisterS0_32(drvno, &data32, S0Addr_XCKCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x0c\tXCKCNT\t0-28\t\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register PIXREG
	status = readRegisterS0_32(drvno, &data32, S0Addr_PIXREG_FFCTRL_FFFLAGS);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x10\tPIXREG\t0-15\t\t%"PRIu32"\n", (data32 & PIXREG_bits_pixel) >> PIXREG_bitindex_pixel);

	/*=======================================================================*/

	//FFCTRL
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x12\tFFCTRL\t4\tRSBTH\t%"PRIu32"\n", (data32 & FFCTRL_bit_block_reset) >> FFCTRL_bitindex_block_reset);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tRSSTH\t%"PRIu32"\n", (data32 & FFCTRL_bit_scan_reset) >> FFCTRL_bitindex_scan_reset);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t6\tSWTrig\t%"PRIu32"\n", (data32 & FFCTRL_bit_SWTRIG) >> FFCTRL_bitindex_SWTRIG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t7\tRS_FF\t%"PRIu32"\n", (data32 & FFCTRL_bit_RSFIFO) >> FFCTRL_bitindex_RSFIFO);

	/*=======================================================================*/

	//FF_FLAGS
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x13\tFF_FLAGS\t1\tBTFH\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_block_read) >> FF_FLAGS_bitindex_block_read);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tSTFH\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_scan_read) >> FF_FLAGS_bitindex_scan_read);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t3\tOVFL\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_overflow) >> FF_FLAGS_bitindex_overflow);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t4\tXCKI\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_xcki) >> FF_FLAGS_bitindex_xcki);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tFF\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_full) >> FF_FLAGS_bitindex_full);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t6\tEF\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_empty) >> FF_FLAGS_bitindex_empty);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t7\tVALID\t%"PRIu32"\n", (data32 & FF_FLAGS_bit_valid) >> FF_FLAGS_bitindex_valid);

	/*=======================================================================*/

	//FIFOCNT
	status = readRegisterS0_32(drvno, &data32, S0Addr_FIFOCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x14\tFIFOCNT\t0-7\tWRCNT\t%"PRIu32"\n", (data32 & FIFOCNT_bits_WRCNT) >> FIFOCNT_bitindex_WRCNT);

	/*=======================================================================*/

	//VCLKCTRL
	status = readRegisterS0_32(drvno, &data32, S0Addr_VCLKCTRL_VCLKFREQ);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x18\tVCLKCTRL\t0-11\tVCLKCNT\t%"PRIu32"\n", (data32 & VCLKCNT_bit_control));

	//Register VCLKFREQ
	int translatedVCLKFREQ = 0;
	if ((data32 & VCLKFREQ_bits) == 0) translatedVCLKFREQ = 0;
	else translatedVCLKFREQ = VCLKFREQ_base_value + ((data32 >> VCLKFREQ_bitindex) * VCLKFREQ_step_value);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t24-31\tVCLKFREQ\t%"PRIu32" (%d ns)\n", data32 >> VCLKFREQ_bitindex, translatedVCLKFREQ);

	/*=======================================================================*/

	//SDAT
	status = readRegisterS0_32(drvno, &data32, S0Addr_SDAT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x20\tSDAT\t0-30\tDelay\t%"PRIu32"\n", (data32 & SDAT_bit_control) >> SDAT_bitindex_control);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tEnabled\t%"PRIu32"\n", (data32 & SDAT_bit_enable) >> SDAT_bitindex_enable);

	/*=======================================================================*/

	//SEC
	status = readRegisterS0_32(drvno, &data32, S0Addr_SEC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x24\tSEC\t0-31\t\t%"PRIu32" (%"PRIu64" ns)\n", data32, ((uint64_t)data32) * 10);

	/*=======================================================================*/

	//TOR Register
	status = readRegisterS0_32(drvno, &data32, S0Addr_TOR_STICNT_TOCNT);
	//TICOUNT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x28\tTOR\t0-6\tSTICNT\t%"PRIu32"\n", (data32 & TOR_bits_STICNT) >> TOR_bitindex_STICNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t7\tSTICNT enabled\t%"PRIu32"\n", (data32 & TOR_bit_STICNT_EN) >> TOR_bitindex_STICNT_EN);
	//TOCOUNT
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t16-22\tTOCNT\t%"PRIu32"\n", (data32 & TOR_bits_TOCNT) >> TOR_bitindex_TOCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t23\tTOCNT enabled\t%"PRIu32"\n", (data32 & TOR_bit_TOCNT_EN) >> TOR_bitindex_TOCNT_EN);
	//TORMSB
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t24\tIS FFT LEGACY\t%"PRIu8"\n", (data32 & TOR_bit_ISFFT_LEGACY) >> TOR_bitindex_ISFFT_LEGACY);

	int combinedTO = data32 >> TOR_bitindex_TO0 | ((data32 & TOR_bitindex_TOSEL) >> TOR_bitindex_TOSEL) << 4;

	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t27-31\tValue\t%d (", combinedTO);

	switch (combinedTO)
	{
	case tor_xck:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "XCK)\n");
		break;
	case tor_rego:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "REGO)\n");
		break;
	case tor_von:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "VON)\n");
		break;
	case tor_dma_act:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "DMA_ACT)\n");
		break;
	case tor_asls:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ASLS)\n");
		break;
	case tor_stimer:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "STIMER)\n");
		break;
	case tor_btimer:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "BTIMER)\n");
		break;
	case tor_isr_act:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ISR_ACT)\n");
		break;
	case tor_s1:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "S1)\n");
		break;
	case tor_s2:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "S2)\n");
		break;
	case tor_block_on_synced:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "BLOCK_ON_SYNCED)\n");
		break;
	case tor_measureon:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "MEASUREON)\n");
		break;
	case tor_sdat:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "SDAT)\n");
		break;
	case tor_bdat:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "BDAT)\n");
		break;
	case tor_sec_mshut:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "SSHUT)\n");
		break;
	case tor_bec_mshut:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "BSHUT)\n");
		break;
	case tor_exposure_window:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "EXPOSURE_WINDOW)\n");
		break;
	case tor_to_cnt_out:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "TO_CNT_OUT)\n");
		break;
	case tor_secon:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "SECON)\n");
		break;
	case tor_i:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "I)\n");
		break;
	case tor_S1S2readDelay:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "S1S2readDelay)\n");
		break;
	case tor_block_on:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "BLOCK_ON)\n");
		break;
	case tor_unused_24:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Unused)\n");
		break;
	case tor_unused_25:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Unused)\n");
		break;
	case tor_unused_26:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Unused)\n");
		break;
	case tor_unused_27:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Unused)\n");
		break;
	case tor_unused_28:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Unused)\n");
		break;
	case tor_enffr:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ENFFR)\n");
		break;
	case tor_enffw:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ENFFW)\n");
		break;
	case tor_enffwrprot:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ENFFWRPROT)\n");
		break;
	}

	/*=======================================================================*/

	//Register ARREG
	status = readRegisterS0_32(drvno, &data32, S0Addr_ARREG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x2c\tARREG\t0-14\tROI Ranges\t%"PRIu32"\n", (data32 & ARREG_bit_pb_control) >> ARREG_bitindex_pb_control);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t15\tPartial Binning\t%"PRIu32"\n", (data32 & ARREG_bit_partial_binning) >> ARREG_bitindex_partial_binning);

	/*=======================================================================*/

	//Register GIOREG
	status = readRegisterS0_32(drvno, &data32, S0Addr_GIOREG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x30\tGIOREG\t0\tOutput 1\t%"PRIu32"\n", (data32 & GIOREG_bit_O1) >> GIOREG_bitindex_O1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t1\tOutput 2\t%"PRIu32"\n", ((data32 & GIOREG_bit_O2)) >> GIOREG_bitindex_O2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tOutput 3\t%"PRIu32"\n", (data32 & GIOREG_bit_O3) >> GIOREG_bitindex_O3);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t3\tOutput 4\t%"PRIu32"\n", (data32 & GIOREG_bit_O4) >> GIOREG_bitindex_O4);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t4\tOutput 5\t%"PRIu32"\n", (data32 & GIOREG_bit_O5) >> GIOREG_bitindex_O5);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tOutput 6\t%"PRIu32"\n", (data32 & GIOREG_bit_O6) >> GIOREG_bitindex_O6);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t6\tOutput 7\t%"PRIu32"\n", (data32 & GIOREG_bit_O7) >> GIOREG_bitindex_O7);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t7\tOutput 8\t%"PRIu32"\n", (data32 & GIOREG_bit_O8) >> GIOREG_bitindex_O8);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t8\tInput 1\t%"PRIu32"\n", (data32 & GIOREG_bit_I1) >> GIOREG_bitindex_I1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t9\tInput 2\t%"PRIu32"\n", (data32 & GIOREG_bit_I2) >> GIOREG_bitindex_I2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t10\tInput 3\t%"PRIu32"\n", (data32 & GIOREG_bit_I3) >> GIOREG_bitindex_I3);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t11\tInput 4\t%"PRIu32"\n", (data32 & GIOREG_bit_I4) >> GIOREG_bitindex_I4);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t12\tInput 5\t%"PRIu32"\n", (data32 & GIOREG_bit_I5) >> GIOREG_bitindex_I5);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t13\tInput 6\t%"PRIu32"\n", (data32 & GIOREG_bit_I6) >> GIOREG_bitindex_I6);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t14\tInput 7\t%"PRIu32"\n", (data32 & GIOREG_bit_I7) >> GIOREG_bitindex_I7);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t15\tInput 8\t%"PRIu32"\n", (data32 & GIOREG_bit_I8) >> GIOREG_bitindex_I8);


	/*=======================================================================*/

	//Register XCK PERIOD
	status = readRegisterS0_32(drvno, &data32, S0Addr_XCK_PERIOD);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x34\tXCK PERIOD\t0-31\tXCK PERIOD\t%"PRIu32" (%"PRIu64" ns)", data32, ((uint64_t)data32) * 10);

	/*=======================================================================*/

	//Register IRQREG
	status = readRegisterS0_32(drvno, &data32, S0Addr_IRQREG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x38\tIRQREG\t0-15\tIRQLAT\t%"PRIu32" (%"PRIu32" ns)\n", (data32 & IRQREG_bits_IRQLAT) >> IRQREG_bitindex_IRQLAT, ((data32 & IRQREG_bits_IRQLAT) >> IRQREG_bitindex_IRQLAT) * 25);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t16-29\tIRQCNT\t%"PRIu32"\n", (data32 & IRQREG_bits_IRQLAT) >> IRQREG_bitindex_IRQCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t30\tHWDREQ_EN\t%"PRIu32"\n", (data32 & IRQREG_bit_HWDREQ_EN) >> IRQREG_bitindex_HWDREQ_EN);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tISR active\t%"PRIu32"\n", ((data32 & IRQREG_bit_INTRSR)) >> IRQREG_bitindex_INTRSR);

	/*=======================================================================*/

	//Register PCIe board version
	status = readRegisterS0_32(drvno, &data32, S0Addr_PCI);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x3C\tPCI board version\t0-31\tversion\t%"PRIu32" (0x%x)\n", data32, data32);

	/*=======================================================================*/

	//Register PCIEFLAGS
	status = readRegisterS0_32(drvno, &data32, S0Addr_PCIEFLAGS);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x40\tPCIEFLAGS\t0\tXCKO\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_XCKI) >> PCIEFLAGS_bitindex_XCKI);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t1\tINTTRIG\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_INTTRIG) >> PCIEFLAGS_bitindex_INTTRIG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tENRSTIMERHW\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_ENRSTIMERHW) >> PCIEFLAGS_bitindex_ENRSTIMERHW);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t3\tUSE_ENFFW_PROTECT\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_USE_ENFFW_PROTECT) >> PCIEFLAGS_bitindex_USE_ENFFW_PROTECT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t4\tBLOCKTRIG\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_BLOCKTRIG) >> PCIEFLAGS_bitindex_BLOCKTRIG);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t5\tMEASUREON\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_MEASUREON) >> PCIEFLAGS_bitindex_MEASUREON);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t6\tBLOCK_EN\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_BLOCK_EN) >> PCIEFLAGS_bitindex_BLOCK_EN);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t8\tTDC\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_IS_TDC) >> PCIEFLAGS_bitindex_IS_TDC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t9\tEWS\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_IS_DSC) >> PCIEFLAGS_bitindex_IS_DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t10\tBLOCK_ON\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_BLOCK_ON) >> PCIEFLAGS_bitindex_BLOCK_ON);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t10\tBLOCK_ON_SYNCED\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_BLOCK_ON_SYNCED) >> PCIEFLAGS_bitindex_BLOCK_ON_SYNCED);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t26\tLinkup SFP3\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_linkup_sfp3) >> PCIEFLAGS_bitindex_linkup_sfp3);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t27\tError SFP3\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_error_sfp3) >> PCIEFLAGS_bitindex_error_sfp3);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t28\tLinkup SFP2\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_linkup_sfp2) >> PCIEFLAGS_bitindex_linkup_sfp2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t29\tError SFP2\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_error_sfp2) >> PCIEFLAGS_bitindex_error_sfp2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t30\tLinkup SFP1\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_linkup_sfp1) >> PCIEFLAGS_bitindex_linkup_sfp1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tError SFP1\t%"PRIu32"\n", (data32 & PCIEFLAGS_bit_error_sfp1) >> PCIEFLAGS_bitindex_error_sfp1);

	/*=======================================================================*/

	//Register NOS
	status = readRegisterS0_32(drvno, &data32, S0Addr_NOS);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x44\tNOS\t0-31\tNOS\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register SCANINDEX
	status = readRegisterS0_32(drvno, &data32, S0Addr_ScanIndex);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x48\tSCANINDEX\t0-30\tSCANINDEX\t%"PRIu32"\n", (data32 & ScanIndex_bits));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tCounter reset\t%"PRIu32"\n", (data32 & ScanIndex_bit_counter_reset) >> ScanIndex_bitindex_counter_reset);

	/*=======================================================================*/

	//Register DMABUFSIZEINSCANS
	status = readRegisterS0_32(drvno, &data32, S0Addr_DmaBufSizeInScans);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x4c\tDMABUFSIZEINSCANS\t0-30\tBuffer length\t%"PRIu32"\n", (data32 & DmaBufSizeInScans_bits));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tCounter reset\t%"PRIu32"\n", (data32 & DmaBufSizeInScans_bit_counter_reset) >> DmaBufSizeInScans_bitindex_counter_reset);

	/*=======================================================================*/

	//Register DMASPERINTERRUPT
	status = readRegisterS0_32(drvno, &data32, S0Addr_DMAsPerIntr);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x50\tDMASPERINTERRUPT\t0-30\tDMASPERINTERRUPT\t%"PRIu32"\n", (data32 & DMAsPerIntrs_bits));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tCounter reset\t%"PRIu32"\n", (data32 & DMAsPerIntr_bit_counter_reset) >> DMAsPerIntr_bitindex_counter_reset);

	/*=======================================================================*/

	//Register BLOCKS
	status = readRegisterS0_32(drvno, &data32, S0Addr_NOB);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x54\tBLOCKS\t0-31\tBLOCKS\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register BLOCKINDEX
	status = readRegisterS0_32(drvno, &data32, S0Addr_BLOCKINDEX);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x58\tBLOCKINDEX\t0-31\tINDEX\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register CAMCNT
	status = readRegisterS0_32(drvno, &data32, S0Addr_CAMCNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x5c\tCAMCNT\t0-3\tCAMCNT\t%"PRIu32"\n", (data32 & CAMCNT_bits) >> CAMCNT_bitindex_camcnt);

	/*=======================================================================*/

	//Register TDC Control
	status = readRegisterS0_32(drvno, &data32, S0Addr_TDCCtrl);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x60\tTDC Control\t0\tReset\t%"PRIu32"\n", (data32 & TDCCtrl_bit_reset) >> TDCCtrl_bitindex_reset);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t1\tInterrupt\t%"PRIu32"\n", (data32 & TDCCtrl_bit_interrupt) >> TDCCtrl_bitindex_interrupt);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tLoad fifo\t%"PRIu32"\n", (data32 & TDCCtrl_bit_load_fifo) >> TDCCtrl_bitindex_load_fifo);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t3\tEmpty fifo\t%"PRIu32"\n", (data32 & TDCCtrl_bit_empty_fifo) >> TDCCtrl_bitindex_empty_fifo);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t27\tCSR\t%"PRIu32"\n", (data32 & TDCCtrl_bit_cs) >> TDCCtrl_bitindex_cs);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t28-31\tADR\t%"PRIu32"\n", (data32 & 0xF0000000) >> TDCCtrl_bitindex_adr0);

	/*=======================================================================*/

	//Register TDC Data
	status = readRegisterS0_32(drvno, &data32, S0Addr_TDCData);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x64\tTDC Data\t0-31\tDelay\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register ROI0
	status = readRegisterS0_32(drvno, &data32, S0Addr_ROI0);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x68\tROI0\t0-15\trange 1\t%"PRIu32"\n", (data32 & ROI0_bits_range1) >> ROI0_bitindex_range1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t16-31\trange 2\t%"PRIu32"\n", (data32 & ROI0_bits_range2) >> ROI0_bitindex_range2);

	/*=======================================================================*/

	//Register ROI1
	status = readRegisterS0_32(drvno, &data32, S0Addr_ROI1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x6c\tROI1\t0-15\trange 3\t%"PRIu32"\n", (data32 & ROI1_bits_range3) >> ROI1_bitindex_range3);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t16-31\trange 4\t%"PRIu32"\n", (data32 & ROI1_bits_range4) >> ROI1_bitindex_range4);

	/*=======================================================================*/

	//Register ROI2
	status = readRegisterS0_32(drvno, &data32, S0Addr_ROI2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x70\tROI2\t0-15\trange 5\t%"PRIu32"\n", (data32 & ROI2_bits_range5) >> ROI2_bitindex_range5);

	/*=======================================================================*/

	//Register XCKDLY
	status = readRegisterS0_32(drvno, &data32, S0Addr_XCKDLY);
	uint64_t val = 500 + (((uint64_t)(data32 & XCKDELAY_bits)) * 10);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x74\tXCKDLY\t0-30\tDelay\t%"PRIu32" (%"PRIu64" ns)\n", (data32 & XCKDELAY_bits), val);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tEnabled\t%"PRIu32"\n", (data32 & XCKDELAY_bit_enable) >> XCKDELAY_bitindex_enable);

	/*=======================================================================*/

	//Register S1S2DLY
	status = readRegisterS0_32(drvno, &data32, S0Addr_S1S2ReadDelay);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x78\tS1S2DLY\t0-31\tDelay\t%"PRIu32" (%"PRIu64" ns)\n", data32, ((uint64_t)data32) * 10);

	/*=======================================================================*/

	//Register BTICNT
	status = readRegisterS0_32(drvno, &data32, S0Addr_BTICNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x7c\tBTICNT\t0-6\tBTICNT\t%"PRIu32"\n", data32 & BTICNT_bits_BTICNT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t7\tBTICNT enable\t%"PRIu32"\n", (data32 & BTICNT_bit_BTICNT_EN) >> BTICNT_bitindex_BTICNT_EN);

	/*=======================================================================*/

	//Register BTIMER
	status = readRegisterS0_32(drvno, &data32, S0Addr_BTIMER);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x80\tBTIMER\t0-27\tTimer\t%"PRIu32" μs\n", (data32 & BTIMER_bits));

	/*=======================================================================*/

	//Register BDAT
	status = readRegisterS0_32(drvno, &data32, S0Addr_BDAT);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x84\tBDAT\t0-30\tDelay After Trigger\t%"PRIu32" (%"PRIu64" ns)\n", (data32 & BDAT_bits_BDAT), (((uint64_t)(data32 & BDAT_bits_BDAT)) * 10));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tEnabled\t%"PRIu32"\n", (data32 & BDAT_bit_enable) >> BDAT_bitindex_enabled);

	/*=======================================================================*/

	//Register BEC
	status = readRegisterS0_32(drvno, &data32, S0Addr_BEC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x88\tBEC\t0-30\tExposure Control\t%"PRIu32" (%"PRIu64" ns)\n", (data32 & BEC_bits_BEC), (((uint64_t)(data32 & BEC_bits_BEC)) * 10));
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t31\tEnabled\t%"PRIu32"\n", (data32 & BEC_bit_enable) >> BEC_bitindex_enabled);

	/*=======================================================================*/

	//Register BFLAGS
	status = readRegisterS0_32(drvno, &data32, S0Addr_BSLOPE);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x8c\tBFLAGS\t0\tBSLOPE\t%"PRIu32"\n", (data32 & BSLOPE_bit_bslope) >> BSLOPE_bitindex_bslope);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t1\tBoth Slopes\t%"PRIu32"\n", (data32 & BSLOPE_bit_both_slopes) >> BSLOPE_bitindex_both_slopes);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t2\tBSWTRIG\t%"PRIu32"\n", (data32 & BSLOPE_bit_both_bswtrig) >> BSLOPE_bitindex_bswtrig);

	/*=======================================================================*/

	//Register A1DSC (Actual Delay Stage Counter)
	status = readRegisterS0_32(drvno, &data32, S0Addr_A1DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x90\tA1DSC\t0-31\tDSC\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register L1DSC (Last Delay Stage Counter)
	status = readRegisterS0_32(drvno, &data32, S0Addr_L1DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x94\tL1DSC\t0-31\tDSC\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register A2DSC
	status = readRegisterS0_32(drvno, &data32, S0Addr_A2DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x98\tA2DSC\t0-31\tDSC\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register L2DSC
	status = readRegisterS0_32(drvno, &data32, S0Addr_L2DSC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0x9c\tL2DSC\t0-31\tDSC\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register ATDC2
	status = readRegisterS0_32(drvno, &data32, S0Addr_ATDC2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xa0\tATDC2\t0-31\tTDC2\t%"PRIu32"\n", data32);
	/*=======================================================================*/

	//Register LTDC2
	status = readRegisterS0_32(drvno, &data32, S0Addr_LTDC2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xa4\tLTDC2\t0-31\tTDC2\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register DSCCtrl
	status = readRegisterS0_32(drvno, &data32, S0Addr_DSCCtrl);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xa8\tDSCCtrl\t0\tRS1\t%"PRIu32"\n", (data32 & DSCCtrl_bit_rs1) >> DSCCtrl_bitindex_rs1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t1\tDIR1\t%"PRIu32"\n", (data32 & DSCCtrl_bit_dir1) >> DSCCtrl_bitindex_dir1);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t8\tRS2\t%"PRIu32"\n", (data32 & DSCCtrl_bit_rs2) >> DSCCtrl_bitindex_rs2);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t9\tDIR2\t%"PRIu32"\n", (data32 & DSCCtrl_bit_dir2) >> DSCCtrl_bitindex_dir2);

	/*=======================================================================*/

	//Register DAC
	status = readRegisterS0_32(drvno, &data32, S0Addr_DAC);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xac\tDAC\t0-31\tDAC\t%"PRIu32"\n", data32);

	/*=======================================================================*/

	//Register XCKLEN
	status = readRegisterS0_32(drvno, &data32, S0Addr_XCKLEN);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xb0\tXCKLEN\t0-31\tXCKLEN\t%"PRIu32" (%"PRIu64" ns)\n", data32, ((uint64_t)data32) * 10);

	/*=======================================================================*/

	//Register BONLEN
	status = readRegisterS0_32(drvno, &data32, S0Addr_BONLEN);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xb4\tBONLEN\t0-31\tBONLEN\t%"PRIu32" (%"PRIu64" ns)\n", data32, ((uint64_t)data32) * 10);

	/*=======================================================================*/

	//Register Camera Type
	status = readRegisterS0_32(drvno, &data32, S0Addr_CAMERA_TYPE);

	//Sensor Type
	uint16_t lowerBitsCAMTYPE = (uint16_t)(data32 & camera_type_sensor_type_bits);
	uint16_t upperBitsCAMTYPE = (uint16_t)((data32 & camera_type_camera_system_bits) >> camera_type_camera_system_bit_index);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xb8\tCamera Type\t0-15\tSensor Type\t%"PRIu16" ", lowerBitsCAMTYPE);
	switch (lowerBitsCAMTYPE)
	{
	case sensor_type_pda:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "PDA");
		break;
	case sensor_type_ir:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "IR");
		break;
	case sensor_type_fft:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "FFT");
		break;
	case sensor_type_cmos:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "CMOS");
		break;
	case sensor_type_hsvis:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "HSVIS");
		break;
	case sensor_type_hsir:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "HSIR");
		break;
	default:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "invalid");
		break;
	}

	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\t\t16-31\tCamera System\t%"PRIu16" ", upperBitsCAMTYPE);
	//Camera System
	switch (upperBitsCAMTYPE)
	{
	case camera_system_3001:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "3001");
		break;
	case camera_system_3010:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "3010");
		break;
	case camera_system_3030:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "3030");
		break;
	default:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "invalid");
		break;
	}

	/*=======================================================================*/

	//Register BON PERIOD
	status = readRegisterS0_32(drvno, &data32, S0Addr_BON_PERIOD);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xbc\tBON PERIOD\t0-31\tBON PERIOD\t%"PRIu32" (%"PRIu64" ns)\n", data32, ((uint64_t)data32) * 10);

	/*=======================================================================*/

	//Register STATECTRL
	status = readRegisterS0_32(drvno, &data32, S0Addr_STATECTRL);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\n0xc0\tSTATECTRL\t0-3\ttrigger select\t%"PRIu32" ", data32);
	switch ((data32 & statectrl_bits_trigger_select) >> statectrl_bitindex_trigger_select)
	{
	case statectrl_trigger_select_manual:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "manual");
		break;
	case statectrl_trigger_select_sti:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "STI");
		break;
	case statectrl_trigger_select_sslope:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "SSlope");
		break;
	case statectrl_trigger_select_scan_gated:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "Scan Gated");
		break;
	case statectrl_trigger_select_sticnt:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "STICNT");
		break;
	case statectrl_trigger_select_sdat:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "SDAT");
		break;
	case statectrl_trigger_select_sec:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "SEC");
		break;
	case statectrl_trigger_select_xck:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "XCK");
		break;
	case statectrl_trigger_select_bti:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BTI");
		break;
	case statectrl_trigger_select_bslope:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BSlope");
		break;
	case statectrl_trigger_select_bticnt:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BTICNT");
		break;
	case statectrl_trigger_select_bdat:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BDAT");
		break;
	case statectrl_trigger_select_bec:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BEC");
		break;
	case statectrl_trigger_select_block_on:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BLOCK_ON");
		break;
	case statectrl_trigger_select_block_on_synced:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "BLOCK_ON_SYNCED");
		break;
	default:
	case statectrl_trigger_select_unused:
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "(%s)\n", "unused");
		break;
	}
	/*=======================================================================*/


	return status;
}

/**
 * @brief Read all DMA registers and write them to a string in hex.
 *
 * @param[in] drvno PCIe board identifier
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
 */
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
		status = readRegisterDma_32(drvno, &data, i * 4);
		if (status != es_no_error)
		{
			//write error to buffer
			len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register 0x%x %s", i * 4, register_names[i]);
			return status;
		}
		//write register name and value to buffer
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "0x%x  \t%s\t0x%x\n", i * 4, register_names[i], data);
	}
	return status;
}

/**
 * @brief Read all TLP registers and write them to a string.
 *
 * @param[in] drvno PCIe board identifier
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
 */
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
	uint32_t maxSizeEncoding[8] = { 128, 256, 512, 1024, 2048, 4096, 0, 0 };
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Max_Payload_Size Supported:\t0x%x (%"PRIu32" bytes)\n", data, maxSizeEncoding[data]);
	status = readConfig_32(drvno, &data, PCIeAddr_DeviceControl);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	uint32_t maxPayloadSize = (data & deviceControl_maxPayloadSize_bits) >> deviceControl_maxPayloadSize_bitindex;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Max_Payload_Size:\t0x%x (%"PRIu32" bytes)\n", maxPayloadSize, maxSizeEncoding[maxPayloadSize]);
	uint32_t maxReadRequestSize = (data & deviceControl_maxReadRequestSize_bits) >> deviceControl_maxReadRequestSize_bitindex;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Max_Read_Request_Size:\t0x%x (%"PRIu32" bytes)\n", maxReadRequestSize, maxSizeEncoding[maxReadRequestSize]);
	uint32_t pixel = 0;
	status = readRegisterS0_32(drvno, &pixel, S0Addr_PIXREG_FFCTRL_FFFLAGS);
	pixel &= 0xFFFF;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Number of pixels:\t%"PRIu32"\n", pixel);
	status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPS);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "TLPS in DMAReg is:\t%"PRIu32" (%"PRIu32" bytes)\n", data, data * 4);
	uint32_t numberOfTlps = 0;
	if (data)
		numberOfTlps = (pixel - 1) / (data * 2) + 1;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "number of TLPs per scan should be:\t(number of pixels - 1) / (TLPS in DMAReg * 2) + 1 \n\t= %"PRIu32" / %"PRIu32" + 1\n\t=%"PRIu32"\n", pixel - 1, data * 2, numberOfTlps);
	status = readRegisterDma_32(drvno, &data, DmaAddr_WDMATLPC);
	if (status != es_no_error)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nerror while reading register\n");
		return status;
	}
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "number of TLPs per scan is:\t%"PRIu32"", data);
	return status;
}

/**
 * @brief Reads registers 0 to 12 of TDC-GPX chip. Time delay counter option.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] stringPtr
 * @return @ref es_status_codes
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

/**
 * @brief Dump all measurement settings to a string.
 *
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
 */
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
		"board_sel\t%"PRIu32"\n"
		"nos\t%"PRIu32"\n"
		"nob\t%"PRIu32"\n"
		"cont_pause_in_microseconds\t%"PRIu32"\n",
		settings_struct.board_sel,
		settings_struct.nos,
		settings_struct.nob,
		settings_struct.cont_pause_in_microseconds);
	return es_no_error;
}

/**
 * @brief Dump all camera settings to a string.
 *
 * @param[in] drvno PCIe board identifier
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
 */
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
		"use_software_polling\t%"PRIu32"\n"
		"sti_mode\t%"PRIu32"\n"
		"bti_mode\t%"PRIu32"\n"
		"stime\t%"PRIu32"\n"
		"btime\t%"PRIu32"\n"
		"sdat_in_10ns\t%"PRIu32"\n"
		"bdat_in_10ns\t%"PRIu32"\n"
		"sslope\t%"PRIu32"\n"
		"bslope\t%"PRIu32"\n"
		"xckdelay_in_10ns\t%"PRIu32"\n"
		"sec_in_10ns\t%"PRIu32"\n"
		"trigger_mode_integrator\t%"PRIu32"\n"
		"sensor_type\t%"PRIu32"\n"
		"camera_system\t%"PRIu32"\n"
		"camcnt\t%"PRIu32"\n"
		"pixel\t%"PRIu32"\n"
		"is_fft_legacy\t%"PRIu32"\n"
		"led_off\t%"PRIu32"\n"
		"sensor_gain\t%"PRIu32"\n"
		"adc_gain\t%"PRIu32"\n"
		"temp_level\t%"PRIu32"\n"
		"bticnt\t%"PRIu32"\n"
		"gpx_offset\t%"PRIu32"\n"
		"fft_lines\t%"PRIu32"\n"
		"vfreq\t%"PRIu32"\n"
		"fft_mode\t%"PRIu32"\n"
		"lines_binning\t%"PRIu32"\n"
		"number_of_regions\t%"PRIu32"\n"
		"s1s2_read_delay_in_10ns\t%"PRIu32"\n",
		settings_struct.camera_settings[drvno].use_software_polling,
		settings_struct.camera_settings[drvno].sti_mode,
		settings_struct.camera_settings[drvno].bti_mode,
		settings_struct.camera_settings[drvno].stime,
		settings_struct.camera_settings[drvno].btime,
		settings_struct.camera_settings[drvno].sdat_in_10ns,
		settings_struct.camera_settings[drvno].bdat_in_10ns,
		settings_struct.camera_settings[drvno].sslope,
		settings_struct.camera_settings[drvno].bslope,
		settings_struct.camera_settings[drvno].xckdelay_in_10ns,
		settings_struct.camera_settings[drvno].sec_in_10ns,
		settings_struct.camera_settings[drvno].trigger_mode_integrator,
		settings_struct.camera_settings[drvno].sensor_type,
		settings_struct.camera_settings[drvno].camera_system,
		settings_struct.camera_settings[drvno].camcnt,
		settings_struct.camera_settings[drvno].pixel,
		settings_struct.camera_settings[drvno].is_fft_legacy,
		settings_struct.camera_settings[drvno].led_off,
		settings_struct.camera_settings[drvno].sensor_gain,
		settings_struct.camera_settings[drvno].adc_gain,
		settings_struct.camera_settings[drvno].temp_level,
		settings_struct.camera_settings[drvno].bticnt,
		settings_struct.camera_settings[drvno].gpx_offset,
		settings_struct.camera_settings[drvno].fft_lines,
		settings_struct.camera_settings[drvno].vfreq,
		settings_struct.camera_settings[drvno].fft_mode,
		settings_struct.camera_settings[drvno].lines_binning,
		settings_struct.camera_settings[drvno].number_of_regions,
		settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "region_size\t");
	for (int i = 0; i < MAX_NUMBER_OF_REGIONS; i++)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%"PRIu32" ", settings_struct.camera_settings[drvno].region_size[i]);
	for (int camera = 0; camera < MAXCAMCNT; camera++)
	{
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\ndac_output board %"PRIu32", camera %d\t", drvno, camera);
		for (int i = 0; i < DACCOUNT; i++)
			len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%"PRIu32" ", settings_struct.camera_settings[drvno].dac_output[camera][i]);
	}
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len,
		"\ntor\t%"PRIu32"\n"
		"adc_mode\t%"PRIu32"\n"
		"adc_custom_pattern\t%"PRIu32"\n"
		"bec_in_10ns\t%"PRIu32"\n"
		"channel_select\t%"PRIu32"\n"
		"ioctrl_impact_start_pixel\t%"PRIu32"\n",
		settings_struct.camera_settings[drvno].tor,
		settings_struct.camera_settings[drvno].adc_mode,
		settings_struct.camera_settings[drvno].adc_custom_pattern,
		settings_struct.camera_settings[drvno].bec_in_10ns,
		settings_struct.camera_settings[drvno].channel_select,
		settings_struct.camera_settings[drvno].ioctrl_impact_start_pixel);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "ioctrl_output_width_in_5ns\t");
	for (int i = 0; i < IOCTRL_OUTPUT_COUNT - 1; i++)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%"PRIu32" ", settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[i]);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "\nIOCtrl_output_delay_in_5ns\t");
	for (int i = 0; i < IOCTRL_OUTPUT_COUNT - 1; i++)
		len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "%"PRIu32" ", settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[i]);
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len,
		"\nIOCtrl_T0_period_in_10ns\t%"PRIu32"\n"
		"dma_buffer_size_in_scans\t%"PRIu32"\n"
		"tocnt\t%"PRIu32"\n"
		"sticnt\t%"PRIu32"\n"
		"sensor_reset_or_hsir_ec\t%"PRIu32"\n"
		"write_to_disc\t%"PRIu32"\n"
		"file_path\t%s\n"
		"shift_s1s2_to_next_scan\t%"PRIu32"\n"
		"is_cooled_camera_legacy_mode\t%"PRIu32"\n"
		"monitor\t%"PRIu32"\n"
		"manipulate_data_mode\t%"PRIu32"\n"
		"manipulate_data_custom_factor\t%f\n"
		"ec_legacy_mode\t%"PRIu32"\n"
		"timer_resolution_mode\t%"PRIu32"\n",
		settings_struct.camera_settings[drvno].ioctrl_T0_period_in_10ns,
		settings_struct.camera_settings[drvno].dma_buffer_size_in_scans,
		settings_struct.camera_settings[drvno].tocnt,
		settings_struct.camera_settings[drvno].sticnt,
		settings_struct.camera_settings[drvno].sensor_reset_or_hsir_ec,
		settings_struct.camera_settings[drvno].write_to_disc,
		settings_struct.camera_settings[drvno].file_path,
		settings_struct.camera_settings[drvno].shift_s1s2_to_next_scan,
		settings_struct.camera_settings[drvno].is_cooled_camera_legacy_mode,
		settings_struct.camera_settings[drvno].monitor,
		settings_struct.camera_settings[drvno].manipulate_data_mode,
		settings_struct.camera_settings[drvno].manipulate_data_custom_factor,
		settings_struct.camera_settings[drvno].ec_legacy_mode,
		settings_struct.camera_settings[drvno].timer_resolution_mode);
	return es_no_error;
}

/**
 * @brief Read all PCIe registers and write them to a string.
 *
 * @param[in] drvno PCIe board identifier
 * @param[out] stringPtr Pointer to a string buffer. The buffer will be allocated by this function. The caller is responsible to free the buffer.
 * @return @ref es_status_codes
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
	"Device ID, Vendor ID",
	"Status, Command",
	"Class Code, Revision ID",
	"BIST, Header Type, Lat. Timer, Cache Line S.",
	"Base Address 0 mem 32 bit",
	"Base Address 1 io",
	"Base Address 2 mem 32 bit",
	"Base Address 3",
	"Base Address 4",
	"Base Address 5",
	"Cardbus CIS Pointer",
	"Subsystem ID, Subsystem Vendor ID",
	"Expansion ROM Base Address",
	"Reserved, Cap. Pointer",
	"Reserved",
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
			length += sprintf_s(*stringPtr + length, bufferSize - (size_t)length, "\nerror while reading register %i %s", i * 4, register_names[i]);
			return status;
		}
		length += sprintf_s(*stringPtr + length, bufferSize - (size_t)length, "0x%x  \t%s\t0x%x\n", i * 4, register_names[i], data);
	}
	return status;
}

/**
* @brief Return infos about the PCIe board.
*
* - win1 : version of driver
* - win2 : ID = 53xx
* - win3 : length of space0 BAR =0x3f
* - win4 : vendor ID = EBST
* - win5 : PCI board version (same as label on PCI board)
* @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
* @param[out] stringPtr string with driver information is given back here
* @return @ref es_status_codes
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
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Board #%i\n", drvno);
	// 4 bytes of S0 address EBST + 1 byte 0 string termination
	char udata1[5] = {0};
	es_status_codes status = readRegisterS0_32(drvno, (uint32_t*)&udata1, S0Addr_EBST);
	if (status != es_no_error) return status;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "EBST register\t%s\n", udata1);
	status = readRegisterS0_32(drvno, &data, S0Addr_PCI);
	if (status != es_no_error) return status;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "FPGA version\t0x%x\n", data);
	// Get device ID and vendor ID
	status = readConfig_32(drvno, &data, PCIeAddr_VendorID);
	if (status != es_no_error) return status;
	len += sprintf_s(*stringPtr + len, bufferSize - (size_t)len, "Device ID, Vendor ID\t0x%x\n", data);
	return es_no_error;
}

/**
 * @brief reset Delay Stage Counter
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] DSCNumber 1: DSC 1; 2: DSC 2
 * @return @ref es_status_codes
 */
es_status_codes ResetDSC(uint32_t drvno, uint8_t DSCNumber)
{
	es_status_codes status;
	ES_LOG("Reset DSC %"PRIu8"\n", DSCNumber);
	uint32_t data = 0;
	switch (DSCNumber)
	{
	case 1: data = 0x1; break;
	case 2: data = 0x100; break;
	}
	//for reset you have to set a 1 to the reg and then a zero to allow a new start again
	status = writeBitsS0_32(drvno, data, data, S0Addr_DSCCtrl);
	if (status != es_no_error) return status;
	return writeBitsS0_32(drvno, 0, data, S0Addr_DSCCtrl);
}

/**
 * @brief set direction of Delay Stage Counter
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] DSCNumber 1: DSC 1; 2: DSC 2
 * @param[in] dir true: up; false: down
 * @return @ref es_status_codes
 */
es_status_codes SetDIRDSC(uint32_t drvno, uint8_t DSCNumber, bool dir)
{
	ES_LOG("set DSC %"PRIu8" in direction %d\n", DSCNumber, dir);
	uint32_t data = 0;
	switch (DSCNumber)
	{
	case 1: data = 0x2; break;
	case 2: data = 0x200; break;
	}

	if (dir)
		return writeBitsS0_32(drvno, data, data, S0Addr_DSCCtrl);
	else
		return writeBitsS0_32(drvno, 0, data, S0Addr_DSCCtrl);

}

/**
 * @brief return all values of Delay Stage Counter
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] DSCNumber 1: DSC 1; 2: DSC 2
 * @param[out] ADSC current DSC
 * @param[out] LDSC last DSC
 * @return @ref es_status_codes
 */
es_status_codes GetDSC(uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC)
{
	es_status_codes status;
	ES_LOG("get DSC %"PRIu8"\n", DSCNumber);
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

	status = readRegisterS0_32(drvno, ADSC, addrADSC);
	if (status != es_no_error) return status;
	return readRegisterS0_32(drvno, LDSC, addrLDSC);
}

/**
 * @brief This function copies valid data from DMA buffer to user buffer.
 *
 * This function tracks the DMA buffer and every time there is new data available, it is copied to the user buffer.
 * The memory of the DMA buffer which was copied is then set to 0. Create a new thread for this function. This function
 * should run parallel to the measurement. This function is only used when USE_SOFTWARE_POLLING is true.
 *
 * @param drvno_p Pointer to PCIe board identifier.
 */
void PollDmaBufferToUserBuffer(uint32_t* drvno_p)
{
	uint32_t drvno = *drvno_p;
	free(drvno_p);
	ES_LOG("Poll DMA buffer to user buffer started. drvno: %"PRIu32"\n", drvno);
	// Get the pointer to DMA buffer.
	uint16_t* dmaBuffer = getVirtualDmaAddress(drvno);
	ES_TRACE("DMA buffer address: %p\n", (void*)dmaBuffer);
	// Set dmaBufferReadPos pointer to base address of DMA buffer. dmaBufferReadPos indicates the current read position in the DMA buffer.
	uint16_t* dmaBufferReadPos = dmaBuffer;
	// Calculate pointer to the end of the DMA buffer.
	uint16_t* dmaBufferEnd = dmaBufferReadPos + getDmaBufferSizeInBytes(drvno) / sizeof(uint16_t);
	ES_TRACE("DMA buffer end: %p\n", (void*)dmaBufferEnd);
	// Calculate the size of the complete measurement in bytes.
	uint32_t dataToCopyInBytes = settings_struct.camera_settings[drvno].pixel * virtualCamcnt[drvno] * settings_struct.nos * settings_struct.nob * sizeof(uint16_t);
	ES_TRACE("Data to copy in bytes: %"PRIu32"\n", dataToCopyInBytes);
	// Calculate the size of one scan.
	uint32_t sizeOfOneScanInBytes = settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t);
	ES_TRACE("Size of one scan in bytes: %"PRIu32"\n", sizeOfOneScanInBytes);
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
	int64_t timebuffer_measurementStopped = GetTimestampInMilliseconds();
	int64_t timebuffer_now = GetTimestampInMilliseconds();
	while (!allDataCopied)
	{
		//ES_TRACE("dmaBufferReadPosNextScan: %p ", dmaBufferReadPosNextScan);
		// scan counter pixel are 4 and 5 and since P202_21 at the last pixel
		scanCounterHardware = (uint32_t)(dmaBufferReadPos[pixel_scan_index_high] << 16) | *(dmaBufferReadPos + settings_struct.camera_settings[drvno].pixel - 1);
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
			ES_TRACE("Data to copy: %"PRIu32"\n", dataToCopyInBytes);
			if (dataToCopyInBytes == 0) allDataCopied = true;
		}
		// Escape while loop after 100ms when Measurement stopped.
		if (!isRunning && !measurementStopped)
		{
			ES_LOG("Measurement aborted. Starting countdown for PollDmaBufferToUserBuffer\n");
			measurementStopped = true;
			timebuffer_measurementStopped = GetTimestampInMilliseconds();
		}
		if (measurementStopped)
		{
			timebuffer_now = GetTimestampInMilliseconds();
			int64_t diff_in_ms = (int64_t)(timebuffer_now - timebuffer_measurementStopped);
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
 * @brief Gives scan and block number of the last scan written to userBuffer.
 *
 * When settings parameter @ref camera_settings.use_software_polling is true this function converts scanCounterTotal to scan and block.
 * This is necessary, because scanCounterTotal is just counting each scan not regarding camcnt and blocks.
 * When @ref camera_settings.use_software_polling is false the scan and block number of the last interrupt is given.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] sample Scan number of the last scan in userBuffer. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * @param[out] block Block number of the last scan in userBuffer. -1 when no scans has been written yet, otherwise 0...(nob-1)
 */
void GetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block)
{
	GetScanNumber(drvno, 0, sample, block);
	return;
}

/**
 * @copydoc GetCurrentScanNumber
 * @param offset from current scan number
 */
void GetScanNumber(uint32_t drvno, int64_t offset, int64_t* sample, int64_t* block)
{
	int64_t scanCount = 0;
	uint32_t dmasPerInterrupt = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans / DMA_BUFFER_PARTS;
	if (settings_struct.camera_settings[drvno].use_software_polling)
		scanCount = scanCounterTotal[drvno];
	else
		scanCount = getCurrentInterruptCounter(drvno) * dmasPerInterrupt;
	//ES_TRACE("scan counter %"PRId64", settings_struct.nos %u, camcnt %u\n", scanCount + offset, settings_struct.nos, virtualCamcnt[drvno]);
	int64_t samples_done_all_cams = scanCount - 1 + offset;
	int64_t samples_done_per_cam = samples_done_all_cams / virtualCamcnt[drvno];
	*block = samples_done_per_cam / settings_struct.nos;
	int64_t count_of_scans_of_completed_blocks = *block * settings_struct.nos;
	*sample = samples_done_per_cam - count_of_scans_of_completed_blocks;
	//ES_TRACE("block %li, scan %li, samples_done_all_cams %li, samples_done_per_cam %li, count_of_scans_of_completed_blocks %li\n", *block, *sample, samples_done_all_cams, samples_done_per_cam, count_of_scans_of_completed_blocks);
	return;
}

/**
 * @brief Set the scan trigger input divider
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param divider
 *		- =0: disable this function (every trigger is used)
 *		- >0: omit n trigger
 * @return @ref es_status_codes
 */
es_status_codes SetSticnt(uint32_t drvno, uint8_t divider)
{
	ES_LOG("Set STICNT to %"PRIu8"\n", divider);
	// If divider is not 0, set the enable bit to 1
	if (divider)
		divider |= TOR_bit_STICNT_EN;
	return writeBitsS0_32(drvno, divider << TOR_bitindex_STICNT, TOR_bits_STICNT | TOR_bit_STICNT_EN, S0Addr_TOR_STICNT_TOCNT);
}

/**
 * @brief Set the block trigger input divider
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param divider
 *		- =0: disable this function (every trigger is used)
 *		- >0: omit n trigger
 * @return @ref es_status_codes
 */
es_status_codes SetBticnt(uint32_t drvno, uint8_t divider)
{
	ES_LOG("Set BTICNT to %"PRIu8"\n", divider);
	// If divider is not 0, set the enable bit to 1
	if (divider)
		divider |= BTICNT_bit_BTICNT_EN;
	return writeRegisterS0_32(drvno, (uint32_t)divider, S0Addr_BTICNT);
}

/**
 * @brief Set the trigger output divider
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param divider 7 bit value
 *		- =0: disable this function (every trigger is used)
 *		- >0: use every n'th trigger
 * @return @ref es_status_codes
 */
es_status_codes SetTocnt(uint32_t drvno, uint8_t divider)
{
	ES_LOG("Set TOCNT to %"PRIu8"\n", divider);
	// If divider is not 0, set the enable bit to 1
	if (divider)
		divider |= TOR_bit_TOCNT_EN;
	return writeBitsS0_32(drvno, divider << TOR_bitindex_TOCNT, TOR_bits_TOCNT | TOR_bit_TOCNT_EN, S0Addr_TOR_STICNT_TOCNT);
}

/**
 * @brief This function inserts data to user buffer for developing purpose.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 */
void FillUserBufferWithDummyData(uint32_t drvno)
{
	ES_LOG("Fill user buffer with dummy data, drvno %"PRIu32"\n", drvno);
	//memset(userBuffer[drvno], 0xAAAA, settings_struct.camera_settings[drvno].pixel * settings_struct.nos * settings_struct.nob * virtualCamcnt[drvno] * sizeof(uint16_t));
	for (uint32_t scan = 0; scan < settings_struct.nos * settings_struct.nob * virtualCamcnt[drvno]; scan++)
	{
		int add = (scan % 2) * 1000;
		for (uint32_t pixel = 0; pixel < settings_struct.camera_settings[drvno].pixel; pixel++)
			userBuffer[drvno][scan * settings_struct.camera_settings[drvno].pixel + pixel] = (uint16_t)(pixel * 10 + add);
	}
	return;
}

/**
 * @brief Read TDC flag in PCIEFLAGS register.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] isTdc	TDC flag is written to this bool*. TRUE: TDC board detected, FALSE: no TDC board detected
 * @return @ref es_status_codes
 */
es_status_codes GetIsTdc(uint32_t drvno, bool* isTdc)
{
	ES_LOG("Get is TDC, drvno %"PRIu32"\n", drvno);
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
 * @brief Read DSC flag in PCIEFLAGS register.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] isDsc DSC flag is written to this bool*. TRUE: DSC board detected, FALSE: no DSC board detected
 * @return @ref es_status_codes
 */
es_status_codes GetIsDsc(uint32_t drvno, bool* isDsc)
{
	ES_LOG("Get is DSC, drvno %"PRIu32"\n", drvno);
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

/**
 * @brief Check the consistency of the file given in vd and return the results in resultString.
 * 
 * @param[in] vd Pointer to a verify_data_parameter struct. The member filename_full must be set.
 * @param[out] resultString Pointer to a char*. The result string is written to this pointer. The buffer is allocated in this function and must be freed by the caller.
 */
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
	if (vd->error_cnt) len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data inconsistent, check counters below\n\n");
	else len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data found as expected\n\n");
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data found in file header:\n");
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "software_version_major:\t%"PRIu32"\n", vd->fh.software_version_major);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "software_version_pcie:\t%"PRIu32"\n", vd->fh.software_version_pcie);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "software_version_minor:\t%"PRIu32"\n", vd->fh.software_version_minor);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "number_of_boards:\t%"PRIu32"\n", vd->fh.number_of_boards);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "drvno:\t%"PRIu32"\n", vd->fh.drvno);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "pixel:\t%"PRIu32"\n", vd->fh.pixel);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "nos:\t%"PRIu32"\n", vd->fh.nos);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "nob:\t%"PRIu32"\n", vd->fh.nob);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "camcnt:\t%"PRIu32"\n", vd->fh.camcnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "measurement cnt:\t%"PRIu64"\n", vd->fh.measurement_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "timestamp:\t%s\n", vd->fh.timestamp);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "Data found:\n");
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "samples found:\t%"PRIu32"\n", vd->sample_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "blocks found:\t%"PRIu32"\n", vd->block_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "measurements found:\t%"PRIu64"\n", vd->measurement_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "error counter:\t%"PRIu32"\n", vd->error_cnt);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last sample in data:\t%"PRIu32"\n", vd->last_sample);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last block in data:\t%"PRIu32"\n", vd->last_block);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last sample before error:\t%"PRIu32"\n", vd->last_sample_before_error);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last block before error:\t%"PRIu32"\n", vd->last_block_before_error);
	len += sprintf_s(*resultString + len, bufferLength - (size_t)len, "last measurement before error:\t%"PRIu64"\n", vd->last_measurement_before_error);
	return;
}

/**
 * @brief Control looping the measurement.
 *
 * @param[in] on 1: measurement runs in a loop, 0: measurement only runs once.
 */
void SetContinuousMeasurement(bool on)
{
	ES_LOG("Set continuous measurement to %"PRIu32"\n", on);
	continuousMeasurementFlag = on;
	return;
}

/**
 * @brief This function returns the bit overTemp of a specific scan.
 *
 * The information over temperature is given in the special pixel camera status (pixel_camera_status) in bit pixel_camera_status_bit_over_temp.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] overTemp Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * @return @ref es_status_codes
 */
es_status_codes GetCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp)
{
	ES_TRACE("Get camera status over temperature, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (!overTemp) return es_invalid_pointer;
	uint16_t data = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_camera_status, 1, &data);
	if (status != es_no_error) return status;
	if (data & pixel_camera_status_bit_over_temp)
		*overTemp = true;
	else
		*overTemp = false;
	return status;
}

/**
 * @brief This function returns the bit tempGood of a specific scan.
 *
 * The information temperature good is given in the special pixel camera status (pixel_camera_status) in bit pixel_camera_status_bit_temp_good. This bit is used only in cooled cameras.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] tempGood Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * @return @ref es_status_codes
 */
es_status_codes GetCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood)
{
	ES_TRACE("Get camera status temp good, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (!tempGood) return es_invalid_pointer;
	uint16_t data = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_camera_status, 1, &data);
	if (status != es_no_error) return status;
	if (data & pixel_camera_status_bit_temp_good)
		*tempGood = true;
	else
		*tempGood = false;
	return status;
}

/**
 * @brief This function returns the block index of a specific scan.
 *
 * The information block index is given in the special pixels pixel_block_index_low and pixel_block_index_high_s1_s2.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] blockIndex Pointer to a uint32_t, where the information block index will be written. Block index is a 30 bit counter, so the highest two bits are not used.
 * @return @ref es_status_codes
 */
es_status_codes GetBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex)
{
	ES_TRACE("Get block index, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (!blockIndex) return es_invalid_pointer;
	uint16_t blockIndexHigh = 0;
	uint16_t blockIndexLow = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_block_index_low, 1, &blockIndexLow);
	if (status != es_no_error) return status;
	status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_block_index_high_s1_s2, 1, &blockIndexHigh);
	if (status != es_no_error) return status;
	blockIndexHigh &= pixel_block_index_high_s1_s2_bits_block_index;
	*blockIndex = (uint32_t)blockIndexHigh << 16 | blockIndexLow;
	return status;
}

/**
 * @brief This function returns the scan index of a specific scan.
 *
 * The information block index is given in the special pixels pixel_scan_index_low and pixel_scan_index_high.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] scanIndex Pointer to a uint32_t, where the information scan index will be written. Scan index is a 32 bit counter.
 * @return @ref es_status_codes
 */
es_status_codes GetScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex)
{
	ES_TRACE("Get scan index, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (!scanIndex) return es_invalid_pointer;
	uint16_t scanIndexHigh = 0;
	uint16_t scanIndexLow = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_scan_index_low, 1, &scanIndexLow);
	if (status != es_no_error) return status;
	status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_scan_index_high, 1, &scanIndexHigh);
	if (status != es_no_error) return status;
	*scanIndex = (uint32_t)scanIndexHigh << 16 | (uint32_t)scanIndexLow;
	return status;
}

/**
 * @brief This function returns the bit S1 state of a specific scan.
 *
 * The information S1 is given in the special pixel pixel_block_index_high_s1_s2 in bit pixel_block_index_high_s1_s2_bit_s1.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] state Pointer to a bool, where the information S1 state will be written. true - S1 is high, false - S1 is low
 * @return @ref es_status_codes
 */
es_status_codes GetS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	ES_TRACE("Get S1 state, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (!state) return es_invalid_pointer;
	uint16_t data = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_block_index_high_s1_s2, 1, &data);
	if (status != es_no_error) return status;
	if (data & pixel_block_index_high_s1_s2_bit_s1)
		*state = true;
	else
		*state = false;
	return status;
}

/**
 * @brief This function returns the bit S2 state of a specific scan.
 *
 * The information S2 is given in the special pixel pixel_block_index_high_s1_s2 in bit pixel_block_index_high_s1_s2_bit_s2.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] state Pointer to a bool, where the information S2 state will be written. true - S2 is high, false - S2 is low
 * @return @ref es_status_codes
 */
es_status_codes GetS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	ES_TRACE("Get S2 state, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (!state) return es_invalid_pointer;
	uint16_t data = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_block_index_high_s1_s2, 1, &data);
	if (status != es_no_error) return status;
	if (data & pixel_block_index_high_s1_s2_bit_s2)
		*state = true;
	else
		*state = false;
	return status;
}

/**
 * @brief This function returns the impact signal 1 of a specific scan.
 *
 * The information impact signal 1 is given in the special pixels pixel_impact_signal_1_low and pixel_impact_signal_1_high. Impact signal 1 is either TDC 1 or DSC 1, depending on the PCIe daughter board.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] impactSignal Pointer to a uint32_t, where the information impact signal will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	ES_TRACE("Get impact signal 1, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	uint16_t data_high = 0;
	uint16_t data_low = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_impact_signal_1_high, 1, &data_high);
	if (status != es_no_error) return status;
	status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_impact_signal_1_low, 1, &data_low);
	if (status != es_no_error) return status;
	*impactSignal = (uint32_t)data_high << 16 | (uint32_t)data_low;
	return status;
}

/**
 * @brief This function returns the impact signal 2 of a specific scan.
 *
 * The information impact signal 2 is given in the special pixels pixel_impact_signal_2_low and pixel_impact_signal_2_high. Impact signal 2 is either TDC 2 or DSC 2, depending on the PCIe daughter board.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] impactSignal Pointer to a uint32_t, where the information impact signal will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	ES_TRACE("Get impact signal 2, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	uint16_t data_high = 0;
	uint16_t data_low = 0;
	es_status_codes status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_impact_signal_2_high, 1, &data_high);
	if (status != es_no_error) return status;
	status = CopyDataArbitrary(drvno, sample, block, camera_pos, pixel_impact_signal_2_low, 1, &data_low);
	if (status != es_no_error) return status;
	*impactSignal = (uint32_t)data_high << 16 | (uint32_t)data_low;
	return status;
}

/**
 * @brief This function returns the all special pixel information of a specific scan.
 *
 * The information impact signal 2 is given in the special pixels pixel_impact_signal_2_low and pixel_impact_signal_2_high. Impact signal 2 is either TDC 2 or DSC 2, depending on the PCIe daughter board.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] sample sample number (0 ... (nos-1))
 * @param[in] block block number (0 ... (nob-1))
 * @param[in] camera_pos camera position (0 ... (CAMCNT-1))
 * @param[out] sp struct special_pixels Pointer to struct special_pixel, where all special pixel information will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	ES_TRACE("Get all special pixel information, drvno %"PRIu32", sample %"PRIu32", block %"PRIu32", camera_pos %"PRIu16"\n", drvno, sample, block, camera_pos);
	if (settings_struct.camera_settings[drvno].pixel <= 63) return es_invalid_pixel_count;
	uint16_t* data = (uint16_t*)malloc(settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t));
	if (!data) return es_allocating_memory_failed;
	es_status_codes status = CopyOneSample(drvno, sample, block, camera_pos, data);
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
	sp->scanIndex2 = (uint32_t)data[(settings_struct.camera_settings[drvno].pixel - 1) - pixel_scan_index2_high] << 16 | (uint32_t)data[(settings_struct.camera_settings[drvno].pixel - 1) - pixel_scan_index2_low];
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
 * @brief Reads the ScanFrequency bit and checks if its high or low.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] scanFrequencyTooHigh True when scanFrequency bit is set
 * @return @ref es_status_codes
 */
es_status_codes ReadScanFrequencyBit(uint32_t drvno, bool* scanFrequencyTooHigh)
{
	ES_TRACE("Read scan frequency bit, drvno %"PRIu32"\n", drvno);
	return ReadBitS0_32(drvno, S0Addr_PIXREG_FFCTRL_FFFLAGS, FF_FLAGS_bitindex_scan_read, scanFrequencyTooHigh);
}

/**
 * @brief Resets the ScanFrequency bit.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ResetScanFrequencyBit(uint32_t drvno)
{
	ES_TRACE("Reset scan frequency bit, drvno %"PRIu32"\n", drvno);
	return pulseBitS0_32(drvno, FFCTRL_bitindex_scan_reset, S0Addr_PIXREG_FFCTRL_FFFLAGS, 100);
}

/**
 * @brief Reads the BlockFrequency bit and checks if its high or low.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] blockFrequencyTooHigh True when BlockFrequency bit is set.
 * @return @ref es_status_codes
 */
es_status_codes ReadBlockFrequencyBit(uint32_t drvno, bool* blockFrequencyTooHigh)
{
	ES_TRACE("Read block frequency bit, drvno %"PRIu32"\n", drvno);
	return ReadBitS0_32(drvno, S0Addr_PIXREG_FFCTRL_FFFLAGS, FF_FLAGS_bitindex_block_read, blockFrequencyTooHigh);
}

/**
 * @brief Resets the BlockFrequency bit.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ResetBlockFrequencyBit(uint32_t drvno)
{
	ES_TRACE("Reset block frequency bit, drvno %"PRIu32"\n", drvno);
	return pulseBitS0_32(drvno, FFCTRL_bitindex_block_reset, S0Addr_PIXREG_FFCTRL_FFFLAGS, 100);
}

/**
 * @brief Copy the data of one block of one camera to pdest.
 * 
 * If @ref camera_settings.camcnt is 1, use CopyOneBlock instead. This function copies the data sample by sample because the data of one block of one camera is not stored in a contiguous memory block if camcnt is greater than 1.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] block block number ( 0...(nob - 1) )
 * @param[in] camera camera number ( 0...(CAMCNT - 1) )
 * @param[out] pdest Pointer where the data will be written to. Make sure that the size of the buffer is >= sizeof(uint16_t) * pixel * nos
 * @return @ref es_status_codes
 */
es_status_codes CopyOneBlockOfOneCamera(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t* pdest)
{
	if (!pdest) return es_invalid_pointer;
	es_status_codes status = es_no_error;
	// iterate through all samples of the requested block
	for (uint32_t sample = 0; sample < settings_struct.nos; sample++)
	{
		uint16_t* sample_address = NULL;
		// get the address of the current sample
		status = GetOneSamplePointer(drvno, sample, block, camera, &sample_address, NULL);
		if (status != es_no_error) return status;
		// check if sample_address is not null
		if (sample_address)
			// copy one sample to the new memory
			memcpy(pdest + sample * settings_struct.camera_settings[drvno].pixel, sample_address, settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t));
	}
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
 * @brief Initializes region of interest.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param number_of_regions determines how many region of interests are initialized, choose 2 to 8
 * @param lines number of total lines in camera
 * @param region_size determines the size of each region. array of size number_of_regions.
 * 	When region_size[0]==0 the lines are equally distributed for all regions.
 * 	I don't know what happens when  region_size[0]!=0 and region_size[1]==0. Maybe don't do this.
 * 	The sum of all regions should equal lines.
 * @param vfreq VCLK frequency
 * @return @ref es_status_codes
 */
es_status_codes SetupROI(uint32_t drvno, uint16_t number_of_regions, uint32_t lines, uint8_t* region_size, uint8_t vfreq)
{
	es_status_codes status = es_no_error;
	// calculate how many lines are in each region when equally distributed
	uint32_t lines_per_region = lines / number_of_regions;
	// calculate the rest of lines when equally distributed
	uint32_t lines_in_last_region = lines - lines_per_region * (number_of_regions - 1);
	ES_LOG("Setup ROI: lines_per_region: %"PRIu32", lines_in_last_region: %"PRIu32"\n", lines_per_region, lines_in_last_region);
	// go from region 1 to number_of_regions
	for (int i = 1; i <= number_of_regions; i++)
	{
		// check whether lines should be distributed equally or by custom region size
		if (*region_size == 0)
		{
			if (i == number_of_regions) status = SetupVPB(drvno, i, lines_in_last_region);
			else status = SetupVPB(drvno, i, lines_per_region);
		}
		else
		{
			status = SetupVPB(drvno, i, *(region_size + (i - 1)));
		}
		if (status != es_no_error) return status;
	}
	status = SetupVCLKReg(drvno, lines, vfreq);
	if (status != es_no_error) return status;
	status = SetPartialBinning(drvno, 0); //I don't know why there first is 0 written, I just copied it from Labview. - FH
	if (status != es_no_error) return status;
	status = SetPartialBinning(drvno, number_of_regions);
	return status;
}

/**
 * @brief For FFTs: Setup area mode.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param lines_binning Determines how many lines are binned (summed) when reading camera in area mode.
 * @param vfreq Frequency for vertical clock.
 * @return @ref es_status_codes
 */
es_status_codes SetupArea(uint32_t drvno, uint32_t lines_binning, uint8_t vfreq)
{
	ES_LOG("Setup Area\n");
	es_status_codes status = SetupVCLKReg(drvno, lines_binning, vfreq);
	if (status != es_no_error) return status;
	return ResetPartialBinning(drvno);
}


/**
 * @brief This functions sets the register S1S2ReadDealy with the setting @ref camera_settings.s1s2_read_delay_in_10ns.
 *
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes SetS1S2ReadDelay(uint32_t drvno)
{
	ES_LOG("Set S1 & S2 read delay to %"PRIu32"\n", settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns);
	return writeRegisterS0_32(drvno, settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns, S0Addr_S1S2ReadDelay);
}

#ifndef MINIMAL_BUILD

/**
 * @brief Export the measurement data to a file.
 * 
 * Depending on the file extension, the data is saved in a binary or HDF5 file.
 * 
 * @param[in] filename Filename with complete absolute path and either .bin or .h5 extension.
 * @return @ref es_status_codes
 */
es_status_codes SaveMeasurementDataToFile(const char* filename)
{
	if (isRunning) return es_measurement_running;
	// Check if memory is initialized yet
	for (uint32_t drvno = 0; drvno < MAXPCIECARDS; drvno++)
	{
		if (settings_struct.board_sel >> drvno & 1 && !userBuffer[drvno])
			return es_memory_not_initialized;
	}
	// Check filename for filetype
	char* extension = strrchr(filename, '.');
	if (extension == NULL) return es_invalid_file_extention;
	else if (strcmp(extension, ".h5") == 0) return SaveMeasurementDataToFileHDF5(filename);
	else if (strcmp(extension, ".bin") == 0) return SaveMeasurementDataToFileBIN(filename);
	else return es_invalid_file_extention;
}

/**
 * @brief TODO. missing documentation.
 * 
 * @param[in] filename
 * @return 
 */
es_status_codes ImportMeasurementDataFromFile(const char* filename)
{
	if (isRunning) return es_measurement_running;
	// Check filename for filetype
	char* extension = strrchr(filename, '.');
	if (extension == NULL) return es_invalid_file_extention;
	//else if (strcmp(extension, ".h5") == 0) return ImportMeasurementDataFromFileHDF5(filename);
	else if (strcmp(extension, ".bin") == 0) return ImportMeasurementDataFromFileBIN(filename);
	else return es_invalid_file_extention;
}

es_status_codes ImportMeasurementDataFromFileBIN(const char* filename)
{
	struct file_header fh;
	getFileHeaderFromFile(&fh, filename);
	// Apply settings from header
	settings_struct.board_sel = fh.board_sel;
	settings_struct.nos = fh.nos;
	settings_struct.nob = fh.nob;
	uint32_t drvno = fh.drvno;
	settings_struct.camera_settings[drvno].camcnt = fh.camcnt;
	settings_struct.camera_settings[drvno].pixel = fh.pixel;
	es_status_codes status = InitSoftware(drvno);
	if (status != es_no_error) return status;
	status = CopyFromFileToUserBufferBIN(filename);
	return es_no_error;
}
/**
 * @brief Exports the measurement data to a HDF5 file.
 *
 * @param filename Filename with complete absolute path.
 * @return @ref es_status_codes
 */
es_status_codes SaveMeasurementDataToFileHDF5(const char* filename)
{
	ES_LOG("Export measurement to HDF5 file\n");
	hid_t file_id;
	hid_t dataspace_scalar = H5Screate(H5S_SCALAR);
	herr_t statusHDF5;
	es_status_codes status;

	if ((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) == H5I_INVALID_HID) return es_create_file_failed;

	uint32_t major_version = VERSION_MAJOR_ESCAM;
	hid_t file_attr_version_major = CreateNumericAttribute(file_id, "Major version", H5T_NATIVE_UINT32, dataspace_scalar, &major_version);
	H5Aclose(file_attr_version_major);
	uint32_t pcie_version = VERSION_PCIE_BOARD_VERSION;
	hid_t file_attr_version_pcie = CreateNumericAttribute(file_id, "Pcie version", H5T_NATIVE_UINT32, dataspace_scalar, &pcie_version);
	H5Aclose(file_attr_version_pcie);
	uint32_t minor_version = VERSION_MINOR_ESCAM;
	hid_t file_attr_version_minor = CreateNumericAttribute(file_id, "Minor version", H5T_NATIVE_UINT32, dataspace_scalar, &minor_version);
	H5Aclose(file_attr_version_minor);
	hid_t file_attr_number_of_boards = CreateNumericAttribute(file_id, "Number of Boards", H5T_NATIVE_UINT8, dataspace_scalar, &number_of_boards);
	H5Aclose(file_attr_number_of_boards);
	char* timestamp[1] = { start_timestamp };
	hid_t file_attr_timestamp = CreateStringAttribute(file_id, "Timestamp", dataspace_scalar, timestamp);
	H5Aclose(file_attr_timestamp);


	for (uint32_t drvno = 0; drvno < MAXPCIECARDS; drvno++)
	{
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			hid_t group_board_id = 0,
				group_board_attr_number_of_cameras = 0;

			char groupBoardName[100];
			sprintf_s(groupBoardName, 100, "/Board_%"PRIu32, drvno + 1);
			group_board_id = H5Gcreate(file_id, groupBoardName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
			uint32_t cameras = settings_struct.camera_settings[drvno].camcnt;

			group_board_attr_number_of_cameras = CreateNumericAttribute(group_board_id, "Number of Cameras", H5T_NATIVE_UINT32, dataspace_scalar, &cameras);
			H5Aclose(group_board_attr_number_of_cameras);

			for (uint32_t camera = 0; camera < cameras; camera++)
			{
				hid_t group_camera_id = 0,
					group_camera_attr_system = 0,
					group_camera_attr_number_of_blocks = 0;

				char groupCameraName[100];
				sprintf_s(groupCameraName, 100, "Camera_%"PRIu32, camera + 1);
				group_camera_id = H5Gcreate(group_board_id, groupCameraName, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

				uint32_t nos = settings_struct.nos;
				uint32_t nob = settings_struct.nob;

				uint32_t camera_system = settings_struct.camera_settings[drvno].camera_system;
				char* camera_system_string;

				if (camera_system == 0)
				{
					camera_system_string = "3001";
				}
				else if (camera_system == 1)
				{
					camera_system_string = "3010";
				}
				else if (camera_system == 2)
				{
					camera_system_string = "3030";
				}
				else
				{
					camera_system_string = "Unknown";
				}

				char* camera_system_attr[1] = { camera_system_string };
				group_camera_attr_system = CreateStringAttribute(group_camera_id, "Camera System", dataspace_scalar, camera_system_attr);
				H5Aclose(group_camera_attr_system);

				group_camera_attr_number_of_blocks = CreateNumericAttribute(group_camera_id, "Number of Blocks", H5T_NATIVE_UINT32, dataspace_scalar, &nob);
				H5Aclose(group_camera_attr_number_of_blocks);

				for (uint32_t block = 0; block < nob; block++)
				{
					hid_t group_block_id = 0,
						group_block_attr_number_of_samples = 0,
						group_block_gcpl = 0;

					group_block_gcpl = H5Pcreate(H5P_GROUP_CREATE);
					statusHDF5 = H5Pset_link_creation_order(group_block_gcpl, (H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED));

					// Create a group in the file.
					// Create a String that contains the name of the group with block
					char groupBlockName[100];
					sprintf_s(groupBlockName, 100, "Block_%"PRIu32, block + 1);
					group_block_id = H5Gcreate(group_camera_id, groupBlockName, H5P_DEFAULT, group_block_gcpl, H5P_DEFAULT);
					H5Pclose(group_block_gcpl);

					group_block_attr_number_of_samples = CreateNumericAttribute(group_block_id, "Number of Samples", H5T_NATIVE_UINT32, dataspace_scalar, &nos);
					H5Aclose(group_block_attr_number_of_samples);

					for (uint32_t sample = 0; sample < nos; sample++)
					{
						// Define the size of the array and create the data space for fixed size dataset.
						uint32_t number_of_sensor_pixels = settings_struct.camera_settings[drvno].pixel - pixel_number_of_special_pixels;
						hsize_t dims[1];
						dims[0] = number_of_sensor_pixels;
						hid_t dataspace_id = H5Screate_simple(1, dims, NULL);

						char datasetName[100];
						sprintf_s(datasetName, 100, "Sample_%"PRIu32, sample + 1);
						hid_t dataset_id = H5Dcreate2(group_block_id, datasetName, H5T_NATIVE_UINT16, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

						uint16_t* data = (uint16_t*)malloc(number_of_sensor_pixels * sizeof(uint16_t));
						status = CopyDataArbitrary(drvno, sample, block, (uint16_t)camera, pixel_first_sensor_pixel, number_of_sensor_pixels, data);
						if (status != es_no_error) return status;
						statusHDF5 = H5Dwrite(dataset_id, H5T_NATIVE_UINT16, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

						free(data);

						struct special_pixels sp;
						status = GetAllSpecialPixelInformation(drvno, sample, block, (uint16_t)camera, &sp);

						// Create the attribute for the special pixels
						hid_t sample_attr_s1State = CreateNumericAttribute(dataset_id, "S1 State", H5T_NATIVE_UINT32, dataspace_scalar, &sp.s1State);
						H5Aclose(sample_attr_s1State);

						hid_t sample_attr_s2State = CreateNumericAttribute(dataset_id, "S2 State", H5T_NATIVE_UINT32, dataspace_scalar, &sp.s2State);
						H5Aclose(sample_attr_s2State);

						hid_t sample_attr_blockIndex = CreateNumericAttribute(dataset_id, "Block Index", H5T_NATIVE_UINT32, dataspace_scalar, &sp.blockIndex);
						H5Aclose(sample_attr_blockIndex);

						hid_t sample_attr_scanIndex = CreateNumericAttribute(dataset_id, "Scan Index", H5T_NATIVE_UINT32, dataspace_scalar, &sp.scanIndex);
						H5Aclose(sample_attr_scanIndex);

						hid_t sample_attr_scanIndex2 = CreateNumericAttribute(dataset_id, "Scan Index 2", H5T_NATIVE_UINT32, dataspace_scalar, &sp.scanIndex2);
						H5Aclose(sample_attr_scanIndex2);

						hid_t sample_attr_impactSignal1 = CreateNumericAttribute(dataset_id, "Impact Signal 1", H5T_NATIVE_UINT32, dataspace_scalar, &sp.impactSignal1);
						H5Aclose(sample_attr_impactSignal1);

						hid_t sample_attr_impactSignal2 = CreateNumericAttribute(dataset_id, "Impact Signal 2", H5T_NATIVE_UINT32, dataspace_scalar, &sp.impactSignal2);
						H5Aclose(sample_attr_impactSignal2);

						hid_t sample_attr_overTemp = CreateNumericAttribute(dataset_id, "Over Temperature", H5T_NATIVE_UINT32, dataspace_scalar, &sp.overTemp);
						H5Aclose(sample_attr_overTemp);

						hid_t sample_attr_tempGood = CreateNumericAttribute(dataset_id, "Temperature Good", H5T_NATIVE_UINT32, dataspace_scalar, &sp.tempGood);
						H5Aclose(sample_attr_tempGood);

						hid_t sample_attr_cameraSystem3001 = CreateNumericAttribute(dataset_id, "Camera System 3001", H5T_NATIVE_UINT32, dataspace_scalar, &sp.cameraSystem3001);
						H5Aclose(sample_attr_cameraSystem3001);

						hid_t sample_attr_cameraSystem3010 = CreateNumericAttribute(dataset_id, "Camera System 3010", H5T_NATIVE_UINT32, dataspace_scalar, &sp.cameraSystem3010);
						H5Aclose(sample_attr_cameraSystem3010);

						hid_t sample_attr_cameraSystem3030 = CreateNumericAttribute(dataset_id, "Camera System 3030", H5T_NATIVE_UINT32, dataspace_scalar, &sp.cameraSystem3030);
						H5Aclose(sample_attr_cameraSystem3030);

						char fpgaVer[100];
						sprintf_s(fpgaVer, 100, "%"PRIu32".%"PRIu32"", sp.fpgaVerMajor, sp.fpgaVerMinor);
						char* special_pixel_fpgaver[1] = { fpgaVer };
						hid_t sample_attr_fpgaVer = CreateStringAttribute(dataset_id, "FPGA Version", dataspace_scalar, special_pixel_fpgaver);
						H5Aclose(sample_attr_fpgaVer);


						// Close the dataset and dataspace.
						statusHDF5 = H5Dclose(dataset_id);
						statusHDF5 = H5Sclose(dataspace_id);

					}
					hsize_t n_links;
					H5Gget_num_objs(group_block_id, &n_links);
					for (hsize_t i = 0; i < n_links; i++)
					{
						char link_name[100];
						H5Lget_name_by_idx(group_block_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, i, link_name, 100, H5P_DEFAULT);
						//ES_LOG("Link name: %s\n", link_name);
					}

					statusHDF5 = H5Gclose(group_block_id);
				}
				statusHDF5 = H5Gclose(group_camera_id);
			}
			statusHDF5 = H5Gclose(group_board_id);
		}
	}
	// Close the file.
	statusHDF5 = H5Fclose(file_id);
	// supress warning
	statusHDF5 = statusHDF5;
	return es_no_error;
}

hid_t CreateNumericAttribute(hid_t parent_object_id, char* attr_name, hid_t goal_type, hid_t dataspace, void* data)
{
	hid_t attr_type = H5Tcopy(goal_type);

	hid_t object_id = H5Acreate(parent_object_id, attr_name, attr_type, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(object_id, attr_type, data);
	return object_id;
}

hid_t CreateStringAttribute(hid_t parent_object_id, char* attr_name, hid_t dataspace, void* data)
{
	hid_t attr_type = H5Tcopy(H5T_C_S1);
	H5Tset_size(attr_type, H5T_VARIABLE);
	H5Tset_strpad(attr_type, H5T_STR_NULLTERM);
	H5Tset_cset(attr_type, H5T_CSET_UTF8);

	hid_t object_id = H5Acreate(parent_object_id, attr_name, attr_type, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(object_id, attr_type, data);
	return object_id;
}

#endif

/**
 * @brief Get the high time duration of XCK from the S0 register @ref S0Addr_XCKLEN.
 * 
 * The signal is measured once per measurement. The fist valid value can be read after the first completed XCK.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] xckLengthIn10ns pointer to uint32 where the XCK length is returned
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * @return @ref es_status_codes
 */
es_status_codes GetXckLength(uint32_t drvno, uint32_t* xckLengthIn10ns)
{
	ES_TRACE("Get XCK length\n");
	return readRegisterS0_32(drvno, xckLengthIn10ns, S0Addr_XCKLEN);
}

/**
 * @brief Get pos edge to pos egde time of XCK time from the S0 register @ref S0Addr_XCK_PERIOD.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the start of the second XCK.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] xckPeriodIn10ns pointer to uint32 where the XCK period is returned
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * @return @ref es_status_codes
 */
es_status_codes GetXckPeriod(uint32_t drvno, uint32_t* xckPeriodIn10ns)
{
	ES_TRACE("Get XCK period\n");
	return readRegisterS0_32(drvno, xckPeriodIn10ns, S0Addr_XCK_PERIOD);
}

/**
 * @brief Get the high time duration of BON from the S0 register @ref S0Addr_BONLEN.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the first completed BON.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] bonLengthIn10ns pointer to uint32 where the BON length is returned
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * @return @ref es_status_codes
 */
es_status_codes GetBonLength(uint32_t drvno, uint32_t* bonLengthIn10ns)
{
	ES_TRACE("Get BON length\n");
	return readRegisterS0_32(drvno, bonLengthIn10ns, S0Addr_BONLEN);
}

/**
 * @brief Get the pos edge to pos edge time of BON from the S0 register @ref S0Addr_BON_PERIOD.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the start of the second BON.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] bonPeriodIn10ns pointer to uint32 where the BON period is returned
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * @return @ref es_status_codes
 */
es_status_codes GetBonPeriod(uint32_t drvno, uint32_t* bonPeriodIn10ns)
{
	ES_TRACE("Get BON period\n");
	return readRegisterS0_32(drvno, bonPeriodIn10ns, S0Addr_BON_PERIOD);
}

/**
 * @brief Get the PCIe card firmware version number.
 * 
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param major_version Pointer to a uint16_t, where the major version number will be written.
 * @param minor_version Pointer to a uint16_t, where the minor version number will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetPcieCardVersion(uint32_t drvno, uint16_t* major_version, uint16_t* minor_version)
{
	uint32_t data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &data, S0Addr_PCI);
	if (status != es_no_error) return status;
	*minor_version = (data & PCI_bits_minor_version) >> PCI_bitindex_minor_version;
	*major_version = (data & PCI_bits_major_version) >> PCI_bitindex_major_version;
	return status;
}

bool PcieCardVersionIsGreaterThan(uint32_t drvno, uint16_t major_version, uint16_t minor_version)
{
	if ((pcieCardMajorVersion[drvno] > major_version) || ((pcieCardMajorVersion[drvno] == major_version) && (pcieCardMinorVersion[drvno] > minor_version)))
		return true;
	else
		return false;
}

bool PcieCardVersionIsSmallerThan(uint32_t drvno, uint16_t major_version, uint16_t minor_version)
{
	if ((pcieCardMajorVersion[drvno] < major_version) || ((pcieCardMajorVersion[drvno] == major_version) && (pcieCardMinorVersion[drvno] < minor_version)))
		return true;
	else
		return false;
}

bool PcieCardVersionIsEqual(uint32_t drvno, uint16_t major_version, uint16_t minor_version)
{
	if ((pcieCardMajorVersion[drvno] == major_version) && (pcieCardMinorVersion[drvno] == minor_version))
		return true;
	else
		return false;
}

/**
 * @brief Get the block on bit from the PCIe flags register.
 * 
 * Since the block on bit position was change in 222.14 this function looks at a different bit depending on the firmware version.
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] blockOn Pointer to a bool, where the block on bit will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetBlockOn(uint32_t drvno, bool* blockOn)
{
	ES_TRACE("Get block on bit\n");
	es_status_codes status;
	// In PCIe card firmware version 222.14 the block_on bit was renamed from BLOCK_ON to BLOCK_EN and BLOCK_ON was added as a new bit.
	if(PcieCardVersionIsSmallerThan(drvno, 0x222, 0x14))
		status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_BLOCK_EN, blockOn);
	else
		status = ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_BLOCK_ON, blockOn);
	return status;
}

/**
 * @brief Read the bit @ref PCIEFLAGS_bits_t.PCIEFLAGS_bit_scan_trigger_detected.
 * 
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] detected Pointer to a bool, where the scan trigger detected bit will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetScanTriggerDetected(uint32_t drvno, bool* detected)
{
	ES_TRACE("Get scan trigger detected bit\n");
	return ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_scan_trigger_detected, detected);
}

/**
 * @brief Read the bit @ref PCIEFLAGS_bits_t.PCIEFLAGS_bit_block_trigger_detected.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[out] detected Pointer to a bool, where the block trigger detected bit will be written.
 * @return @ref es_status_codes
 */
es_status_codes GetBlockTriggerDetected(uint32_t drvno, bool* detected)
{
	ES_TRACE("Get block trigger detected bit\n");
	return ReadBitS0_32(drvno, S0Addr_PCIEFLAGS, PCIEFLAGS_bitindex_block_trigger_detected, detected);
}

/**
 * @brief Reset the bit @ref PCIEFLAGS_bits_t.PCIEFLAGS_bit_scan_trigger_detected to 0.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ResetScanTriggerDetected(uint32_t drvno)
{
	ES_TRACE("Reset scan trigger detected bit\n");
	return pulseBitS0_32(drvno, PCIEFLAGS_bitindex_reset_scan_trigger_detected, S0Addr_PCIEFLAGS, 10);
}

/**
 * @brief Reset the bit @ref PCIEFLAGS_bits_t.PCIEFLAGS_bit_block_trigger_detected to 0.
 *
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @return @ref es_status_codes
 */
es_status_codes ResetBlockTriggerDetected(uint32_t drvno)
{
	ES_TRACE("Reset block trigger detected bit\n");
	return pulseBitS0_32(drvno, PCIEFLAGS_bitindex_reset_block_trigger_detected, S0Addr_PCIEFLAGS, 10);
}

es_status_codes WaitForBlockOn(uint32_t drvno)
{
	ES_LOG("Wait for block on\n");
	bool blockOn = false;
	es_status_codes status = es_no_error;
	while (!blockOn && !abortMeasurementFlag)
	{
		status = GetBlockOn(drvno, &blockOn);
		if (status != es_no_error) return status;
		if (checkEscapeKeyState())
		{
			abortMeasurementFlag = true;
			return es_abortion;
		}
	}
	return status;
}

es_status_codes SetShiftS1S2ToNextScan(uint32_t drvno)
{
	return writeBitsS0_32(drvno, (1 & settings_struct.camera_settings[drvno].shift_s1s2_to_next_scan) << CTRL_bitindex_shift_s, CTRL_bit_shift_s, S0Addr_CTRL);
}

/**
 * @brief Manipulate the incoming data buffer with a preset polynomial.
 * 
 * @param drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param startAddress
 * @param numberOfScansToManipulate
 */
void manipulateData(uint32_t drvno, uint16_t* startAddress, uint32_t numberOfScansToManipulate)
{
	double a, b, c, d, e, oldValue, newValue;
	// Decide which manipulation mode is used
	switch (settings_struct.camera_settings[drvno].manipulate_data_mode)
	{
	default:
	case(manipulate_data_mode_none):
		return;
	case(manipulate_data_mode_preset_linearization_polynom):
		a = 0.0000001,
		b = 0.0000001,
		c = 0.000001,
		d = 0.1,
		e = 0.001;
		break;
	case(manipulate_data_mode_custom_factor):
		a = 0.,
		b = 0.,
		c = 0.,
		d = settings_struct.camera_settings[drvno].manipulate_data_custom_factor,
		e = 0.;
		break;
	}
	// Iterate through all scans
	for (uint32_t scan = 0; scan < numberOfScansToManipulate; scan++)
	{
		// Iterate through all pixels and leave out the special pixels
		for (uint32_t i = pixel_first_sensor_pixel; i < settings_struct.camera_settings[drvno].pixel - pixel_last_sensor_pixel; i++)
		{
			uint32_t pixel = scan * settings_struct.camera_settings[drvno].pixel + i;
			oldValue = (double)startAddress[pixel];
			// Calculate the new manipulated value with the given factors and a polynomial of degree 4
			newValue = a * oldValue * oldValue * oldValue * oldValue + b * oldValue * oldValue * oldValue + c * oldValue * oldValue + d * oldValue + e;
			// Check if the new value is in the range of a uint16_t
			if (newValue > UINT16_MAX)
				startAddress[pixel] = UINT16_MAX;
			else if (newValue < 0)
				startAddress[pixel] = 0;
			else
				startAddress[pixel] = (uint16_t)newValue;
		}
	}
}

void clearKeyStates()
{
	checkEscapeKeyState();
	checkSpaceKeyState();
	return;
}

/**
 * \brief Control the general outputs of the PCIe card addition board.
 * 
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * \param[in] output 0 ... 7
 * \param[in] state true = high, false = low
 * \return @ref es_status_codes
 */
es_status_codes SetGeneralOutput(uint32_t drvno, uint8_t output, bool state)
{
	ES_LOG("Set general output %"PRIu8" to state %d\n", output, state);
	if (output > 7) return es_parameter_out_of_range;
	if(state)
		return setBitS0_32(drvno, output, S0Addr_GIOREG);
	else
		return resetBitS0_32(drvno, output, S0Addr_GIOREG);
}

/**
 * @brief Sets all shutter states in one call.
 * 
 * @param[in] drvno identifier of PCIe card, 0 ... @ref MAXPCIECARDS, when there is only one PCIe board: always 0
 * @param[in] shutter_states 16 bit value where each bit of the lower 4 represents the state of one shutter.
 *		* bit 0: shutter 1, 1: closed, 0: open
 *		* bit 1: shutter 2, 1: closed, 0: open
 *		* bit 2: shutter 3, 1: closed, 0: open
 *		* bit 3: shutter 4, 1: closed, 0: open
 * @return @ref es_status_codes
 */
es_status_codes SetShutterStates(uint32_t drvno, uint16_t shutter_states)
{
	ES_LOG("Set shutter states to 0x%"PRIx16", drvno %"PRIu32"\n", shutter_states, drvno);
	return Cam_SendData(drvno, maddr_ioctrl, ioctrl_shutter, shutter_states);
}

es_status_codes SetStateControlRegister(uint32_t drvno, uint16_t state)
{
	ES_LOG("Set state control register to 0x%"PRIx16", drvno %"PRIu32"\n", state, drvno);
	return writeBitsS0_32(drvno, (state << statectrl_bitindex_trigger_select), statectrl_bits_trigger_select, S0Addr_STATECTRL);
}

es_status_codes SetManualState(uint32_t drvno, bool state)
{
	ES_LOG("Set manual state to %d, drvno %"PRIu32"\n", state, drvno);
	if(state)
		return setBitS0_32(drvno, statectrl_bitindex_manual_trigger, S0Addr_STATECTRL);
	else
		return resetBitS0_32(drvno, statectrl_bitindex_manual_trigger, S0Addr_STATECTRL);
}
