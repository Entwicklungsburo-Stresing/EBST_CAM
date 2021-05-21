#include "../crossPlattformBoard_ll.h"

***REMOVED***#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"

#include "Jungo/windrvr.h"
#include "Jungo/wdc_lib.h"
#include "Jungo/wdc_defs.h"

WDC_DEVICE_HANDLE hDev_tmp[MAXPCIECARDS];
WDC_DEVICE_HANDLE* hDev = &hDev_tmp;
DWORD dmaBufferSizeInBytes = 0;

#define interrupt_handler1(drvno, data) isr()

/**
\brief This call comes every DMASPERINTR=500 here a DMASubBuf could be copied to the DMABigBuf.
the size of a drivers continous memory is limited, so we must copy it via this small buf to the big buf
The INTR occurs every DMASPERINTR and copies this block of scans in lower/upper half blocks.
*/
void isr( UINT drvno, PVOID pData )
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
}//DLLCALLCONV interrupt_handler

//TODO is this neceserry?
VOID DLLCALLCONV interrupt_handler1( PVOID pData ) { isr( 1, pData ); }
VOID DLLCALLCONV interrupt_handler2( PVOID pData ) { isr( 2, pData ); }

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
es_status_codes SetupDma( UINT32 drvno )
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
	// DREQ: every XCK h->l starts DMA by hardware
	//set hardware start des dma  via DREQ withe data = 0x4000000
	ULONG mask = 0x40000000;
	ULONG data = 0;// 0x40000000;
	if (HWDREQ_EN)
		data = 0x40000000;
	return SetS0Reg( data, mask, S0Addr_IRQREG, drvno );
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