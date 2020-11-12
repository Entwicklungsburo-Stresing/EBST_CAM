#include "Board_ll.h"

WDC_DEVICE_HANDLE hDev_tmp[MAXPCIECARDS];
WDC_DEVICE_HANDLE* hDev = &hDev_tmp;
//PWDC_DEVICE pDev_tmp[MAXPCIECARDS];
//PWDC_DEVICE* pDev = &pDev_tmp;

/*
\brief Set specified bits to 1 in register at memory address.

*/
BOOL SetS0Reg( ULONG Data, ULONG Bitmask, CHAR Address, UINT32 drvno )
{
	UINT32 OldRegisterValues, Setbit_mask, OldRegVals_and_SetBits, Clearbit_mask, NewRegisterValues;

	//read the old Register Values in the S0 Address Reg
	if (!ReadLongS0( drvno, &OldRegisterValues, Address ))
	{
		ErrLog( "ReadLong S0 Failed in SetDMAReg \n" );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		return FALSE;
	}
	//step 0: delete not needed "1"s
	Data &= Bitmask;
	//step 1: save Data as setbitmask for making this part humanreadable
	Setbit_mask = Data;
	//step 2: setting high bits in the Data
	OldRegVals_and_SetBits = OldRegisterValues | Setbit_mask;
	//step 3: prepare to clear bits
	Clearbit_mask = Data | ~Bitmask;
	//step 4: clear the low bits in the Data
	NewRegisterValues = OldRegVals_and_SetBits & Clearbit_mask;

	//write the data to the S0 controller
	if (!WriteLongS0( drvno, NewRegisterValues, Address ))
	{
		ErrLog( "WriteLong S0 Failed in SetDMAReg \n" );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		return FALSE;
	}
	return TRUE;
}

/**
\brief Set bit to 1 in register at memory address.
\param bitnumber 0...31, 0 is LSB, 31 MSB
\param Address register address
\param drvno board number (=1 if one PCI board)
\return =0 if write fails, otherwise != 0
*/
BOOL SetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno )
{
	//bitnumber: 0...31
	/*
	old:
	ULONG bitmask = 0x1 << bitnumber;
	if (!SetS0Reg( 0xFFFFFFFF, bitmask, Address, drvno ))
	{
		ErrLog( "WriteLong S0 Failed in SetDMAReg \n" );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		return FALSE;
	}
	*/

	PWDC_DEVICE pDev = ((PWDC_DEVICE)hDev[drvno]);
	WDC_MEM_DIRECT_ADDR( pDev->pAddrDesc );

	BitTestAndSet( pDev->pAddrDesc->pUserDirectMemAddr + 0x80 + Address, bitnumber );
	return TRUE;
}

/**
\brief Set bit to 0 in register at memory address.
\param bitnumber 0...31, 0 is LSB, 31 MSB
\param Address register address
\param drvno board number (=1 if one PCI board)
\return TRUE when success, otherwise FALSE
*/
BOOL ResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno )
{
	/*
	old:
	ULONG bitmask = 0x1 << bitnumber;
	if (!SetS0Reg( 0x0, bitmask, Address, drvno ))
	{
		ErrLog( "WriteLong S0 Failed in SetDMAReg \n" );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		return FALSE;
	}
	*/
	PWDC_DEVICE pDev = ((PWDC_DEVICE)hDev[drvno]);
	WDC_MEM_DIRECT_ADDR( pDev->pAddrDesc );

	BitTestAndReset( pDev->pAddrDesc->pUserDirectMemAddr + 0x80 + Address, bitnumber );
	return TRUE;
}

/**
\brief Read long (32 bit) from runtime register of PCIe board. This function reads the memory mapped data , not the I/O Data. Reads data from PCIe conf space.
\param drvno board number (=1 if one PCI board)
\param DWData pointer to where data is stored
\param PortOff offset of register (count in bytes)
\return ==0 if error, TRUE if success
*/
BOOL ReadLongIOPort( UINT32 drvno, UINT32 *DWData, ULONG PortOff )
{
	volatile DWORD dwStatus = 0;
	DWORD   ReturnedLength;
	ULONG	PortOffset;

	PortOffset = PortOff;
	dwStatus = WDC_PciReadCfg( hDev[drvno], PortOff, DWData, sizeof( UINT32 ) );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "ReadLongIOPort in address 0x%x failed\n", PortOff );
		//old message...i kept it because i dont know what it does
		ErrorMsg( "Read IORunReg failed" );
		return FALSE;
	}
	return TRUE;
};  // ReadLongIOPort

