#include "../crossPlattformBoard_ll.h"
#include <cstdint>
#include <stdint.h>

***REMOVED***#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"

#include "Jungo/windrvr.h"
#include "Jungo/wdc_lib.h"
#include "Jungo/wdc_defs.h"

WDC_DEVICE_HANDLE hDev_tmp[MAXPCIECARDS];
WDC_DEVICE_HANDLE* hDev = &hDev_tmp;
DWORD dmaBufferSizeInBytes = 0;
uint16_t* dmaBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };

/**
\brief This call comes every DMASPERINTR=500 here a DMASubBuf could be copied to the DMABigBuf.
the size of a drivers continous memory is limited, so we must copy it via this small buf to the big buf
The INTR occurs every DMASPERINTR and copies this block of scans in lower/upper half blocks.
*/
void isr( uint32_t drvno, void* pData )
{
	WDC_Err( "*isr(): 0x%x\n", IsrCounter );
	es_status_codes status = SetS0Bit( IRQFLAGS_bitindex_INTRSR, S0Addr_IRQREG, drvno );//set INTRSR flag for TRIGO
	if (status != es_no_error) return;
	//! be sure not to stop run before last isr is ready - or last part is truncated
	//usually dmaBufferSizeInBytes = 1000scans 
	//dmaBufferPartSizeInBytes = 1000 * pixel *2 -> /2 = 500 scans = 1088000 bytes
	// that means one 500 scan copy block has 1088000 bytes
	//!GS sometimes (all 10 minutes) one INTR more occurs -> just do not serve it and return
	// Fehler wenn zu viele ISRs -> memcpy out of range
	WDC_Err( "ISR Counter : 0x%x \n", IsrCounter );
	if (IsrCounter > numberOfInterrupts)
	{
		WDC_Err( "numberOfInterrupts: 0x%x \n", numberOfInterrupts );
		WDC_Err( "ISR Counter overflow: 0x%x \n", IsrCounter );
		status = ResetS0Bit( IRQFLAGS_bitindex_INTRSR, S0Addr_IRQREG, drvno );//reset INTRSR flag for TRIGO
		return;
	}
	WDC_Err("dmaBufferSizeInBytes: 0x%x \n", dmaBufferSizeInBytes);
	size_t dmaBufferPartSizeInBytes = dmaBufferSizeInBytes / DMA_BUFFER_PARTS; //1088000 bytes
	UINT16* dmaBufferReadPos = dmaBuffer[drvno] + dmaBufferPartReadPos[drvno] * dmaBufferPartSizeInBytes / sizeof(UINT16);
	//here the copyprocess happens
	memcpy( userBufferWritePos[drvno], dmaBufferReadPos, dmaBufferPartSizeInBytes );
	WDC_Err( "userBufferWritePos: 0x%x \n", userBufferWritePos[drvno] );
	dmaBufferPartReadPos[drvno]++;
	if (dmaBufferPartReadPos[drvno] >= DMA_BUFFER_PARTS)		//number of ISR per dmaBuf - 1
		dmaBufferPartReadPos[drvno] = 0;						//dmaBufferPartReadPos is 0 or 1 for buffer devided in 2 parts
	userBufferWritePos[drvno] += dmaBufferPartSizeInBytes / sizeof( UINT16 );
	status = ResetS0Bit( IRQFLAGS_bitindex_INTRSR, S0Addr_IRQREG, drvno );//reset INTRSR flag for TRIGO
	IsrCounter++;
	return;
}

void DLLCALLCONV interrupt_handler1( void* pData ) { isr( 1, pData ); }
void DLLCALLCONV interrupt_handler2( void* pData ) { isr( 2, pData ); }

/**
 * @brief Reads long on DMA area.
 *
 * @param drvno PCIe board identifier
 * @param data buffer for data
 * @param address Offset from BaseAdress - in Bytes ! 0..3= Regs of Board.
 * @return es_status_codes
	- es_no_error
	- es_register_read_failed
 */
es_status_codes readRegister_32(uint32_t drvno, uint32_t* data, uint16_t address)
{
	volatile DWORD dwStatus = WDC_ReadAddrBlock(hDev[drvno], 0, address, sizeof(uint32_t), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG("ReadLongDMA in address 0x%x failed\n", address);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_read_failed;
	}
	return es_no_error;
};

