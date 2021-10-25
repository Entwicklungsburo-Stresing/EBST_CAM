#include "../Board_ll.h"
#include <stdint.h>
#include "../Board.h"

***REMOVED***#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"

#include "Jungo/windrvr.h"
#include "Jungo/wdc_defs.h"
#include "shared_src/lscpciej_lib.h"
#include "shared_src/ESLSCDLL_pro.h"

WDC_DEVICE_HANDLE hDev_tmp[MAXPCIECARDS];
WDC_DEVICE_HANDLE* hDev = &hDev_tmp;
DWORD dmaBufferSizeInBytes = 0;
uint16_t* dmaBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
DWORD64 IsrCounter = 0;
UINT8 dmaBufferPartReadPos[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
WD_DMA* dmaBufferInfos[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL }; //there will be saved the neccesary parameters for the dma buffer
WDC_PCI_SCAN_RESULT scanResult;
WD_PCI_CARD_INFO deviceInfo[MAXPCIECARDS];
__int64 TPS = 0;
bool _SHOW_MSG = TRUE;
ULONG oldPriClass = 0;
ULONG oldThreadLevel = 0;
HANDLE hProcess;
HANDLE hThread;

/**
\brief Initializes the pro DLL. Call this before using it. While initialization global variables are set in pro dll.
\return void
*/
void InitProDLL()
{
	struct global_vars g;
	g.userBuffer = userBuffer;
	g.hDev = hDev;
	g.aPIXEL = aPIXEL;
	g.aCAMCNT = aCAMCNT;
	g.Nospb = Nospb;
	g.Nob = Nob;
	g.useSWTrig = useSWTrig;
	DLLInitGlobals(g);
	return;
}

/**
 * \brief
 *
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_unlocking_dma_failed
 */
es_status_codes CleanupDma(uint32_t drvno)
{
	ES_LOG("Cleanup DMA\n");
	/* Disable DMA interrupts */
	WDC_IntDisable(hDev[drvno]);
	/* Unlock and free the DMA buffer */
	DWORD dwStatus = WDC_DMABufUnlock(dmaBufferInfos[drvno]);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog("Failed unlocking a contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return es_unlocking_dma_failed;
	}
	WDC_Err("Unlock DMABuf Successfull\n");
	return es_no_error;
}

// System Timer in Ticks
/**
\brief Converts Large to Int64.
*/
UINT64 LargeToInt(LARGE_INTEGER li)
{
	UINT64 res = 0;
	res = li.HighPart;
	res = res << 32;
	res = res + li.LowPart;
	return res;
} //LargeToInt

/**
\brief Init high resolution counter.
\return TPS ticks per sec
*/
int64_t InitHRCounter()
{
	BOOL ifcounter;
	UINT64 tps = 0;
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 2;

	//tps:: ticks per second = freq
	ifcounter = QueryPerformanceFrequency(&freq);
	tps = LargeToInt(freq); //ticks per second
	WDC_Err("TPS: %lld\n", tps);

	return tps;
}


/**
\brief This call comes every DMASPERINTR=500 here a DMASubBuf could be copied to the DMABigBuf.
the size of a drivers continous memory is limited, so we must copy it via this small buf to the big buf
The INTR occurs every DMASPERINTR and copies this block of scans in lower/upper half blocks.
*/
void isr( uint32_t drvno, void* pData )
{
	ES_LOG( "*isr(): 0x%x\n", IsrCounter );
	es_status_codes status = setBitS0_32(drvno, IRQFLAGS_bitindex_INTRSR, S0Addr_IRQREG );//set INTRSR flag for TRIGO
	if (status != es_no_error) return;
	//! be sure not to stop run before last isr is ready - or last part is truncated
	//usually dmaBufferSizeInBytes = 1000scans 
	//dmaBufferPartSizeInBytes = 1000 * pixel *2 -> /2 = 500 scans = 1088000 bytes
	// that means one 500 scan copy block has 1088000 bytes
	//!GS sometimes (all 10 minutes) one INTR more occurs -> just do not serve it and return
	// Fehler wenn zu viele ISRs -> memcpy out of range
	ES_LOG( "ISR Counter : 0x%x \n", IsrCounter );
	if (IsrCounter > numberOfInterrupts)
	{
		ES_LOG( "numberOfInterrupts: 0x%x \n", numberOfInterrupts );
		ES_LOG( "ISR Counter overflow: 0x%x \n", IsrCounter );
		status = resetBitS0_32( drvno, IRQFLAGS_bitindex_INTRSR, S0Addr_IRQREG );//reset INTRSR flag for TRIGO
		return;
	}
	ES_LOG("dmaBufferSizeInBytes: 0x%x \n", dmaBufferSizeInBytes);
	size_t dmaBufferPartSizeInBytes = dmaBufferSizeInBytes / DMA_BUFFER_PARTS; //1088000 bytes
	UINT16* dmaBufferReadPos = dmaBuffer[drvno] + dmaBufferPartReadPos[drvno] * dmaBufferPartSizeInBytes / sizeof(UINT16);
	//here the copyprocess happens
	memcpy( userBufferWritePos[drvno], dmaBufferReadPos, dmaBufferPartSizeInBytes );
	ES_LOG( "userBufferWritePos: 0x%x \n", userBufferWritePos[drvno] );
	dmaBufferPartReadPos[drvno]++;
	if (dmaBufferPartReadPos[drvno] >= DMA_BUFFER_PARTS)		//number of ISR per dmaBuf - 1
		dmaBufferPartReadPos[drvno] = 0;						//dmaBufferPartReadPos is 0 or 1 for buffer devided in 2 parts
	userBufferWritePos[drvno] += dmaBufferPartSizeInBytes / sizeof( UINT16 );
	status = resetBitS0_32(drvno, IRQFLAGS_bitindex_INTRSR, S0Addr_IRQREG );//reset INTRSR flag for TRIGO
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
		ES_LOG("readRegister_32 in address 0x%x failed\n", address);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_read_failed;
	}
	return es_no_error;
};

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
es_status_codes readRegister_16(uint32_t drvno, uint16_t* data, uint16_t address)
{
	volatile DWORD dwStatus = WDC_ReadAddrBlock(hDev[drvno], 0, address, sizeof(uint16_t), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG("readRegister_16 in address 0x%x failed\n", address);
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
		ES_LOG("readRegister_8 in address 0x%x failed\n", address);
		return es_register_read_failed;
	}
	return es_no_error;
};