/**
\brief Read long (32 bit) from register in space0 of PCIe board.
\param drvno board number (=1 if one PCI board)
\param DWData pointer to where data is stored
\param PortOff offset of register from base address (count in bytes)
\return ==0 if error, TRUE if success
*/
BOOL ReadLongS0( UINT32 drvno, UINT32 * DWData, ULONG PortOff )
{
	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;

	//space0 starts at S0-Offset=0x80 in BAR0
	PortOffset = PortOff + 0x80;

	dwStatus = WDC_ReadAddrBlock( hDev[drvno], 0, PortOffset, sizeof( UINT32 ), DWData, WDC_MODE_8, WDC_ADDR_RW_DEFAULT );
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "ReadLongS0 in address 0x%x failed\n", PortOff );
		ErrorMsg( "Read long in space0 failed" );
		return FALSE;
	}

	return TRUE;
};  // ReadLongS0


/**
\brief Reads long on DMA area.
\param drvno PCIe board identifier
\param pDWData buffer for data
\param PortOff Offset from BaseAdress - in Bytes ! 0..3= Regs of Board.
\return TRUE (!=0) if success
*/
BOOL ReadLongDMA( UINT32 drvno, UINT32 * DWData, ULONG PortOff )
{
	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;

	PortOffset = PortOff;
	dwStatus = WDC_ReadAddrBlock( hDev[drvno], 0, PortOffset, sizeof( UINT32 ), DWData, WDC_MODE_8, WDC_ADDR_RW_DEFAULT );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "ReadLongDMA in address 0x%x failed\n", PortOff );
		ErrorMsg( "Read long in DMA failed" );
		return FALSE;
	}

	return TRUE;
};  // ReadLongDMA

/**
\brief Read byte (8 bit) from register in space0 of PCIe board, except r10-r1f.
\param drvno board number (=1 if one PCI board)
\param data pointer to where data is stored
\param PortOff offset of register from base address (count in bytes)
\return ==0 if error, TRUE if success
*/
BOOL ReadByteS0( UINT32 drvno, BYTE *data, ULONG PortOff )
{
	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;

	PortOffset = PortOff + 0x80;
	dwStatus = WDC_ReadAddrBlock( hDev[drvno], 0, PortOffset, sizeof( BYTE ), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT );
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "ReadByteS0 in address 0x%x failed\n", PortOff );
		ErrorMsg( "Read byte in space0 failed" );
		return FALSE;
	}
	return TRUE;
};  // ReadByteS0

/**
\brief Write long (32 bit) to register in space0 of PCIe board.
\param drvno board number (=1 if one PCI board)
\param DataL long value to write
\param PortOff offset from base address of register (count in bytes)
\return ==0 if error, TRUE if success
*/
BOOL WriteLongIOPort( UINT32 drvno, UINT32 DataL, ULONG PortOff )
{
	volatile DWORD dwStatus = 0;
	PUINT32 data = &DataL;

	//WriteData.POff	= PortOff;
	//WriteData.Data	= DWData;
	//DataLength		= 8; 

	dwStatus = WDC_PciWriteCfg( hDev[drvno], PortOff, data, sizeof( UINT32 ) );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "WriteLongIOPort in address 0x%x with data: 0x%x failed\n", PortOff, DataL );
		ErrorMsg( "WriteLongIOPort failed" );
		return FALSE;
	}//else WDC_Err("I0PortWrite /t address /t0x%x /t data: /t0x%x \n", PortOff, DWData);

	return TRUE;
};  // WriteLongIOPort

/**
\brief Write long (32 bit) to register in space0 of PCIe board.
\param drvno board number (=1 if one PCI board)
\param DWData long value to write
\param PortOff offset of register from base address (count in bytes)
\return ==0 if error, TRUE if success
*/
BOOL WriteLongS0( UINT32 drvno, UINT32 DWData, ULONG PortOff )
{
	volatile DWORD dwStatus = 0;
	ULONG	PortOffset;
	PUINT32 data = &DWData;
	//PULONG data = &DWData;

	PortOffset = PortOff + 0x80;
	dwStatus = WDC_WriteAddrBlock( hDev[drvno], 0, PortOffset, sizeof( UINT32 ), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT );
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "WriteLongS0 in address 0x%x with data: 0x%x failed\n", PortOff, DWData );
		ErrorMsg( "WriteLongS0 failed" );
		return FALSE;
	}//else WDC_Err("LongS0Write /t address /t0x%x /t data: /t0x%x \n", PortOff, DWData);

	return TRUE;
};  // WriteLongS0