/**
 * @brief Read byte (8 bit) from register in space0 of PCIe board, except r10-r1f.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param data pointer to where data is stored
 * @param address offset of register from base address (count in bytes)
 * @return es_status_codes
	- es_no_error
	- es_register_read_failed
 */
es_status_codes readRegister_8(uint32_t drvno, uint8_t* data, uint16_t address)
{
	volatile DWORD dwStatus = WDC_ReadAddrBlock(hDev[drvno], 0, address, sizeof(uint8_t), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG("ReadByteS0 in address 0x%x failed\n", address);
		return es_register_read_failed;
	}
	return es_no_error;
};

/**
 * \brief Writes long to space0 register.
 *
 * \param drvno PCIe board identifier.
 * \param data data to write
 * \param address Register offset from BaseAdress - in bytes
 * \return es_status_codes
	- es_no_error
	- es_register_write_failed
 */
es_status_codes writeRegister_32(uint32_t drvno, uint32_t data, uint16_t address)
{
	DWORD dwStatus = WDC_WriteAddrBlock(hDev[drvno], 0, address, sizeof(uint32_t), &data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG("WriteLongDMA in address 0x%x with data: 0x%x failed\n", address, data);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_write_failed;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", address, data);
	return es_no_error;
};


/**
* \brief Write byte (8 bit) to register in space0 of PCIe board, except r10-r1f.
*
* \param drv board number (=1 if one PCI board)
* \param data byte value to write
* \param address Offset drom BaseAdress of register (count in bytes)
* \return es_status_codes
	- es_no_error
	- es_register_write_failed
*/
es_status_codes writeRegister_8(uint32_t drv, uint8_t data, uint16_t address)
{
	volatile DWORD dwStatus = WDC_WriteAddrBlock(hDev[drv], 0, address, sizeof(BYTE), &data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG("WriteByteS0 in address 0x%x with data: 0x%x failed\n", address, data);
		return es_register_write_failed;
	}//else WDC_Err("ByteS0Write /t address /t0x%x /t data: /t0x%x \n", address, data);
	return es_no_error;
};  // WriteByteS0

/**
 * Check drvno for beeing legit
 * 
 * \param drvno driver number
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 */
es_status_codes checkDriverHandle(uint32_t drvno)
{
	if ((drvno < 1) || (drvno > 2))
		return es_invalid_driver_number;
	else if (hDev[drvno] == INVALID_HANDLE_VALUE)
	{
		ES_LOG("Handle is invalid of drvno: %i", drvno);
		return es_invalid_driver_handle;
	}
	else
		return es_no_error;
}

uint64_t getDmaAddress( uint32_t drvno)
{
    WD_DMA** ppDma = &dmaBufferInfos[drvno];
	return (*ppDma)->Page[0].pPhysicalAddr;
}

/**
 * \brief Alloc DMA buffer - should only be called once.
 * 
 * Gets address of DMASubBuf from driver and copy it later to our pDMABigBuf.
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_getting_dma_buffer_failed
 * 		- es_register_read_failed
 *		- es_register_write_failed
 *		- es_enabling_interrupts_failed
 */
es_status_codes SetupDma( uint32_t drvno )
{
	DWORD dwStatus;
	ES_LOG( "Setup DMA\n" );
	//If interrupt was enabled before, first cleanup DMA.
	if (WDC_IntIsEnabled(hDev[drvno]))
	{
		ES_LOG("Cleanup DMA\n");
		es_status_codes status = CleanupPCIE_DMA(drvno);
		if (status != es_no_error) return status;
	}
	dmaBufferSizeInBytes = DMA_BUFFER_SIZE_IN_SCANS * aPIXEL[drvno] * sizeof( UINT16 );
	DWORD dwOptions = DMA_FROM_DEVICE | DMA_KERNEL_BUFFER_ALLOC;// | DMA_ALLOW_64BIT_ADDRESS;// DMA_ALLOW_CACHE ;
	if (DMA_64BIT_EN)
		dwOptions |= DMA_ALLOW_64BIT_ADDRESS;
//usually we use contig buf: here we get the buffer address from labview.
#if DMA_CONTIGBUF
	// dmaBuffer is the space which is allocated by this function = output - must be global
	dwStatus = WDC_DMAContigBufLock( hDev[drvno], &dmaBuffer[drvno], dwOptions, dmaBufferSizeInBytes, &dmaBufferInfos[drvno] ); //size in Bytes
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG( "Failed locking a contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		return es_getting_dma_buffer_failed;
	}
	// data must be copied afterwards to user Buffer 
#else
	if (!pDMABigBuf)
	{
		ES_LOG( "Failed: buf pointer not valid.\n" );
		return es_getting_dma_buffer_failed;
	}
	// pDMABigBuf is the big space which is passed to this function = input - must be global
	dwStatus = WDC_DMASGBufLock( hDev[drvno], pDMABigBuf, dwOptions, dmaBufferSizeInBytes, &dmaBufferInfos ); //size in Bytes
#endif
}

es_status_codes enableInterrupt( uint32_t drvno )
{
	switch (drvno)
	{
	case 1:
		dwStatus = LSCPCIEJ_IntEnable( hDev[drvno], interrupt_handler1 );
		if (WD_STATUS_SUCCESS != dwStatus)
		{
			ES_LOG( "Failed to enable the Interrupts1. Error 0x%lx - %s\n",	dwStatus, Stat2Str( dwStatus ) );
			return es_enabling_interrupts_failed;
		}
		break;

	case 2:
		dwStatus = LSCPCIEJ_IntEnable( hDev[drvno], interrupt_handler2 );
		if (WD_STATUS_SUCCESS != dwStatus)
		{
			ES_LOG( "Failed to enable the Interrupts2. Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
			return es_enabling_interrupts_failed;
		}
		break;
	default:
		return es_parameter_out_of_range;
	}
	return;
}

void ResetBufferWritePos(uint32_t drvno)
{
    dmaBufferPartReadPos[drvno] = 0;
	// reset buffer index to base we got from InitDMA
	userBufferWritePos[drvno] = userBuffer[drvno];
	ES_LOG( "RESET userBufferWritePos to %x\n", userBufferWritePos[drvno] );
	IsrCounter = 0;
    return;
}

void copyRestData(size_t rest_in_bytes)
{
	ES_LOG( "Copy rest data:\n" );
	ES_LOG( "dmaBufferSizeInBytes: 0x%x \n", dmaBufferSizeInBytes );
	INT_PTR dmaBufferReadPos = dmaBuffer[drvno];
	dmaBufferReadPos += dmaBufferPartReadPos[drvno] * dmaBufferSizeInBytes / DMA_BUFFER_PARTS;
	memcpy( userBufferWritePos[drvno], dmaBufferReadPos, rest_in_bytes );
	return;
}

es_status_codes _InitBoard(uint32_t drvno)
{
	if ((drvno < 1) || (drvno > 2)) return es_invalid_driver_number;
	//PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
	volatile DWORD dwStatus = 0;
	ES_LOG( "Info: scan result: a board found:%lx , dev=%lx, ven=%lx \n", scanResult.dwNumDevices, scanResult.deviceId[drvno - 1].dwDeviceId, scanResult.deviceId[drvno - 1].dwVendorId );
	//gives the information received from PciScanDevices to PciGetDeviceInfo
	BZERO( deviceInfo[drvno] );
	memcpy( &deviceInfo[drvno].pciSlot, &scanResult.deviceSlot[drvno - 1], sizeof( deviceInfo[drvno].pciSlot ) );

	/* Retrieve the device's resources information */
	dwStatus = WDC_PciGetDeviceInfo( &deviceInfo[drvno] );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG( "DeviceOpen: Failed retrieving the device's resources information.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_DriverClose();
		return es_getting_device_info_failed;
	}

	ES_LOG( "Info: device info: bus:%lx , slot=%lx, func=%lx \n", deviceInfo[drvno].pciSlot.dwBus, deviceInfo[drvno].pciSlot.dwSlot, deviceInfo[drvno].pciSlot.dwFunction );
	ES_LOG( "Info: device info: items:%lx , item[0]=%lx \n", deviceInfo[drvno].Card.dwItems, deviceInfo[drvno].Card.Item[0] );

	hDev[drvno] = LSCPCIEJ_DeviceOpen( &deviceInfo[drvno] );
	if (!hDev[drvno])
	{
		ES_LOG( "DeviceOpen: Failed opening a handle to the device: %s\n", LSCPCIEJ_GetLastErr() );
		WDC_DriverClose();
		return es_open_device_failed;
	}
	PWDC_DEVICE pDev = ((PWDC_DEVICE)hDev[drvno]);
	ES_LOG( "DRVInit hDev id % x, hDev pci slot %x, hDev pci bus %x, hDev pci function %x, hDevNumAddrSp %x \n"	, pDev->id, pDev->slot.dwSlot, pDev->slot.dwBus, pDev->slot.dwFunction, pDev->dwNumAddrSpaces );
	InitProDLL();
	return es_no_error ;
}

es_status_codes _InitDriver()
{
	//depends on os, how big a buffer can be
	bool fResult = false;
	char AString[80] = "";
	HANDLE hccddrv = INVALID_HANDLE_VALUE;
	//PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
	volatile DWORD dwStatus = 0;
	PLSCPCIEJ_DEV_CTX pDevCtx = NULL;

#if KER_MODE
	LSCPCIEJ_DEV_ADDR_DESC devAddrDesc;
#endif

#if defined(WD_DRIVER_NAME_CHANGE)
	/* Set the driver name */
	if (!WD_DriverName( LSCPCIEJ_STRESING_DRIVER_NAME ))
	{
		ErrLog( "Failed to set the driver name for WDC library.\n" );
		return es_setting_driver_name_failed;
	}
#endif
	/* Set WDC library's debug options (default: level TRACE, output to Debug Monitor) */
	ES_LOG("set debug options\n");
#if defined(_DEBUG)
	dwStatus = WDC_SetDebugOptions( WDC_DBG_DEFAULT, NULL );
#else
	dwStatus = WDC_SetDebugOptions( WDC_DBG_NONE, NULL );
#endif
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG( "Failed to initialize debug options for WDC library.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_DriverClose();
		return es_debug_init_failed;
	}
	/* Open a handle to the driver and initialize the WDC library */
	ES_LOG("open WDC\n");
	//No WDC Err messages can be sent to debug monitor before WDC_DriverOpen call
***REMOVED***	ES_LOG("\n*** Init driver ***\n");
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "Failed to initialize the WDC library. Error 0x%lx - %s\n",
			dwStatus, Stat2Str( dwStatus ) );
		//doesnt work at this moment before debugsetup
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "Failed to initialize the WDC library. Maybe the driver was not unloaded correctly.\n" );
		WDC_DriverClose();
		return es_driver_init_failed;
	}
	BZERO( scanResult );
	WDC_Err("Scan PCIe devices\n");
	dwStatus = WDC_PciScanDevices( LSCPCIEJ_DEFAULT_VENDOR_ID, LSCPCIEJ_DEFAULT_DEVICE_ID, &scanResult ); //VendorID, DeviceID
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "DeviceFind: Failed scanning the PCI bus.\n"
			"Error: 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "Device not found" );
		WDC_DriverClose();
		ErrorMsg( "driver closed.\n" );
		return es_device_not_found;
	}
	number_of_boards = (UINT8) scanResult.dwNumDevices;
	WDC_Err("Init HR counter\n");
	TPS = InitHRCounter();//for ticks function
	return es_no_error;	  // no Error, driver found
}