/**
 * \brief Writes 32 bits (4 bytes) to register.
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
		ES_LOG("writeRegister_32 in address 0x%x with data: 0x%x failed\n", address, data);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_write_failed;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", address, data);
	return es_no_error;
};

/**
 * \brief Writes 32 bits (4 bytes) to register. Two boards sync version.
 *
 * \param data1 data to write board 1
 * \param data2 data to write board 2
 * \param address Register offset from BaseAdress - in bytes
 * \return es_status_codes
	- es_no_error
	- es_register_write_failed
 */
es_status_codes writeRegister_32twoBoards(uint32_t data1, uint32_t data2, uint16_t address)
{
	DWORD dwStatus1 = WDC_WriteAddr32(hDev[1], 0, address, data1);
	DWORD dwStatus2 = WDC_WriteAddr32(hDev[2], 0, address, data2);
	if (WD_STATUS_SUCCESS != dwStatus1)
	{
		ES_LOG("writeRegister_32twoBoards in address 0x%x with data: 0x%x failed\n", address, data1);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_write_failed;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", address, data);
	if (WD_STATUS_SUCCESS != dwStatus2)
	{
		ES_LOG("writeRegister_32twoBoards in address 0x%x with data: 0x%x failed\n", address, data2);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_write_failed;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", address, data);
	return es_no_error;
};

/**
 * \brief Writes 16 bits (2 bytes) to register.
 *
 * \param drvno PCIe board identifier.
 * \param data data to write
 * \param address Register offset from BaseAdress - in bytes
 * \return es_status_codes
	- es_no_error
	- es_register_write_failed
 */
es_status_codes writeRegister_16(uint32_t drvno, uint16_t data, uint16_t address)
{
	DWORD dwStatus = WDC_WriteAddrBlock(hDev[drvno], 0, address, sizeof(uint16_t), &data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT);
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ES_LOG("writeRegister_16 in address 0x%x with data: 0x%x failed\n", address, data);
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
		ES_LOG("writeRegister_8 in address 0x%x with data: 0x%x failed\n", address, data);
		return es_register_write_failed;
	}//else WDC_Err("ByteS0Write /t address /t0x%x /t data: /t0x%x \n", address, data);
	return es_no_error;
};  // WriteByteS0

/**
 * \brief Writes 8 bits (1 bytes) to register. Two boards sync version.
 *
 * \param data1 data to write board 1
 * \param data2 data to write board 2
 * \param address Register offset from BaseAdress - in bytes
 * \return es_status_codes
	- es_no_error
	- es_register_write_failed
 */
es_status_codes writeRegister_8twoBoards(uint8_t data1, uint8_t data2, uint16_t address)
{
	DWORD dwStatus1 = WDC_WriteAddr8(hDev[1], 0, address, data1);
	DWORD dwStatus2 = WDC_WriteAddr8(hDev[2], 0, address, data2);
	if (WD_STATUS_SUCCESS != dwStatus1)
	{
		ES_LOG("writeRegister_8twoBoards in address 0x%x with data: 0x%x failed\n", address, data1);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_write_failed;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", address, data);
	if (WD_STATUS_SUCCESS != dwStatus2)
	{
		ES_LOG("writeRegister_8twoBoards in address 0x%x with data: 0x%x failed\n", address, data2);
		ES_LOG("%s", LSCPCIEJ_GetLastErr());
		return es_register_write_failed;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", address, data);
	return es_no_error;
};

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
		es_status_codes status = CleanupDma(drvno);
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
	return es_no_error;
}

es_status_codes enableInterrupt( uint32_t drvno )
{
	switch (drvno)
	{
	case 1:
	{
		DWORD dwStatus = LSCPCIEJ_IntEnable(hDev[drvno], interrupt_handler1);
		if (WD_STATUS_SUCCESS != dwStatus)
		{
			ES_LOG("Failed to enable the Interrupts1. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
			return es_enabling_interrupts_failed;
		}
		break;
	}
	case 2:
	{
		DWORD dwStatus = LSCPCIEJ_IntEnable(hDev[drvno], interrupt_handler2);
		if (WD_STATUS_SUCCESS != dwStatus)
		{
			ES_LOG("Failed to enable the Interrupts2. Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
			return es_enabling_interrupts_failed;
		}
		break;
	}
	default:
		return es_parameter_out_of_range;
	}
	return es_no_error;
}

/**
 * \brief Reset the buffer write pointers and software ISR counter.
 * 
 * \param drvno PCIe board identifier.
 */
void ResetBufferWritePos(uint32_t drvno)
{
	dmaBufferPartReadPos[drvno] = 0;
	// reset buffer index to base we got from InitDMA
	userBufferWritePos[drvno] = userBuffer[drvno];
	ES_LOG( "RESET userBufferWritePos to %x\n", userBufferWritePos[drvno] );
	IsrCounter = 0;
	return;
}

void copyRestData(uint32_t drvno, size_t rest_in_bytes)
{
	ES_LOG( "Copy rest data:\n" );
	ES_LOG( "dmaBufferSizeInBytes: 0x%x \n", dmaBufferSizeInBytes );
	INT_PTR dmaBufferReadPos = dmaBuffer[drvno];
	dmaBufferReadPos += dmaBufferPartReadPos[drvno] * dmaBufferSizeInBytes / DMA_BUFFER_PARTS;
	memcpy( userBufferWritePos[drvno], dmaBufferReadPos, rest_in_bytes );
	return;
}

/**
 * \brief Initializes PCIe board on a platform specific way.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_invalid_driver_number
 *		- es_getting_device_info_failed
 *		- es_open_device_failed
 */
es_status_codes _InitBoard(uint32_t drvno)
{
	if ((drvno < 1) || (drvno > 2)) return es_invalid_driver_number;
	DWORD dwStatus = 0;
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
#ifdef WIN32
	InitProDLL();
#endif
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
		WDC_DriverClose();
		return es_device_not_found;
	}
	number_of_boards = (UINT8) scanResult.dwNumDevices;
	WDC_Err("Init HR counter\n");
	TPS = InitHRCounter();//for ticks function
	return es_no_error;	  // no Error, driver found
}

/**
 * \brief Exit driver.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_unlocking_dma_failed
 */
es_status_codes _ExitDriver(uint32_t drvno)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	ES_LOG( "Driver exit, drv: %u\n", drvno);
	if (WDC_IntIsEnabled( hDev[drvno] ))
	{
		status = CleanupDma( drvno );
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
		return es_register_read_failed;
	}
	return es_no_error;
}

/**
 * \brief Write long (32 bit) to register in space0 of PCIe board.
 *
 * \param drvno board number (=1 if one PCI board)
 * \param data long value to write
 * \param address offset from base address of register (count in bytes)
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes writeConfig_32(uint32_t drvno, uint32_t data, uint16_t address)
{
	volatile DWORD dwStatus = 0;
	dwStatus = WDC_PciWriteCfg(hDev[drvno], address, &data, sizeof(uint32_t));
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err("writeConfig_32 in address 0x%x with data: 0x%x failed\n", address, data);
		return es_register_write_failed;
	}//else WDC_Err("I0PortWrite /t address /t0x%x /t data: /t0x%x \n", address, DWData);
	return es_no_error;
};

/**
\brief Get the free and installed memory info.
\param pmemory_all how much is installed
\param pmemory_free how much is free
\return none
*/
void FreeMemInfo(uint64_t* pmemory_all, uint64_t* pmemory_free)
{
	// Use to convert bytes to KB
#define DIV 1024
	// Specify the width of the field in which to print the numbers. 
	// The asterisk in the format specifier "%*I64d" takes an integer 
	// argument and uses it to pad and right justify the number.
#define WIDTH 7
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	//_tprintf(TEXT("There is  %*ld percent of memory in use.\n"),
	//	WIDTH, statex.dwMemoryLoad);
	//_tprintf(TEXT("There are %*I64d total KB of physical memory.\n"),
	//	WIDTH, 
	*pmemory_all = statex.ullTotalPhys;
	// DIV);
	//_tprintf(TEXT("There are %*I64d free  KB of physical memory.\n"),
	//WIDTH, 
	*pmemory_free = statex.ullAvailPhys;
	// DIV);
	//_tprintf(TEXT("There are %*I64d total KB of paging file.\n"),
	//	WIDTH, statex.ullTotalPageFile / DIV);
	//_tprintf(TEXT("There are %*I64d free  KB of paging file.\n"),
	//	WIDTH, statex.ullAvailPageFile / DIV);
	//_tprintf(TEXT("There are %*I64d total KB of virtual memory.\n"),
	//	WIDTH, statex.ullTotalVirtual / DIV);
	//_tprintf(TEXT("There are %*I64d free  KB of virtual memory.\n"),
	//	WIDTH, statex.ullAvailVirtual / DIV);
	// Show the amount of extended memory available.
	//_tprintf(TEXT("There are %*I64d free  KB of extended memory.\n"),
	//	WIDTH, statex.ullAvailExtendedVirtual / DIV);
	return;
}

es_status_codes StartCopyDataToUserBufferThread(uint32_t drvno)
{
	//On Windows the copy process is done in ISR
	return es_no_error;
}

es_status_codes InitMutex(uint32_t drvno)
{
    //no mutex on windows
    return es_no_error;
}

/**
* \brief Return infos about the PCIe board.
* 	Shows 5 info messages. Can be used to test the communication with the PCI board.
* 	Is called automatically for 2 boards.
*
* - win1 : version of driver
* - win2 : ID = 53xx
* - win3 : length of space0 BAR =0x3f
* - win4 : vendor ID = EBST
* - win5 : PCI board version (same as label on PCI board)
* \param drvno board number (=1 if one PCI board)
* \return es_status_codes
* 	- es_no_error
* 	- es_register_read_failed
*	- es_no_space0
*/
es_status_codes AboutDrv(uint32_t drvno)
{
	char pstring[80] = "";
	HWND hWnd = GetActiveWindow();
	HDC aDC = GetDC(hWnd);
	// read ISA Id from S0Base+7
	UINT32 S0Data = 0;
	es_status_codes status = readRegisterS0_32(drvno, &S0Data, S0Addr_CTRLA); // Board ID =5053
	if (status != es_no_error) return status;
	S0Data = S0Data >> 16;
	sprintf_s(pstring, 80, " Board #%i    ID = 0x%I32x", drvno, S0Data);
	if (MessageBox(hWnd, pstring, " Board ID=53 ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	//The following lines doesn't make sense. S0Data is never 0.
	S0Data = 0x07FF;
	if (S0Data == 0)
	{
		ErrorMsg("Board #%i  no Space0!", drvno);
		return es_no_space0;
	}
	sprintf_s(pstring, 80, "Board #%i     length = 0x%I32x", drvno, S0Data);
	if (MessageBox(hWnd, pstring, "  PCI space0 length=", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	UCHAR udata1 = 0,
		udata2 = 0,
		udata3 = 0,
		udata4 = 0;
	if (S0Data >= 0x1F)
	{//if WE -> has space 0x20
		status = readRegisterS0_8(drvno, &udata1, 0x1C);
		if (status != es_no_error) return status;
		status = readRegisterS0_8(drvno, &udata2, 0x1D);
		if (status != es_no_error) return status;
		status = readRegisterS0_8(drvno, &udata3, 0x1E);
		if (status != es_no_error) return status;
		status = readRegisterS0_8(drvno, &udata4, 0x1F);
		if (status != es_no_error) return status;
		sprintf_s(pstring, 80, "Board #%i  ven ID = %c%c%c%c", drvno, udata1, udata2, udata3, udata4);
		if (MessageBox(hWnd, pstring, " Board vendor=EBST ", MB_OK | MB_ICONEXCLAMATION) == IDOK);
	}
	if (S0Data >= 0x3F)
	{//if 9056 -> has space 0x40
		status = readRegisterS0_32(drvno, &S0Data, S0Addr_PCI);
		if (status != es_no_error) return status;
		sprintf_s(pstring, 80, "Board #%i   board version = 0x%I32x", drvno, S0Data);
		if (MessageBox(hWnd, pstring, "Board version ", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
	ReleaseDC(hWnd, aDC);
	return es_no_error;
};

/**
 * \brief Reads registers 0 to 12 of TDC-GPX chip. Time delay counter option.
 *
 * \param drvno PCIe board identifier
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes AboutGPX(uint32_t drvno)
{
	HWND hWnd;
	char pstring[80] = "";
	int i, j = 0;
	char fn[1000];
	ULONG regData, regNumber, tempData;
	BOOL space, abbr, irf, empty;
	char LUTS0Reg[16][30] = {
		"Reg0 \t",
		"Reg1 \t",
		"Reg2\t",
		"Reg3\t",
		"Reg4 \t",
		"Reg5\t",
		"Reg6 \t",
		"Reg7\t",
		"Reg8\t",
		"Reg9 \t",
		"Reg10\t",
		"Reg11 \t",
		"Reg12 \t",
		"Reg13\t ",
		"Reg14 \t",
		"Reg15\t"
	}; //Look-Up-Table for the S0 Registers

	hWnd = GetActiveWindow();

	j = sprintf(fn, "GPX- registers   \n");
	es_status_codes status = es_no_error;
	for (i = 0; i < 8; i++)
	{
		status = ReadGPXCtrl(drvno, i, &regData);
		if (status != es_no_error) return status;
		j += sprintf(fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], regData);
	}

	for (i = 11; i < 13; i++)
	{
		status = ReadGPXCtrl(drvno, i, &regData);
		if (status != es_no_error) return status;
		j += sprintf(fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], regData);
	}
	MessageBox(hWnd, fn, "GPX regs", MB_OK);
	j = sprintf(fn, "delay- registers   \n");
	i = 0;
	abbr = FALSE;
	empty = FALSE;
	while (!abbr)
	{
		status = WaitTrigger(1, FALSE, &space, &abbr);
		if (status != es_no_error) return status;
		irf = FALSE;
		i = 0;
		j = sprintf(fn, "read- regs   \n");
		i += 1;
		status = ReadGPXCtrl(drvno, 8, &regData); //lege addr 8 an bus !!!!
		if (status != es_no_error) return status;
		j += sprintf(fn + j, "%d \t: 0x%I32x\n", i, regData);
		i += 1;
		status = ReadGPXCtrl(drvno, 9, &regData); //lege addr 9 an bus !!!!
		if (status != es_no_error) return status;
		j += sprintf(fn + j, "%d \t: 0x%I32x\n", i, regData);
		MessageBox(hWnd, fn, "GPX regs", MB_OK);
	}
	status = ReadGPXCtrl(drvno, 11, &regData);
	if (status != es_no_error) return status;
	j += sprintf(fn + j, "%s \t: 0x%I32x\n", " stop hits", regData);
	status = ReadGPXCtrl(drvno, 12, &regData);
	if (status != es_no_error) return status;
	j += sprintf(fn + j, "%s \t: 0x%I32x\n", " flags", regData);
	MessageBox(hWnd, fn, "GPX regs", MB_OK);
	return ReadGPXCtrl(drvno, 8, &regData); //read access follows                 set addr 8 to bus !!!!
}

/**
 * \brief Read registers of space0. Space0 are the control registers of the PCIe board.
 *
 * \param drvno PCIe board identifier
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes AboutS0(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpS0Registers(drvno, &cstring);
	if (status != es_no_error) return status;
	MessageBox(GetActiveWindow(), cstring, "S0 regs", MB_OK);
	return AboutTLPs(drvno);
}//AboutS0

/**
 * \brief
 *
 * \param drvno PCIe board identifier.
 * \return es_status_codes
 * 		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes AboutTLPs(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpTlpRegisters(drvno, &cstring);
	MessageBox(GetActiveWindow(), cstring, "DMA transfer payloads", MB_OK | MB_DEFBUTTON2);
	return status;
}//AboutTLPs

/**
 * \brief Switch on error message boxes of our software. Default is On.
 *
 * \return none.
 */
void ErrMsgBoxOn()
{
	_SHOW_MSG = TRUE;
}

/**
 * \brief Disable error message boxes, if not needed.
 * \return none
 */
void ErrMsgBoxOff()
{
	_SHOW_MSG = FALSE;
}

/**
 * \brief Display error message. If ErrMsgBoxOn is set.
 *
 * \param ErrMsg Message. Buffer size: 100.
 * \return none
 */
void ErrorMsg(char ErrMsg[100])
{
	if (_SHOW_MSG)
	{
		if (MessageBox(GetActiveWindow(), ErrMsg, "ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

/**
 * \brief Simple display of unsigned integer as error message for test purpose.
 *
 * \param val unsigned integer 64 bit
 * \return none.
 */
void ValMsg(uint64_t val)
{
	char AString[60];
	if (_SHOW_MSG)
	{
		sprintf_s(AString, 60, "%s%d 0x%I64x", "val= ", val, val);
		if (MessageBox(GetActiveWindow(), AString, "ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

/**
* \brief Reads system timer.
*
* Read 2x ticks and calculate the difference between the calls in microsec with DLLTickstous, init timer by calling DLLInitSysTimer before use.
* \return act ticks
*/
long long ticksTimestamp()
{
	LARGE_INTEGER PERFORMANCECOUNTERVAL = { 0, 0 };

	QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
	return PERFORMANCECOUNTERVAL.QuadPart;

}//ticksTimestamp

/**
 * \brief Returns if trigger or key.
 *
 * Wait for raising edge of Pin #17 SubD = D6 in CtrlA register
 * ReturnKey is 0 if trigger, else keycode (except space )
 * if keycode is space, the loop is not canceled
 *
 * D6 depends on Slope (D5)
 * HighSlope = TRUE  : pos. edge
 * HighSlope = FALSE : neg. edge
 *
 * \param drvno PCIe board identifier
 * \param ExtTrigFlag =FALSE: this function is used to get the keyboard input
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes WaitTrigger(uint32_t drvno, bool ExtTrigFlag, bool *SpaceKey, bool *AbrKey)
{
	bool FirstLo = FALSE;
	bool HiEdge = FALSE;
	bool Abbr = FALSE;
	bool Space = FALSE;
	BYTE ReadTrigPin = 0;
	es_status_codes status = es_no_error;
	do
	{
		if (ExtTrigFlag)
		{
			status = readRegisterS0_8(drvno, &ReadTrigPin, S0Addr_CTRLA);
			if (status != es_no_error) return status;
			ReadTrigPin &= CTRLA_bit_DIR_TRIGIN;
			if (ReadTrigPin == 0) FirstLo = TRUE; //first look for lo
			if (FirstLo)
			{
				if (ReadTrigPin > 0) HiEdge = TRUE;
			}; // then look for hi
		}
		else HiEdge = TRUE;
		if (GetAsyncKeyState(VK_ESCAPE))
			Abbr = TRUE;
		if (GetAsyncKeyState(VK_SPACE))  Space = TRUE;
	} while ((!HiEdge) && (!Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
	return status;
};// WaitTrigger

/**
 * \brief Translate ticks to micro seconds.
 * \param tks ticks of system timer
 * \return micro seconds of tks
*/
uint32_t Tickstous(uint64_t tks)
{
	BOOL ifcounter;
	UINT64 delay = 0;
	UINT64 tps = 0; //ticks per second
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 0;

	//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency(&freq);
	tps = LargeToInt(freq); //ticks per second

	if (tps == 0) return 0; // no counter available

	delay = tks * 1000000;
	delay = delay / tps;
	return (UINT32)delay;
} // Tickstous

/**
\brief This functions returns after a time given in microseconds. The time is measured in CPU ticks. The function is escable by pressing ESC.
\param musec Time to wait in microseconds.
\return 1 when success, 0 when aborted by ESC or failure
*/
uint8_t WaitforTelapsed(long long musec)
{
	LONGLONG ticks_to_wait = musec * TPS / 1000000;
	LONGLONG start_timestamp = ticksTimestamp();
	LONGLONG destination_timestamp = start_timestamp + ticks_to_wait;
	//WDC_Err("Startzeit: %lld\n", start_timestamp);
	// detect overflow
	if (destination_timestamp < start_timestamp) return 0;
	// wait until time elapsed
	while (destination_timestamp > ticksTimestamp())
	{
		if (GetAsyncKeyState(VK_ESCAPE) | (FindCam(1) != es_no_error) | abortMeasurementFlag) return 0; // check for kill ?
	}
	//WDC_Err("Endzeit:  %lld\n", ticksTimestamp());
	return 1;
}

uint16_t checkEscapeKeyState()
{
	return GetAsyncKeyState(VK_ESCAPE);
}

uint16_t checkSpaceKeyState()
{
	return GetAsyncKeyState(VK_SPACE);
}

/**
 * \brief Converts threadp value (1..31) to process priority class and thread priority level.
 *
 * \param threadp Thread priority from 1 to 31. Is split in class and level.
 * \param priclass Priority class return value
 * \param prilevel Priority level return value
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes ThreadToPriClass(ULONG threadp, DWORD *priclass, DWORD *prilevel)
{
	DWORD propriclass[31] = { IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS, IDLE_PRIORITY_CLASS,
		NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS,
		HIGH_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, HIGH_PRIORITY_CLASS, HIGH_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS, REALTIME_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS };
	DWORD threprilevel[31] = { THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE, THREAD_PRIORITY_IDLE,
		THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_TIME_CRITICAL };
	// range check
	if ((0 < threadp) && (threadp < 32))
	{
		threadp -= 1; //array index from 0..30
		*priclass = propriclass[threadp];
		*prilevel = threprilevel[threadp];
		return es_no_error;
	}
	else
		return es_parameter_out_of_range;
}

/**
 * \brief Set thread to new priority level.
 * 
 * Old level is kept in global oldPriClass and oldThreadLevel.
 * \param threadp Thread priority 1..31. Is split in class and level.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_setting_thread_priority_failed
 */
es_status_codes SetPriority(ULONG threadp)
{
	ULONG priClass = 0;
	ULONG priLevel = 0;
	es_status_codes status = ThreadToPriClass(threadp, &priClass, &priLevel);
	if (status != es_no_error) return status;
	hProcess = GetCurrentProcess();
	oldPriClass = GetPriorityClass(hProcess);
	if (!SetPriorityClass(hProcess, priClass))
		return es_setting_thread_priority_failed;
	hThread = GetCurrentThread();
	oldThreadLevel = GetThreadPriority(hThread);
	if (!SetThreadPriority(hThread, priLevel))
		return es_setting_thread_priority_failed;
	return status;
}

/**
 * \brief Reset thread priority to old level.
 * 
 * \return es_status_codes:
 *		- es_no_error
 *		- es_setting_thread_priority_failed
 */
es_status_codes ResetPriority()
{
	if (!SetPriorityClass(hProcess, oldPriClass))
		return es_setting_thread_priority_failed;
	if (!SetThreadPriority(hThread, oldThreadLevel))
		return es_setting_thread_priority_failed;
	return es_no_error;
}