/**
\brief Writes long to space0 register.
\param drvno PCIe board identifier.
\param DWData data to write
\param PortOff Register offset from BaseAdress - in bytes
\return Returns TRUE if success.
*/
BOOL WriteLongDMA( UINT32 drvno, UINT32 DWData, ULONG PortOff )
{
	BOOL fResult = FALSE;
	DWORD dwStatus = 0;
	PUINT32 data = &DWData;

	dwStatus = WDC_WriteAddrBlock( hDev[drvno], 0, PortOff, sizeof( UINT32 ), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT );
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "WriteLongDMA in address 0x%x with data: 0x%x failed\n", PortOff, DWData );
		ErrorMsg( "WriteLongDMA failed" );
		return FALSE;
	}//else WDC_Err("DMAWrite /t address /t0x%x /t data: /t0x%x \n", PortOff, DWData);

	return TRUE;
};  // WriteLongDMA


/**
\brief Write byte (8 bit) to register in space0 of PCIe board, except r10-r1f.
\param drv board number (=1 if one PCI board)
\param DataByte byte value to write
\param PortOff Offset drom BaseAdress of register (count in bytes)
\return ==0 if error, TRUE if success
*/
BOOL WriteByteS0( UINT32 drv, BYTE DataByte, ULONG PortOff )
{
	//BOOL fResult = FALSE;
	//sDLDATA WriteData;
	//ULONG	DataLength;
	//DWORD   ReturnedLength;
	volatile DWORD dwStatus = 0;
	PBYTE data = &DataByte;
	ULONG	PortOffset;
	PortOffset = PortOff + 0x80;
	dwStatus = WDC_WriteAddrBlock( hDev[drv], 0, PortOffset, sizeof( BYTE ), data, WDC_MODE_8, WDC_ADDR_RW_DEFAULT );
	//the second parameter gives the memory space 0:mem mapped cfg/S0-space 1:I/O cfg/S0-space 2:DMA-space
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		WDC_Err( "WriteByteS0 in address 0x%x with data: 0x%x failed\n", PortOff, DataByte );
		ErrorMsg( "WriteByteS0 failed" );
		return FALSE;
	}//else WDC_Err("ByteS0Write /t address /t0x%x /t data: /t0x%x \n", PortOff, DWData);
	//no comparison possible because some Read-Only-Register are changing when we are writing in the same register
	/*
	BYTE checkdata;
	ReadByteS0( drv, &checkdata, PortOff );
	if (*data != checkdata)
	{
		WDC_Err( "\nWriteByteError in address 0x%x:\ndata to write: %x\n", PortOff, DataByte );
		WDC_Err( "data read: %x\n", checkdata );
	}
	*/
	return TRUE;
};  // WriteByteS0

/**
\brief Sends data via fibre link, e.g. used for sending data to ADC (ADS5294).

Send setup:
- d0:d15 = data for AD-Reg  ADS5294
- d16:d23 = ADR of  AD-Reg
- d24 = ADDR0		AD=1
- d25 = ADDR1		AD=0
- d26 makes load pulse
- all written to DB0 in Space0 = Long0
- for AD set maddr=01, adaddr address of reg
\param drvno board number (=1 if one PCI board)
\param maddr master address for specifying device (2 for ADC)
\param adaddr register address
\param data data
\return none
*/
void SendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data )
{
	UINT32 ldata = 0;
	ldata = maddr;
	ldata = ldata << 8;
	ldata |= adaddr;
	ldata = ldata << 16;
	ldata |= data;
	WriteLongS0( drvno, ldata, S0Addr_DBR );
	ldata |= 0x4000000;		//load val
	WriteLongS0( drvno, ldata, S0Addr_DBR );
	ldata = 0;		//rs load
	WriteLongS0( drvno, ldata, S0Addr_DBR );
	Sleep( 1 );
	return;
}//SendFLCAM