es_status_codes _ExitDriver(uint32_t drvno)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	ES_LOG( "Driver exit, drv: %u\n", drvno);
	if (WDC_IntIsEnabled( hDev[drvno] ))
	{
		ES_LOG( "cleanup dma\n" );
		status = CleanupPCIE_DMA( drvno );
		if (status != es_no_error) return status;
	}
	WDC_DriverClose();
	WDC_PciDeviceClose( hDev[drvno] );
	ES_LOG( "Driver closed and PciDeviceClosed \n" );
	//if (ahCCDDRV[drvno]!=INVALID_HANDLE_VALUE)
	//CloseHandle(ahCCDDRV[drvno]);	   // close driver
	return status;
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
 * @brief Read long (32 bit) from runtime register of PCIe board.
 *  
 * This function reads the memory mapped data , not the I/O Data. Reads data from PCIe conf space.
 * 
 * @param drvno board number (=1 if one PCI board)
 * @param data pointer to where data is stored
 * @param address offset of register (count in bytes)
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint16_t address )
{
	uint8_t dwStatus = WDC_PciReadCfg( hDev[drvno], address, data, sizeof( uint32_t ) );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "ReadLongIOPort in address 0x%x failed\n", address );
		//old message...i kept it because i dont know what it does
		ErrorMsg( "Read IORunReg failed" );
		return es_register_read_failed;
	}
	return es_no_error;
}