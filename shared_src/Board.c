/*
This file is part of CCDExamp and ECLSCDLL.

CCDExamp and ECLSCDLL are free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CCDExamp and ECLSCDLL are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see < http://www.gnu.org/licenses/>.

Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
*/

#include "Board.h"

//try different methodes - only one can be TRUE!

//for jungo projects
#define KER_MODE FALSE
#define KERNEL_64BIT	

//jungodriver specific variables
WD_PCI_CARD_INFO deviceInfo[MAXPCIECARDS];
//Buffer of WDC_DMAContigBufLock function = one DMA sub block - will be copied to the big pDMABigBuf later
USHORT* dmaBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
WD_DMA *dmaBufferInfos[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL }; //there will be saved the neccesary parameters for the dma buffer
DWORD64 IsrCounter = 0;
UINT8 dmaBufferPartReadPos[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
DWORD dmaBufferSizeInBytes = 0;
WDC_PCI_SCAN_RESULT scanResult;
//priority globals
UINT32 NEWPRICLASS = 0;
UINT32 NEWPRILEVEL = 0;
UINT32 READTHREADPriority =  15;
UINT32 OLDTHREADLEVEL = 0;
UINT32 OLDPRICLASS = 0;
HANDLE hPROCESS = 0;
HANDLE hTHREAD = 0;
//general switch to suppress ErrorMsg windows , global in BOARD
BOOL _SHOW_MSG = TRUE;
__int64 TPS = 0;				// ticks per second; is set in InitHRCounter
UINT32 NO_TLPS;//0x12; //was 0x11-> x-offset			//0x11=17*128  = 2176 Bytes  = 1088 WORDS
UINT32 TLPSIZE = 0x20; //default = 0x20 A.M. Dec'20 //with0x21: crash
UINT32 BDATA = 0;
UINT16* userBufferWritePos[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
UINT32 numberOfInterrupts;

// extern global variables
UINT8 number_of_boards = 0;
UINT32 Nob = 1;
UINT32 tmp_Nosbp = 1000;
UINT32* Nospb = &tmp_Nosbp;
UINT32 tmp_aCAMCNT[MAXPCIECARDS] = { 1, 1, 1, 1, 1 };	// cameras parallel
UINT32* aCAMCNT = tmp_aCAMCNT;	// cameras parallel
UINT32 ADRDELAY=0;
BOOL escape_readffloop = FALSE;
BOOL CONTFFLOOP = FALSE;
UINT32 CONTPAUSE = 1;  // delay between loops in continous mode
UINT16* temp_userBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
UINT16** userBuffer= temp_userBuffer;
UINT32 tmp_aPIXEL[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
UINT32* aPIXEL = tmp_aPIXEL;
BOOL Running = FALSE;
UINT32 BOARD_SEL = 1;
BOOL useSWTrig = FALSE;

// ***********     functions    ********************** 

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
void ErrorMsg( char ErrMsg[100] )
{
	if (_SHOW_MSG)
	{
		if (MessageBox( GetActiveWindow(), ErrMsg, "ERROR", MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	}
};

/**
 * \brief Simple display of unsigned integer as error message for test purpose.
 * 
 * \param val unsigned integer 64 bit
 * \return none.
 */
void ValMsg( UINT64 val )
{
	char AString[60];
	if (_SHOW_MSG)
	{
		sprintf_s( AString, 60, "%s%d 0x%I64x", "val= ", val, val );
		if (MessageBox( GetActiveWindow(), AString, "ERROR", MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	}
};

/**
\brief Regs des Contig DMA Buf.
*/
void AboutDMARegs( UINT32 drv )
{
	HWND hWnd = GetActiveWindow();
	int j = 0;
#define s_size 1000
	char fn[s_size];
	PUCHAR pmaxaddr = NULL;

	if (!dmaBufferInfos[drv]) { j = sprintf_s( fn, s_size, " pBufInfo = 0x%p \n", dmaBufferInfos[drv] ); }
	else
	{
		j = 0;
		if (DMA_SGBUF) 		j = sprintf_s( fn, s_size, " SG version \n\n" );
		if (DMA_CONTIGBUF) 		j = sprintf_s( fn, s_size, " Contig Buf version \n\n" );

		if (j == 0)
		{
			ErrorMsg( "not SG and not Contig" );
			return;
		}
		for (int i = 1; i < 3; i++)
		{
			pmaxaddr = dmaBufferInfos[i]->pUserAddr;// calc the upper addr
			pmaxaddr += dmaBufferInfos[i]->dwBytes;
			j += sprintf_s( fn + j, s_size, " pBufInfo = 0x%p \n", dmaBufferInfos[i] );
			j += sprintf_s( fn + j, s_size, " hDMA = 0x%I32x \n", dmaBufferInfos[i]->hDma );
			j += sprintf_s( fn + j, s_size, " pUserAddr = 0x%p \n", dmaBufferInfos[i]->pUserAddr );
			j += sprintf_s( fn + j, s_size, " pMAXUserAddr = 0x%p \n", pmaxaddr );
			j += sprintf_s( fn + j, s_size, " pKernelAddr = 0x%p \n", dmaBufferInfos[i]->pKernelAddr );
			j += sprintf_s( fn + j, s_size, " dwBytes = 0x%I32x = %d\n", dmaBufferInfos[i]->dwBytes, dmaBufferInfos[i]->dwBytes );
			j += sprintf_s( fn + j, s_size, " dwOptions = 0x%I32x \n", dmaBufferInfos[i]->dwOptions );
			j += sprintf_s( fn + j, s_size, " dwPages = 0x%I32x \n", dmaBufferInfos[i]->dwPages );
			j += sprintf_s( fn + j, s_size, " hCard = 0x%I32x \n", dmaBufferInfos[i]->hCard );
			j += sprintf_s( fn + j, s_size, " physAddr(page0) = 0x%p \n", dmaBufferInfos[i]->Page[0].pPhysicalAddr );
			j += sprintf_s( fn + j, s_size, " MAXphysAddr(page0) = 0x%p \n", dmaBufferInfos[i]->Page[0].pPhysicalAddr + dmaBufferInfos[i]->dwBytes );
			j += sprintf_s( fn + j, s_size, " pagesize(page0) = 0x%I32x \n", dmaBufferInfos[i]->Page[0].dwBytes );
			j += sprintf_s( fn + j, s_size, " physAddr(page1) = 0x%p \n", dmaBufferInfos[i]->Page[1].pPhysicalAddr );
			j += sprintf_s( fn + j, s_size, " pagesize(page1) = 0x%I32x \n", dmaBufferInfos[i]->Page[1].dwBytes );
		}
	}
	if (MessageBox( hWnd, fn, " DMA Buf Regs ", MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	return;
}

/**
 * \brief
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes
 * 		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes AboutTLPs( UINT32 drvno )
{
	ULONG BData = 0;
	ULONG j = 0;
	char fn[600];
	ULONG actpayload = 0;
	j += sprintf( fn + j, "PAY_LOAD values : 0 = 128 bytes, 1 = 256 bytes, 2 = 512 bytes\n" );
	es_status_codes status = ReadLongIOPort( drvno, &BData, PCIeAddr_devCap );//0x4c
	if (status != es_no_error) return status;
	j += sprintf( fn + j, "PAY_LOAD Supported : 0x%x\n", BData & 0x7 );
	//		WriteLongIOPort(DRV,0x2840,0x60);  not working  !! destroys PC? !!
	status = ReadLongIOPort( drvno, &BData, PCIeAddr_devStatCtrl );
	if (status != es_no_error) return status;
	actpayload = (BData >> 5) & 0x7;
	j += sprintf( fn + j, "PAY_LOAD : 0x%x\n", actpayload );
	status = ReadLongIOPort( drvno, &BData, PCIeAddr_devStatCtrl );
	if (status != es_no_error) return status;
	j += sprintf( fn + j, "MAX_READ_REQUEST_SIZE : 0x%x\n\n", (BData >> 12) & 0x7 );
	//BData &= 0xFFFF8FFF;//set from 256 to 128 max read request size
	//BData |= 0x00001000;//set from 128 to 256 
	//WriteLongIOPort(DRV, BData, PCIeAddr_devStatCtrl);
	//ReadLongIOPort(DRV, &BData, PCIeAddr_devStatCtrl);
	//j += sprintf(fn + j, "MAX_READ_REQUEST_SIZE after : 0x%x\n\n", (BData >> 12) & 0x7);
	BData = aPIXEL[drvno];
	j += sprintf( fn + j, "pixel: %d \n", BData );
	switch (actpayload)
	{
	case 0: BData = 0x20;		break;
	case 1: BData = 0x40;		break;
	case 2: BData = 0x80;		break;
	case 3: BData = 0x100;		break;
	}
	j += sprintf( fn + j, "TLP_SIZE is: %d DWORDs = %d BYTEs\n", BData, BData * 4 );
	status = ReadLongDMA( drvno, &BData, DmaAddr_WDMATLPS );
	if (status != es_no_error) return status;
	j += sprintf( fn + j, "TLPS in DMAReg is: %d \n", BData );
	if (LEGACY_202_14_TLPCNT) // A.M. Dec'20
		BData = (aPIXEL[drvno] - 1) / (BData * 2) + 1 + 1;
	else
		BData = (aPIXEL[drvno] - 1) / (BData * 2) + 1;
	j += sprintf( fn + j, "number of TLPs should be: %d\n", BData );
	status = ReadLongDMA( drvno, &BData, DmaAddr_WDMATLPC );
	if (status != es_no_error) return status;
	j += sprintf( fn + j, "number of TLPs is: %d \n", BData );
	MessageBox( GetActiveWindow(), fn, "DMA transfer payloads", MB_OK | MB_DEFBUTTON2 );
	return status;
}//AboutTLPs

/**
 * \brief Read registers of space0. Space0 are the control registers of the PCIe board.
 *
 * \param drvno PCIe board identifier
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes AboutS0( UINT32 drvno )
{
	#define entries  41		//32 
	int i, j = 0;
	int numberOfBars = 0;
	char fn[entries*40];
	UINT32 S0Data = 0;
	ULONG length = 0;
	HWND hWnd;
	char LUTS0Reg[entries][40] = {
		"DBR \t",
		"CTRLA \t",
		"XCKLL \t",
		"XCKCNTLL",
		"PIXREG \t",
		"FIFOCNT \t",
		"VCLKCTRL",
		"'EBST' \t",
		"SDAT \t",
		"SEC \t",
		"TOR \t",
		"ARREG \t",
		"GIOREG \t",
		"nc\t ",
		"IRQREG \t",
		"PCI board version",
		"R0 PCIEFLAGS",
		"R1 NOS\t",
		"R2 SCANINDEX",
		"R3 DMABUFSIZE",
		"R4 DMASPERINTR",
		"R5 BLOCKS",
		"R6 BLOCKINDEX",
		"R7 CAMCNT",
		"R8 GPX Ctrl",
		"R9 GPX Data",
		"R10 ROI 0 \t",
		"R11 ROI 1 \t",
		"R12 ROI 2\t",
		"R13 XCKDLY",
		"R14 ADSC ",
		"R15 LDSC\t",
		"R16 BTimer",
		"R17 BDAT\t",
		"R18 BEC\t",
		"R19 BFLAGS",
		"R20 TR1\t",
		"R21 TR2\t",
		"R22 TR3\t",
		"R23 TR4\t",
		"R24 TR5\t"
	}; //Look-Up-Table for the S0 Registers
	hWnd = GetActiveWindow();
	j = sprintf( fn, "S0- registers   \n" );
	//Hier werden alle 6 Adressen der BARs in Hex abgefragt
	//WriteLongS0( 1, 0xADDAFEED, 32 );
	es_status_codes status = es_no_error;
	for (i = 0; i <= entries-1; i++)
	{
		status = ReadLongS0( drvno, &S0Data, i * 4 );
		if (status != es_no_error) return status;
		j += sprintf( fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], S0Data );
	}
	MessageBox( hWnd, fn, "S0 regs", MB_OK );
	return AboutTLPs( drvno );
}//AboutS0

/**
 * \brief Initializes WDC driver.
 * \return enum es_status_codes
 	- es_no_error
	- es_setting_driver_name_failed
	- es_debug_init_failed
	- es_driver_init_failed
	- es_device_not_found
 */
es_status_codes CCDDrvInit()
{
	WDC_Err("start driver init\n");
	//WDC_Err(drvno);
	//depends on os, how big a buffer can be
	BOOL fResult = FALSE;
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
	WDC_Err("set debug options\n");
#if defined(_DEBUG)		
	dwStatus = WDC_SetDebugOptions( WDC_DBG_DEFAULT, NULL );
#else				
	dwStatus = WDC_SetDebugOptions( WDC_DBG_NONE, NULL );
#endif	
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "Failed to initialize debug options for WDC library.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "Failed to initialize debug options for WDC library.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_DriverClose();
		ErrorMsg( "driver closed.\n" );
		return es_debug_init_failed;
	}
	//ErrorMsg("CCDDrvInit start of %x \n", drvno);
	/* Open a handle to the driver and initialize the WDC library */
	WDC_Err("open WDC\n");
***REMOVED***	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "Failed to initialize the WDC library. Error 0x%lx - %s\n",
			dwStatus, Stat2Str( dwStatus ) );
		//doesnt work at this moment before debugsetup
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "Failed to initialize the WDC library. Maybe the driver was not unloaded correctly.\n" );
		WDC_DriverClose();
		ErrorMsg( "driver closed.\n" );
		return es_driver_init_failed;
	}
	BZERO( scanResult );
	WDC_Err("scan PCIe devices\n");
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
	WDC_Err("init HR counter\n");
	TPS = InitHRCounter();//for ticks function
	WDC_Err("driver init done\n");
	return es_no_error;	  // no Error, driver found
}; //CCDDrvInit

/**
 * \brief Frees handle and memory -> exit driver.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_unlocking_dma_failed
 */
es_status_codes CCDDrvExit( UINT32 drvno )
{
	es_status_codes status = es_no_error;
	WDC_Err( "Driver exit, drv: %u\n", drvno);
	if (WDC_IntIsEnabled( hDev[drvno] ))
	{
		WDC_Err( "cleanup dma\n" );
		status = CleanupPCIE_DMA( drvno );
		if (status != es_no_error) return status;
	}
	WDC_DriverClose();
	WDC_PciDeviceClose( hDev[drvno] );
	WDC_Err( "Driver closed and PciDeviceClosed \n" );
	//if (ahCCDDRV[drvno]!=INVALID_HANDLE_VALUE)
	//CloseHandle(ahCCDDRV[drvno]);	   // close driver
	return status;
};

/**
 * @brief Initializes PCIe board.
 * 
 * @param drvno PCIe board identifier.
 * @return es_status_codes:
 *		- es_no_error
 * 		- es_invalid_driver_number
 */
es_status_codes InitBoard( UINT32 drvno )
{
	if ((drvno < 1) || (drvno > 2)) return es_invalid_driver_number;
	//PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
	volatile DWORD dwStatus = 0;
	WDC_Err( "Info: scan result: a board found:%lx , dev=%lx, ven=%lx \n", scanResult.dwNumDevices, scanResult.deviceId[drvno - 1].dwDeviceId, scanResult.deviceId[drvno - 1].dwVendorId );
	//gives the information received from PciScanDevices to PciGetDeviceInfo
	BZERO( deviceInfo[drvno] );
	memcpy( &deviceInfo[drvno].pciSlot, &scanResult.deviceSlot[drvno - 1], sizeof( deviceInfo[drvno].pciSlot ) );

	/* Retrieve the device's resources information */
	dwStatus = WDC_PciGetDeviceInfo( &deviceInfo[drvno] );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "DeviceOpen: Failed retrieving the device's resources information.\n"
			"Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "WDC_PciGetDeviceInfo failed" );
		WDC_DriverClose();
		ErrorMsg( "driver closed.\n" );
		return es_getting_device_info_failed;
	}

	WDC_Err( "Info: device info: bus:%lx , slot=%lx, func=%lx \n", deviceInfo[drvno].pciSlot.dwBus, deviceInfo[drvno].pciSlot.dwSlot, deviceInfo[drvno].pciSlot.dwFunction );
	WDC_Err( "Info: device info: items:%lx , item[0]=%lx \n", deviceInfo[drvno].Card.dwItems, deviceInfo[drvno].Card.Item[0] );

	hDev[drvno] = LSCPCIEJ_DeviceOpen( &deviceInfo[drvno] );
	if (!hDev[drvno])
	{
		printf( "DeviceOpen: Failed opening a handle to the device: %s\n", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "DeviceOpen failed\n" );
		WDC_Err( "DeviceOpen failed %s\n", LSCPCIEJ_GetLastErr() );
		WDC_DriverClose();
		ErrorMsg( "driver closed.\n" );
		return es_open_device_failed;
	}
	PWDC_DEVICE pDev = ((PWDC_DEVICE)hDev[drvno]);
	WDC_Err( "DRVInit hDev id % x, hDev pci slot %x, hDev pci bus %x, hDev pci function %x, hDevNumAddrSp %x \n"
		, pDev->id, pDev->slot.dwSlot, pDev->slot.dwBus, pDev->slot.dwFunction, pDev->dwNumAddrSpaces );
	InitProDLL();
	return es_no_error ;
};  // InitBoard

/**
 * \brief .
 * 
 * \param drvno
 * \param camera_system
 * \param pixel
 * \param trigger_mode
 * \param sensor_type
 * \param ADC_Mode
 * \param ADC_custom_pettern
 * \param led_on
 * \param gain_3010
 * \param gain_3030
 */
es_status_codes initCamera(UINT32 drvno,  UINT8 camera_system, UINT16 pixel, UINT16 trigger_mode, UINT16 sensor_type, UINT8 ADC_Mode, UINT16 ADC_custom_pettern, UINT16 led_on, UINT16 gain_3010, UINT8 gain_3030)
{
	InitCameraGeneral(drvno, pixel, trigger_mode, sensor_type, 0, 0);
	switch (camera_system)
	{
	case camera_system_3001:
		InitCamera3001(drvno, pixel, trigger_mode, sensor_type, 0);
		break;
	case camera_system_3010:
		InitCamera3010(drvno, ADC_Mode, ADC_custom_pettern);
		break;
	case camera_system_3030:
		InitCamera3030(drvno, ADC_Mode, ADC_custom_pettern, gain_3030);
		//TODO use DAC...maybe extra function or sendflcam_dac
		break;
	}
}

/**
 * \brief Initialize Measurement.
 * 
 * \param settings struct global_settings
 * \return es_status_codes
 *		- 
 */
es_status_codes InitMeasurement(struct global_settings* settings)
{
	es_status_codes status = ClearAllUserRegs(settings->drvno);
	if (status != es_no_error) return status;
	status = SetGlobalVariables(settings->drvno, settings->camcnt, settings->pixel, settings->xckdelay);
	if (status != es_no_error) return status;
	status = SetBoardVars( settings->drvno );
	if (status != es_no_error) return status;
	//set PDA and FFT
	status = SetSensorType(settings->drvno, settings->sensor_type);
	if (status != es_no_error) return status;
	if (settings->sensor_type == FFTsensor)
	{
		switch (settings->FFTMode)
		{
			case full_binning:
				status = SetupFullBinning(settings->drvno, settings->FFTLines, settings->Vfreq);
				if (status != es_no_error) return status;
				useSWTrig = FALSE;
				break;
			case partial_binning:
				status = DLLSetupROI(settings->drvno, settings->number_of_regions, settings->FFTLines, settings->keep_first, settings->region_size, settings->Vfreq);
				if (status != es_no_error) return status;
				useSWTrig = TRUE;
				break;
			case area_mode:
				status = DLLSetupArea(settings->drvno, settings->lines_binning, settings->Vfreq);
				if (status != es_no_error) return status;
				useSWTrig = TRUE;
				break;
		}
	}
	else useSWTrig = FALSE;
	//allocate Buffer
	status = SetMeasurementParameters(settings->drvno, settings->nos, settings->nob);
	if (status != es_no_error) return status;
	status = CloseShutter(settings->drvno); //set cooling  off
	if (status != es_no_error) return status;
	//set mshut
	if (settings->mshut)
	{
		status = SetSEC(settings->drvno, settings->ShutterExpTime * 100);
		if (status != es_no_error) return status;
		status = SetTORReg(settings->drvno, TOR_SSHUT);
		if (status != es_no_error) return status;
	}
	else
	{
		status = ResetSEC(settings->drvno);
		if (status != es_no_error) return status;
		status = SetTORReg(settings->drvno, settings->TORmodus);
		if (status != es_no_error) return status;
	}
	//SSlope
	if (settings->sslope == 0)	status = HighSlope(settings->drvno);
	if (settings->sslope == 1)	status = LowSlope(settings->drvno);
	if (settings->sslope == 2)	status = BothSlope(settings->drvno);
	if (status != es_no_error) return status;
	//BSlope
	status = SetBSlope(settings->drvno, settings->bslope);
	if (status != es_no_error) return status;
	//SetTimer
	status = SetSTI(settings->drvno, settings->sti_mode);
	if (status != es_no_error) return status;
	status = SetBTI(settings->drvno, settings->bti_mode);
	if (status != es_no_error) return status;
	status = SetSTimer(settings->drvno, settings->stime_in_microsec);
	if (status != es_no_error) return status;
	status = SetBTimer(settings->drvno, settings->btime_in_microsec);
	if (status != es_no_error) return status;
	if (settings->enable_gpx) status = InitGPX(settings->drvno, settings->gpx_offset);
	if (status != es_no_error) return status;
	//Delay after Trigger
	status = SetSDAT(settings->drvno, settings->sdat_in_100ns);
	if (status != es_no_error) return status;
	status = SetBDAT(settings->drvno, settings->bdat_in_100ns);
	if (status != es_no_error) return status;
	//init Camera
	status = initCamera(settings->drvno, settings->camera_system, settings->pixel, settings->trigger_mode_cc, settings->sensor_type, settings->ADC_Mode, settings->ADC_custom_pettern, settings->led_on, settings->gain_3010, settings->gain_3030);
	if (status != es_no_error) return status;
	//if IR-Sensor
	status = SendFLCAM(settings->drvno, maddr_cam, 0, settings->isIRSensor);
	if (status != es_no_error) return status;
	//for coooled Cam
	status = SetTemp(settings->drvno, settings->Temp_level);
	if (status != es_no_error) return status;
	//DAC
	//TODO: Move DAC to CAM 3030
	int dac_channel_count = 8;
	if (settings->dac) {
		SendFLCAM_DAC(settings->drvno, dac_channel_count, 0, 0, 1);
		int reorder_ch[8] = { 3, 4, 0, 5, 1, 6, 2, 7 };
		for (UINT8 channel = 0; channel < dac_channel_count; channel++)
		{
			status = DAC_setOutput(settings->drvno, channel, settings->dac_output[reorder_ch[channel]]);
			if (status != es_no_error) return status;
		}
	}
	//DMA
	SetupPCIE_DMA(settings->drvno);
	//TODO set cont FF mode with DLL style(CONTFFLOOP = activate;//0 or 1;CONTPAUSE = pause;) or CCDExamp style(check it out)
	//TODO  SetBEC( choosen_board, Bec );
	BOARD_SEL = settings->board_sel;
	return status;
}

es_status_codes StartMeasurement()
{
	ReadFFLoopThread();
	return;
}

/**
 * \brief Set DMA register.
 * 
 * \param Data
 * \param Bitmask
 * \param Address
 * \param drvno PCIe board identifier.
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetDMAReg( ULONG Data, ULONG Bitmask, ULONG Address, UINT32 drvno )
{//the bitmask have "1" on the data dates like Bitmask: 1110 Data:1010 
	ULONG OldRegisterValues;
	ULONG NewRegisterValues;
	//read the old Register Values in the DMA Address Reg
	es_status_codes status = ReadLongDMA(drvno, &OldRegisterValues, Address);
	if (status != es_no_error)
	{
		ErrLog("ReadLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
		return status;
	}
	//save the bits, which shall not changed
	OldRegisterValues = OldRegisterValues & ~Bitmask; //delete the bits, which are "1" in the bitmask
	Data = Data & Bitmask; //to make sure that there are no bits, where the bitmask isnt set
	NewRegisterValues = Data | OldRegisterValues;
	//write the data to the DMA controller
	status = WriteLongDMA(drvno, NewRegisterValues, Address);
	if (status != es_no_error)
	{
		ErrLog("WriteLong DMA Failed in SetDMAReg \n");
		WDC_Err("%s", LSCPCIEJ_GetLastErr());
	}
	return status;
}

/**
 * \brief
 * 
 * \param PhysAddrDMABuf64
 * \param tlpSize
 * \param no_tlps
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetDMAAddrTlpRegs( UINT64 PhysAddrDMABuf64, ULONG tlpSize, ULONG no_tlps, UINT32 drvno )
{
	UINT64 PhysAddrDMABuf;
	ULONG RegisterValues;
	ULONG wr_addr_64bit_en = 0x1 << 19; //bit 19 is for 64bit enable
	ULONG BitMask;
	//proof the physical address for the max. size of the DMA Controller
	/*this error never happens. I think there is no need for this check
	if (PhysAddrDMABuf64 > 0x3fffffffff){
	ErrLog("Physical DMA buffer address is more than 5bytes: 0x%016x \n",
	PhysAddrDMABuf64);
	WDC_Err("%s", LSCPCIEJ_GetLastErr());
	return FALSE;
	}*/
	//WDMATLPA (Reg name): write the lower part (bit 02:31) of the DMA adress to the DMA controller
	//WDC_Err("64 bit Address 0x%llx\n", PhysAddrDMABuf64);
	RegisterValues = (ULONG)PhysAddrDMABuf64;
	BitMask = 0xFFFFFFFC;
	//WDC_Err("lower part of 64 bit Address 0x%x\n", RegisterValues);
	es_status_codes status = SetDMAReg(RegisterValues, BitMask, DmaAddr_WDMATLPA, drvno);
	if (status != es_no_error)
	{
		WDC_Err("Set the lower part of the DMA Address failed");
		return status;
	}
	//WDMATLPS: write the upper part (bit 32:39) of the address	
	PhysAddrDMABuf = ((UINT64)PhysAddrDMABuf64 >> 32);
	//WDC_Err("upper part of 64 bit Address 0x%x\n", PhysAddrDMABuf);
	//PhysAddrDMABuf >> 32;	//shift to the upper part 
	PhysAddrDMABuf = PhysAddrDMABuf << 24;		//shift to prepare for the Register
	BitMask = 0xFF081FFF;
	RegisterValues = (ULONG)PhysAddrDMABuf;
	RegisterValues |= tlpSize;
	//64bit address enable
	if (DMA_64BIT_EN)
		RegisterValues |= wr_addr_64bit_en;
	status = SetDMAReg(RegisterValues, BitMask, DmaAddr_WDMATLPS, drvno);
	if (status != es_no_error)
	{
		WDC_Err("Set the upper part of the DMA Address and the TLPsize failed");
		return status;
	}
	//WDMATLPC: Set the number of DMA transfer count
	BitMask = 0xFFFF;
	status = SetDMAReg(no_tlps, BitMask, DmaAddr_WDMATLPC, drvno);
	if (status != es_no_error)
		WDC_Err("Set the number of DMA transfer count failed");
	return status;
}//SetDMAAddrTlpRegs

/**
 * \brief
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetDMAAddrTlp( UINT32 drvno )
{
	WD_DMA **ppDma = &dmaBufferInfos[drvno];
	UINT64 PhysAddrDMABuf64;
	ULONG BitMask;
	//ULONG BData = 0; //-> ersetzt durch globale variable BDATA
	int tlpmode = 0;
	es_status_codes status = ReadLongIOPort( drvno, &BDATA, PCIeAddr_devCap );
	if (status != es_no_error) return status;
	tlpmode = BDATA & 0x7;//0xE0 ;
	//tlpmode = tlpmode >> 5;
	if (_FORCETLPS128) tlpmode = 0;
	BDATA &= 0xFFFFFF1F;//delete the old values
	BDATA |= (0x2 << 12);//set maxreadrequestsize
	switch (tlpmode)
	{
	case 0:
		BDATA |= 0x00;//set to 128 bytes = 32 DWORDS 
		//BData |= 0x00000020;//set from 128 to 256 
		//WriteLongIOPort( drvno, BData, PCIeAddr_devStatCtrl );
		//NO_TLPS setup now in setboardvars
		TLPSIZE = 0x20;
		break;
	case 1:
		BDATA |= 0x20;//set to 256 bytes = 64 DWORDS 
		//BData |= 0x00000020;//set to 256 
		//WriteLongIOPort( drvno, BData, PCIeAddr_devStatCtrl );
		//NO_TLPS = 0x9;//x9 was before. 0x10 is calculated in aboutlp and 0x7 is working;
		TLPSIZE = 0x40;
		break;
	case 2:
		BDATA |= 0x40;//set to 512 bytes = 128 DWORDS 
		//BData |= 0x00000020;//set to 512 
		//WriteLongIOPort( drvno, BData, PCIeAddr_devStatCtrl );
		//NO_TLPS = 0x5;
		TLPSIZE = 0x80;
		break;
	}
	//ValMsg( BData ); //TESTPOINT
	if (MANUAL_OVERRIDE_TLP)
		SetManualTLP_vars();
	status = WriteLongIOPort( drvno, BDATA, PCIeAddr_devStatCtrl );
	if (status != es_no_error) return status;
	PhysAddrDMABuf64 = (*ppDma)->Page[0].pPhysicalAddr;
	return SetDMAAddrTlpRegs(PhysAddrDMABuf64, TLPSIZE, NO_TLPS, drvno);
}

void SetManualTLP_vars()
{
	BDATA  |= 0x00; // sets max TLP size {0x00, 0x20, 0x40} <=> {128 B, 256 B, 512 B}
	TLPSIZE = 0x20; // sets utilized TLP size in DWORDS (1 DWORD = 4 byte)
	NO_TLPS = 0x11; // sets number of TLPs per scan/frame
	return;
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
es_status_codes SetDMABufRegs( UINT32 drvno )
{
	//DMABufSizeInScans - use 1 block
	es_status_codes status = SetS0Reg(DMA_BUFFER_SIZE_IN_SCANS, 0xffffffff, S0Addr_DmaBufSizeInScans, drvno);
	if (status != es_no_error) return status;
	//scans per intr must be 2x per DMA_BUFFER_SIZE_IN_SCANS to copy hi/lo part
	//aCAMCNT: double the INTR if 2 cams
	status = SetS0Reg(DMA_DMASPERINTR, 0xffffffff, S0Addr_DMAsPerIntr, drvno);
	if (status != es_no_error) return status;
	WDC_Err( "scansPerInterrupt/camcnt: %x \n", DMA_DMASPERINTR / aCAMCNT[drvno] );
	status = SetS0Reg(*Nospb, 0xffffffff, S0Addr_NOS, drvno);
	if (status != es_no_error) return status;
	status = SetS0Reg(Nob, 0xffffffff, S0Addr_NOB, drvno);
	if (status != es_no_error) return status;
	return SetS0Reg(aCAMCNT[drvno], 0xffffffff, S0Addr_CAMCNT, drvno);
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
es_status_codes SetDMAReset( UINT32 drvno )
{
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
}

es_status_codes SetDMAStart( UINT32 drvno )
{
	ULONG BitMask = 0x1;
	ULONG RegisterValues = 0x1;
	return SetDMAReg(RegisterValues, BitMask, DmaAddr_DDMACR, drvno);
}

//BOOL SendDMAInfoToKP(void){
//
//	DWORD hDma;
//	DWORD dwOptions;
//	PVOID pData;
//	DWORD dwResult;
//	PDWORD	pdwResult = &dwResult;
//
//
//	WDC_Err("WDC:  hDma %u\n", dmaBufferInfos->hDma);
//	WDC_Err("WDC:  pUserAddr %u\n", dmaBufferInfos->pUserAddr);
//	WDC_Err("WDC:  pKernelAddr %u\n", dmaBufferInfos->pKernelAddr);
//	WDC_Err("WDC:  dwBytes %u\n", dmaBufferInfos->dwBytes);
//	WDC_Err("WDC:  dwOptions %u\n", dmaBufferInfos->dwOptions);
//	WDC_Err("WDC:  dwPages %u\n", dmaBufferInfos->dwPages);
//	WDC_Err("WDC:  hCard %u\n", dmaBufferInfos->hCard);
//
//	/*for WDK Flush
//
//	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_BUFSIZE, pData, pdwResult);
//	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
//	{
//	WDC_Err("sendDMAInfoToKP dwDMASubBufSize send failed\n");
//	return FALSE;
//	}
//	*/
//	//for JUNGO Flush, complete WD_DMA struct
//	/*
//	pData = dmaBufferInfos;
//	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_WDDMA, pData, pdwResult);
//	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
//	{
//	WDC_Err("sendDMAInfoToKP WD_DMA send failed\n");
//	return FALSE;
//	}
//	*/
//	//for Testfkt to write to the Regs
//	/*
//	pData = &hDev;// &deviceInfo.Card;//
//	WDC_Err("deviceInfo.Card.Item[0].I.Mem.pTransAddr : 0x %x\n", deviceInfo.Card.Item[0].I.Mem.pTransAddr);
//	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_TESTSIGNALPREP, pData, pdwResult);
//	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
//	{
//	WDC_Err("sendDMAInfoToKP hDev send failed\n");
//	return FALSE;
//	}
//	*/
//	/*
//	//for JUNGO Flush , fragmented WD_DMA struct
//	dwOptions = dmaBufferInfos->dwPages; //Im using dwPages instead of dwOptions
//
//	pData = &dwOptions;
//	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_DWOPT, pData, pdwResult);
//	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
//	{
//	WDC_Err("sendDMAInfoToKP dwOptions send failed\n");
//	return FALSE;
//	}
//
//	//send hDma of WD_DMA struct to KP
//	hDma = dmaBufferInfos->dwBytes;	 //Im using pUserAddr instead of hDma
//	//because the structure is mixed. I think it is a bug.
//	//Jungo says our dll or h files are mixed up
//	pData = &hDma;
//	WDC_CallKerPlug(hDev, KP_LSCPCIEJ_MSG_HDMA, pData, pdwResult);
//	if (*pdwResult != KP_LSCPCIEJ_STATUS_OK)
//	{
//	WDC_Err("sendDMAInfoToKP hDma send failed\n");
//	return FALSE;
//	}
//	return TRUE;
//}

ULONG GetScanindex( UINT32 drvno )
{
	UINT32 ldata = 0;
	if (!ReadLongS0( drvno, &ldata, S0Addr_ScanIndex ))
	{
		ErrorMsg( "Error GetScanindex" );
		return 0;
	}
	return ldata;
}

/**
 * \brief For the rest part of the buffer.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes GetLastBufPart( UINT32 drvno )
{
	//get the rest if buffer is not multiple of 500 (BUFSIZEINSCANS/2)
	//also if nos is < BUFSIZEINSCANS/2 - here: no intr occurs
	UINT32 spi = 0;
	es_status_codes status = ReadLongS0( drvno, &spi, S0Addr_DMAsPerIntr ); //get scans per intr
	if (status != es_no_error) return status;
	//halfbufize is 500 with default values
	UINT32 dmaHalfBufferSize = DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS;
	UINT32 scans_all_cams = (*Nospb) * Nob * aCAMCNT[drvno];
	UINT32 rest_overall = scans_all_cams % dmaHalfBufferSize;
	size_t rest_in_bytes = rest_overall * aPIXEL[drvno] * sizeof( USHORT );
	WDC_Err( "*GetLastBufPart():\n" );
	WDC_Err( "nos: 0x%x, nob: 0x%x, scansPerInterrupt: 0x%x, camcnt: 0x%x\n", (*Nospb), Nob, spi, aCAMCNT[drvno]);
	WDC_Err( "scans_all_cams: 0x%x \n", scans_all_cams );
	WDC_Err( "rest_overall: 0x%x, rest_in_bytes: 0x%x\n", rest_overall, rest_in_bytes );
	WDC_Err( "dmaBufferSizeInBytes: 0x%x \n", dmaBufferSizeInBytes );
	if (rest_overall)
	{
		WDC_Err( "has rest_overall:\n" );
		INT_PTR dmaBufferReadPos = dmaBuffer[drvno];
		dmaBufferReadPos += dmaBufferPartReadPos[drvno] * dmaBufferSizeInBytes / DMA_BUFFER_PARTS;
		memcpy( userBufferWritePos[drvno], dmaBufferReadPos, rest_in_bytes );
	}
	return status;
}//GetLastBufPart

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

VOID DLLCALLCONV interrupt_handler1( PVOID pData ) { isr( 1, pData ); }

VOID DLLCALLCONV interrupt_handler2( PVOID pData ) { isr( 2, pData ); }

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
es_status_codes SetupPCIE_DMA( UINT32 drvno )
{
	DWORD dwStatus;
	WDC_Err( "entered SetupPCIE_DMA\n" );
	//If interrupt was enabled before, first cleanup DMA.
	if (WDC_IntIsEnabled(hDev[drvno]))
	{
		WDC_Err("cleanup dma\n");
		es_status_codes status = CleanupPCIE_DMA(drvno);
		if (status != es_no_error) return status;
	}
	dmaBufferSizeInBytes = DMA_BUFFER_SIZE_IN_SCANS * aPIXEL[drvno] * sizeof( UINT16 );
	DWORD dwOptions = DMA_FROM_DEVICE | DMA_KERNEL_BUFFER_ALLOC;// | DMA_ALLOW_64BIT_ADDRESS;// DMA_ALLOW_CACHE ;
	if (DMA_64BIT_EN)
		dwOptions |= DMA_ALLOW_64BIT_ADDRESS;
#if (DMA_SGBUF)
	if (!pDMABigBuf)
	{
		ErrLog( "Failed: buf pointer not valid.\n" );
		WDC_Err( "%s", "Failed: buf pointer not valid.\n" );
		ErrorMsg( "DMA buffer addr is not valid" );
		return es_getting_dma_buffer_failed;
	}
	// pDMABigBuf is the big space which is passed to this function = input - must be global
	dwStatus = WDC_DMASGBufLock( hDev[drvno], pDMABigBuf, dwOptions, dmaBufferSizeInBytes, &dmaBufferInfos ); //size in Bytes
#endif
#if (DMA_CONTIGBUF)		//usually we use contig buf: here we get the buffer address from labview.
	// dmaBuffer is the space which is allocated by this function = output - must be global
	dwStatus = WDC_DMAContigBufLock( hDev[drvno], &dmaBuffer[drvno], dwOptions, dmaBufferSizeInBytes, &dmaBufferInfos[drvno] ); //size in Bytes
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "Failed locking a contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "DMA buffer not sufficient" );
		return es_getting_dma_buffer_failed;
	}
	// data must be copied afterwards to user Buffer 
#endif
	//set Init Regs
	es_status_codes status = SetDMAAddrTlp(drvno);
	if (status != es_no_error)
	{
		ErrLog( "DMARegisterInit for TLP and Addr failed \n" );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		ErrorMsg( "DMARegisterInit for TLP and Addr failed" );
		return status;
	}
	// DREQ: every XCK h->l starts DMA by hardware
	//set hardware start des dma  via DREQ withe data = 0x4000000
	ULONG mask = 0x40000000;
	ULONG data = 0;// 0x40000000;
	if (HWDREQ_EN)
		data = 0x40000000;
	status = SetS0Reg( data, mask, S0Addr_IRQREG, drvno );
	if (status != es_no_error) return status;
	// Enable DMA interrupts (if not polling)
	// INTR should copy DMA buffer to user buf: 
	if (INTR_EN)
	{
		switch (drvno)
		{
		case 1:
			dwStatus = LSCPCIEJ_IntEnable( hDev[drvno], interrupt_handler1 );
			if (WD_STATUS_SUCCESS != dwStatus)
			{
				WDC_Err( "Failed to enable the Interrupts1. Error 0x%lx - %s\n",
					dwStatus, Stat2Str( dwStatus ) );
				//(ErrorMsg("Failed to enable the Interrupts");
				return es_enabling_interrupts_failed;
			}
			break;

		case 2:
			dwStatus = LSCPCIEJ_IntEnable( hDev[drvno], interrupt_handler2 );
			if (WD_STATUS_SUCCESS != dwStatus)
			{
				WDC_Err( "Failed to enable the Interrupts2. Error 0x%lx - %s\n",
					dwStatus, Stat2Str( dwStatus ) );
				//ErrorMsg("Failed to enable the Interrupts");
				return es_enabling_interrupts_failed;
			}
			break;
		}
	}
	WDC_Err( "finished SetupDMA\n" );
	return status;
}//SetupPCIE_DMA

/**
\brief Starts transfer from PCIe board to PCs main RAM
*/
void StartPCIE_DMAWrite( UINT32 drvno )
{
	if (!HWDREQ_EN)
	{

		// DCSR: set the Iniator Reset 
		SetDMAReset( drvno );

		/* Flush the I/O caches (see documentation of WDC_DMASyncIo()) */
		//WDC_DMASyncIo(dmaBufferInfos);
		/****DMA Transfer start***/
		/* Flush the CPU caches (see documentation of WDC_DMASyncCpu()) */
		//WDC_DMASyncCpu(dmaBufferInfos);

		//SetDMADataPattern();
		/* DDMACR: Start DMA - write to the device to initiate the DMA transfer */
		SetDMAStart( drvno );
	}
}

/**
 * \brief
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_unlocking_dma_failed
 */
es_status_codes CleanupPCIE_DMA( UINT32 drvno )
{
	/* Disable DMA interrupts */
	WDC_IntDisable( hDev[drvno] );
	/* Unlock and free the DMA buffer */
	DWORD dwStatus = WDC_DMABufUnlock( dmaBufferInfos[drvno] );
	if (WD_STATUS_SUCCESS != dwStatus)
	{
		ErrLog( "Failed unlocking a contiguous DMA buffer. Error 0x%lx - %s\n", dwStatus, Stat2Str( dwStatus ) );
		WDC_Err( "%s", LSCPCIEJ_GetLastErr() );
		return es_unlocking_dma_failed;
	}
	WDC_Err( "Unlock DMABuf Successfull\n" );
	return es_no_error;
}

int GetNumofProcessors()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo( &siSysInfo );
	return siSysInfo.dwNumberOfProcessors;
}//GetNumofProcessors

/**
\brief Set global variables camcnt, pixel and TLP size depending on pixel. Best call before doing anything else.
\param drvno PCIe board identifier
\param camcnt camera count
\param pixel pixel count
\param xckdelay XCK delay
\return es_status_codes
	- es_no_error
	- es_invalid_pixel_count
*/
es_status_codes SetGlobalVariables( UINT32 drvno, UINT32 camcnt, UINT32 pixel, UINT32 xckdelay )
{
	WDC_Err("Setting global variables: drv: %u, camcnt: %u, pixel: %u, xckdelay: %u\n", drvno, camcnt, pixel, xckdelay);
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
	switch (pixel)
	{
	case 128:
		NO_TLPS = 0x2;//3
		break;
	case 192:
		NO_TLPS = 0x3;//4
		break;
	case 320:
		NO_TLPS = 0x5;//6
		break;
	case 576:
		NO_TLPS = 0x9;//a
		break;
	case 1088:
		NO_TLPS = 0x11;
		break;
	case 2112:
		NO_TLPS = 0x21;//22
		break;
	case 4160:
		NO_TLPS = 0x41;//42
		break;
	case 8256:
		NO_TLPS = 0x81;//82
		break;
	default:
		if (!MANUAL_OVERRIDE_TLP)
		{
			WDC_Err("Could not choose TLP size, no valid pixel count.\n");
			return es_invalid_pixel_count;
		}
	}
	if (LEGACY_202_14_TLPCNT) NO_TLPS = NO_TLPS + 1;
	aPIXEL[drvno] = pixel;
	aCAMCNT[drvno] = camcnt;
	ADRDELAY = xckdelay;
	return es_no_error;
}

/**
 * \brief Initiates board registers.
 *
 * \param drvno PCIe board identifier
 * \return es_status_codes
 * 	- es_no_error
 * 	- es_invalid_driver_handle
 * 	- es_register_write_failed
 *	- es_register_read_failed
 */
es_status_codes SetBoardVars( UINT32 drvno )
{
	if (hDev[drvno] == INVALID_HANDLE_VALUE)
	{
		WDC_Err( "Handle is invalid of drvno: %i", drvno );
		return es_invalid_driver_handle;
	}
	//set startval for CTRLA Reg  +slope, IFC=h, VON=1 
	es_status_codes status = WriteByteS0(drvno, 0x23, S0Addr_CTRLA);
	if (status != es_no_error) return status;
	//write CTRLB reg in S0
	status = WriteByteS0(drvno, 0, S0Addr_CTRLB);
	if (status != es_no_error) return status;
	//write CTRLC reg in S0
	status = WriteByteS0(drvno, 0, S0Addr_CTRLC);
	if (status != es_no_error) return status;
	//write pixel to PIXREG  & stop timer & int trig
	status = SetS0Reg(aPIXEL[drvno], 0xFFFF, S0Addr_PIXREGlow, drvno);
	if (status != es_no_error) return status;
	status = SetS0Reg(aCAMCNT[drvno], 0xF, S0Addr_CAMCNT, drvno);
	if (status != es_no_error) return status;
	status = SetS0Reg(ADRDELAY, 0xFFFF, S0Addr_XCKDLY, drvno);
	WDC_Err( "*** SetBoardVars done.\n" );
	return status; //no error
};  // SetBoardVars


/**
 * Allocate user memory.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes
 *		- es_no_error
 *		- es_allocating_memory_failed
 *		- es_not_enough_ram
 */
es_status_codes allocateUserMemory( UINT32 drvno )
{
	//free old memory before allocating new one
	free( userBuffer[drvno] );
	UINT64 memory_all = 0;
	UINT64 memory_free = 0;
	FreeMemInfo( &memory_all, &memory_free );
	INT64 memory_free_mb = memory_free / (1024 * 1024);
	INT64 needed_mem = (INT64)aCAMCNT[drvno] * (INT64)Nob * (INT64)*Nospb * (INT64)aPIXEL[drvno] * (INT64)sizeof( USHORT );
	INT64 needed_mem_mb = needed_mem / (1024 * 1024);
	WDC_Err( "available memory:%lld MB\n \tmemory needed: %lld MB\n", memory_free_mb, needed_mem_mb );
	//check if enough space is available in the physical ram
	if (memory_free > (UINT64)needed_mem)
	{
		// sometimes it makes one ISR more, so better to allocate nos+1 thaT IN THIS CASE THE ADDRESS pDMAIndex is valid
		//B! "2 *" because the buffer is just 2/3 of the needed size. +1 oder *2 weil sonst absturz im continuous mode
		UINT16* userBufferTemp = calloc( (UINT64)aCAMCNT[drvno] * (UINT64)(*Nospb) * (UINT64)Nob * (UINT64)aPIXEL[drvno], sizeof( UINT16 ) );
		if (userBufferTemp)
		{
			userBuffer[drvno] = userBufferTemp;
			return es_no_error;
		}
		else
		{
			WDC_Err( "Allocating user memory failed.\n" );
			return es_allocating_memory_failed;
		}
	}
	else
	{
		ErrorMsg( "Not enough physical RAM available!" );
		WDC_Err( "ERROR for buffer %d: available memory: %lld MB \n \tmemory needed: %lld MB\n", number_of_boards, memory_free_mb, needed_mem_mb );
		return es_not_enough_ram;
	}
}

/**
 * \brief Clears DAT and EC.
 * 
 * \param drv PCIe board identifier.
 * \return es_status_codes 
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes ClearAllUserRegs(UINT32 drv)
{
	es_status_codes status = WriteLongS0( drv, 0, S0Addr_BDAT );
	if (status != es_no_error) return status;
	status = WriteLongS0( drv, 0, S0Addr_BEC );
	if (status != es_no_error) return status;
	status = WriteLongS0( drv, 0, S0Addr_SDAT );
	if (status != es_no_error) return status;
	return WriteLongS0( drv, 0, S0Addr_SEC );
} //ClearAllUserRegs


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
es_status_codes AboutDrv( UINT32 drvno )
{
	char pstring[80] = "";
	HWND hWnd = GetActiveWindow();
	HDC aDC = GetDC( hWnd );
	// read ISA Id from S0Base+7
	UINT32 S0Data = 0;
	es_status_codes status = ReadLongS0( drvno, &S0Data, S0Addr_CTRLA ); // Board ID =5053
	if (status != es_no_error) return status;
	S0Data = S0Data >> 16;
	//or
	//S0Data = (UCHAR)ReadByteS0(8); // ID=53
	sprintf_s( pstring, 80, " Board #%i    ID = 0x%I32x", drvno, S0Data );
	if (MessageBox( hWnd, pstring, " Board ID=53 ", MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	//ReadLongIOPort(drvno, &S0Data, 0); //read LCR0 for check length 0xffffffco
	//S0Data = ~S0Data; //length is inverted
	//GS
	//The following lines doesn't make sense. S0Data is never 0.
	S0Data = 0x07FF;
	if (S0Data == 0)
	{ 
		ErrorMsg( "Board #%i  no Space0!", drvno );
		return es_no_space0;
	}
	sprintf_s( pstring, 80, "Board #%i     length = 0x%I32x", drvno, S0Data );
	if (MessageBox( hWnd, pstring, "  PCI space0 length=", MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	UCHAR udata1 = 0,
		udata2 = 0,
		udata3 = 0,
		udata4 = 0;
	if (S0Data >= 0x1F)
	{//if WE -> has space 0x20
		status = ReadByteS0( drvno, &udata1, 0x1C );
		if (status != es_no_error) return status;
		status = ReadByteS0( drvno, &udata2, 0x1D );
		if (status != es_no_error) return status;
		status = ReadByteS0( drvno, &udata3, 0x1E );
		if (status != es_no_error) return status;
		status = ReadByteS0( drvno, &udata4, 0x1F );
		if (status != es_no_error) return status;
		sprintf_s( pstring, 80, "Board #%i  ven ID = %c%c%c%c", drvno, udata1, udata2, udata3, udata4 );
		if (MessageBox( hWnd, pstring, " Board vendor=EBST ", MB_OK | MB_ICONEXCLAMATION ) == IDOK);
	}
	if (S0Data >= 0x3F)
	{//if 9056 -> has space 0x40
		status = ReadLongS0( drvno, &S0Data, S0Addr_PCI );
		if (status != es_no_error) return status;
		sprintf_s( pstring, 80, "Board #%i   board version = 0x%I32x", drvno, S0Data );
		if (MessageBox( hWnd, pstring, "Board version ", MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	}
	ReleaseDC( hWnd, aDC );
	return es_no_error;
};

/* functions for managing controlbits in CtrlA register

the bits of CtrlA register have these functions:

DB5		DB4		  DB3			DB2		DB1		DB0
Slope	BothSlope TrigOut		XCK		IFC		V_ON
1: pos	1: on     1: high		1: high	1: high	1: high	V=1
0: neg	0: off    0: low		0: low	0: low	0: low	V=VFAK

D6, D7 have only read function

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

/**
 * \brief Set the external trigger slope to low (PCI Reg CrtlA:D5 -> manual).
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes LowSlope( UINT32 drvno )
{// clear bit D5
	BYTE CtrlA = 0;
	es_status_codes status = NotBothSlope( drvno );
	if (status != es_no_error) return status;
	status = ReadByteS0( drvno, &CtrlA, S0Addr_CTRLA );
	if (status != es_no_error) return status;
	CtrlA &= 0x0df;
	return WriteByteS0( drvno, CtrlA, S0Addr_CTRLA );
}; //LowSlope

/**
 * \brief Functions for managing controlbits in CtrlA register. Set input Trigger slope high.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes HighSlope( UINT32 drvno )
{// set bit D5
	BYTE CtrlA = 0;
	NotBothSlope( drvno );
	es_status_codes status = ReadByteS0( drvno, &CtrlA, S0Addr_CTRLA );
	if (status != es_no_error) return status;
	CtrlA |= 0x20;
	return WriteByteS0( drvno, CtrlA, S0Addr_CTRLA );
}; //HighSlope

/**
 * \brief Set trigger input to pos. & neg. slope.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
es_status_codes BothSlope( UINT32 drvno )
{// set bit D4
	BYTE CtrlA = 0;
	es_status_codes status = HighSlope( drvno );
	if (status != es_no_error) return status;
	status = ReadByteS0( drvno, &CtrlA, S0Addr_CTRLA );
	if (status != es_no_error) return status;
	CtrlA |= 0x10;
	return WriteByteS0( drvno, CtrlA, S0Addr_CTRLA );
}; //BothSlope

/**
 * \brief Reset both slopes.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes NotBothSlope( UINT32 drvno )
{// set bit D4
	BYTE CtrlA = 0;
	es_status_codes status = ReadByteS0( drvno, &CtrlA, S0Addr_CTRLA );
	if (status != es_no_error) return status;
	CtrlA &= 0xEF;
	return WriteByteS0( drvno, CtrlA, S0Addr_CTRLA );
}; //NotBothSlope

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines High-Signals an Pin 17                                      */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \brief Reset trigger out(Reg CtrlA:D3) of PCI board. Can be used to control timing issues in software.
 * 
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * Functions is not optimized for 2 cams.
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OutTrigLow( UINT32 drvno )
{
	BYTE CtrlA;
	es_status_codes status = ReadByteS0( drvno, &CtrlA, S0Addr_CTRLA );
	if (status != es_no_error) return status;
	CtrlA &= 0xf7;
	return WriteByteS0( drvno, CtrlA, S0Addr_CTRLA );
};						//OutTrigLow

/*---------------------------------------------------------------------------*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines Low-Signals an Pin 17                                       */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
es_status_codes OutTrigHigh( UINT32 drvno )
{
	BYTE CtrlA = 0;
	es_status_codes status = ReadByteS0( drvno, &CtrlA, S0Addr_CTRLA );
	if (status != es_no_error) return status;
	CtrlA |= 0x08;
	return WriteByteS0( drvno, CtrlA, S0Addr_CTRLA );
}; //OutTrigHigh

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Ausgabe eines PulseWidth breiten Rechteckpulses an Pin 17                 */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
es_status_codes OutTrigPulse( UINT32 drvno, ULONG PulseWidth )
{
	es_status_codes status = OutTrigHigh( drvno );
	if (status != es_no_error) return status;
	Sleep( PulseWidth );
	return OutTrigLow( drvno );
};

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
es_status_codes WaitTrigger( UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *AbrKey )
{
	BOOL FirstLo = FALSE;
	BOOL HiEdge = FALSE;
	BOOL Abbr = FALSE;
	BOOL Space = FALSE;
	UCHAR ReturnKey = 0;
	BYTE ReadTrigPin = 0;
	es_status_codes status = es_no_error;
	do
	{
		if (ExtTrigFlag)
		{
			status = ReadByteS0( drvno, &ReadTrigPin, S0Addr_CTRLA );
			if (status != es_no_error) return status;
			ReadTrigPin &= 0x040;
			if (ReadTrigPin == 0) FirstLo = TRUE; //first look for lo
			if (FirstLo) { if (ReadTrigPin > 0) HiEdge = TRUE; }; // then look for hi
		}
		else HiEdge = TRUE;
		if (GetAsyncKeyState( VK_ESCAPE ))
			Abbr = TRUE;
		if (GetAsyncKeyState( VK_SPACE ))  Space = TRUE;
	}
	while ((!HiEdge) && (!Abbr));
	if (Abbr) *AbrKey = TRUE;	//stops immediately
	if (Space) *SpaceKey = TRUE;	//stops after next trigger
	return status;
};// WaitTrigger

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \brief Sets the IFC Bit of Interface for sensors with shutter function. IFC=low
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes CloseShutter( UINT32 drvno )   // ehemals IFC = low, in CTRLA
{
	//This function does a bit set. Unfortunately the following line doesn't work. We don't know why, yet. Maybe there is a problem iin ResetS0Bit. -FH, BB
	//ResetS0Bit(CTRLB_bitindex_SHON, S0Addr_CTRLB, drvno);
	UCHAR CtrlB = 0;
	es_status_codes status = ReadByteS0(drvno, &CtrlB, S0Addr_CTRLB);
	if (status != es_no_error) return status;
	CtrlB &= ~0x08; // clr bit D3 (MSHT) in CtrlB, ehemals 0x0fd;	/* $FD = 1111 1101 */
	return WriteByteS0(drvno, CtrlB, S0Addr_CTRLB);
}; //CloseShutter

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \brief Open shutter for sensors with EC (exposure control) / sets IFC signal = high.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes OpenShutter( UINT32 drvno )   // ehemals IFC = low, in CTRLA
{
	//This function does a bit set. Unfortunately the following line doesn't work. We don't know why, yet. Maybe there is a problem in SetS0Bit. -FH, BB
	//SetS0Bit(CTRLB_bitindex_SHON, S0Addr_CTRLB, drvno);
	UCHAR CtrlB = 0;
	es_status_codes status = ReadByteS0(drvno, &CtrlB, S0Addr_CTRLB);
	if (status != es_no_error) return status;
	CtrlB |= 0x08; // set bit D3 (MSUT) in CtrlB
	return WriteByteS0(drvno, CtrlB, S0Addr_CTRLB);
}; //OpenShutter

BOOL GetShutterState( UINT32 drvno )
{
	UCHAR CtrlB;
	ReadByteS0( drvno, &CtrlB, S0Addr_CTRLB );
	CtrlB &= CTRLB_bit_SHON; // read bit D3 (MSUT) in CtrlB
	if (CtrlB == 0) return FALSE;
	return TRUE;
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
es_status_codes SetSDAT( UINT32 drvno, UINT32 datin100ns )
{
	if (datin100ns)
	{
		datin100ns |= 0x80000000; // enable delay
		return WriteLongS0(drvno, datin100ns, S0Addr_SDAT);
	}
	else return WriteLongS0(drvno, 0, S0Addr_SDAT);
}; //SetDAT

/**
 * \brief Sets delay after trigger hardware register.
 * 
 * \param drvno PCIe board identifier.
 * \param datin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetBDAT( UINT32 drvno, UINT32 datin100ns )
{
	if (datin100ns)
	{
		datin100ns |= 0x80000000; // enable delay
		return WriteLongS0(drvno, datin100ns, S0Addr_BDAT);
	}
	else return WriteLongS0(drvno, 0, S0Addr_BDAT);
}; //SetDAT

/**
 * \brief Exposure control (EC) signal is used for mechanical shutter or sensors with EC function.
 * 
 * Starts after delay after trigger (DAT) signal and is active for ecin100ns.
 * \param drvno PCIe board identifier
 * \param ecin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetSEC( UINT32 drvno, UINT32 ecin100ns )
{
	//ULONG data = 0;
	//ReadLongS0(drvno, &data, S0Addr_EC);
	//ecin100ns |= data;
	ecin100ns |= 0x80000000; // enable delay
	return WriteLongS0( drvno, ecin100ns, S0Addr_SEC );
}; //SetEC

/**
 * \brief Resets additional delay after trigger hardware register.
 * 
 * \param drvno PCIe board identifier
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes ResetSEC( UINT32 drvno )
{
	return WriteLongS0( drvno, 0, S0Addr_SEC );
}; //ResetEC

/**
 * \brief Exposure control (EC) signal is used for mechanical shutter or sensors with EC function.
 * 
 * Starts after delay after trigger (DAT) signal and is active for ecin100ns.
 * \param drvno PCIe board identifier
 * \param ecin100ns Time in 100 ns steps.
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes SetBEC( UINT32 drvno, UINT32 ecin100ns )
{
	//ULONG data = 0;
	//ReadLongS0(drvno, &data, S0Addr_EC);
	//ecin100ns |= data;
	ecin100ns |= 0x80000000; // enable delay
	return WriteLongS0( drvno, ecin100ns, S0Addr_BEC );
}; //SetEC

/**
 * \brief Resets additional delay after trigger hardware register.
 * 
 * \param drvno PCIe board identifier
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_write_failed
 */
es_status_codes ResetBEC( UINT32 drvno )
{
	return WriteLongS0( drvno, 0, S0Addr_BEC );
}; //ResetEC

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
es_status_codes SetTORReg( UINT32 drvno, BYTE fkt )
{
	BYTE val = 0; //defaut XCK= high during read
	BYTE read_val = 0;
	if (fkt == 0) val = 0x00; // set to XCK
	if (fkt == 1) val = 0x10; // set to REG -> OutTrig
	if (fkt == 2) val = 0x20; // set to VON
	if (fkt == 3) val = 0x30; // set to DMA_ACT
	if (fkt == 4) val = 0x40; // set to ASLS
	if (fkt == 5) val = 0x50; // set to STIMER
	if (fkt == 6) val = 0x60; // set to BTIMER 
	if (fkt == 7) val = 0x70; // set to ISR_ACT 
	if (fkt == 8) val = 0x80; // set to S1 
	if (fkt == 9) val = 0x90; // set to S2 
	if (fkt == 10) val = 0xa0; // set to BON 
	if (fkt == 11) val = 0xb0; // set to MEASUREON
	if (fkt == 12) val = 0xc0; // set to SDAT
	if (fkt == 13) val = 0xd0; // set to BDAT
	if (fkt == 14) val = 0xe0; // set to SSHUT
	if (fkt == 15) val = 0xf0; // set to BSHUT

	es_status_codes status = ReadByteS0( drvno, &read_val, S0Addr_TOR + 3 );
	if (status != es_no_error) return status;
	read_val &= 0x0f; //dont disturb lower bits
	val |= read_val;
	return WriteByteS0( drvno, val, S0Addr_TOR + 3 );
}//SetTORReg

/**
 * \brief Set/reset bit for PDA sensor timing(set Reg TOR:D25 -> manual).
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param set if set is true (not 0)-> bit is set, reset else
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetISPDA( UINT32 drvno, BOOL set )
{//set bit if PDA sensor - used for EC and IFC
	BYTE val = 0;
	es_status_codes status = ReadByteS0( drvno, &val, S0Addr_TOR + 3 );
	if (status != es_no_error) return status;
	if (set)
	{
		val |= 0x02;
		status = OpenShutter( drvno );
		if (status != es_no_error) return status;
	}
	else val &= 0xfd;
	return WriteByteS0( drvno, val, S0Addr_TOR + 3 );
}//SetISPDA

/**
 * \brief Set/reset bit for FFT sensor timing(set Reg TOR:D24 -> manual).
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param set if set is true (not 0)-> bit is set, reset else
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetISFFT( UINT32 drvno, BOOL set )
{//set bit if FFT sensor - used for vclks and IFC
	BYTE val = 0;
	es_status_codes status = ReadByteS0( drvno, &val, S0Addr_TOR + 3 );
	if (status != es_no_error) return status;
	if (set) val |= 0x01;
	else val &= 0xfe;
	return WriteByteS0( drvno, val, S0Addr_TOR + 3 );
}//SetISFFT

/**
 * \brief Sets PDA sensor timing(set Reg TOR:D25 -> manual) or FFT.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param sensor_type Determines sensor type.
 * 	- 0: PDA (line sensor)
 * 	- 1: FFT (area sensor)
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes SetSensorType( UINT32 drvno, UINT8 sensor_type )
{
	es_status_codes status = es_no_error;
	switch (sensor_type)
	{
	default:
	case 0:
		status = SetISFFT( drvno, FALSE );
		if (status != es_no_error) return status;
		status = SetISPDA( drvno, TRUE );
		break;
	case 1:
		status = SetISFFT( drvno, TRUE );
		if (status != es_no_error) return status;
		status = SetISPDA( drvno, FALSE );
		break;
	}
	return status;
}

/**
 * \brief Reset TOR register. Is used to set the signal of the O-plug of interface board) -> manual.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return none
 */
es_status_codes RsTOREG( UINT32 drvno )
{// reset TOREG
	return WriteByteS0( drvno, 0, S0Addr_TOR + 3 );
}

/**
 * \brief
 * 
 * \param drv PCIe board identifier
 * \return es_status_codes
 *		- es_no_error
 * 		- es_register_read_failed
 *		- es_camera_not_found
 *		- es_register_write_failed
 */
es_status_codes initReadFFLoop( UINT32 drv )
{
	es_status_codes status = es_no_error;
	//WDC_Err("entered DLLReadFFLoop of PCIEcard #%i\n", drv);
	if (!DBGNOCAM)
	{
		//Check if Camera there
		status = FindCam(drv);
		if (status != es_no_error)
		{
			ErrorMsg( "no Camera found" );
			return status;
		}
	}
	//reset the internal block counter and ScanIndex before START
	//set to hw stop of timer hwstop=TRUE
	status = RS_DMAAllCounter( drv, TRUE );
	if (status != es_no_error) return status;
	//reset intr copy buf function
	dmaBufferPartReadPos[drv] = 0;
	userBufferWritePos[drv] = userBuffer[drv]; // reset buffer index to base we got from InitDMA
	WDC_Err( "RESET userBufferWritePos to %x\n", userBufferWritePos[drv] );
	IsrCounter = 0;
	//set MeasureOn Bit
	return setMeasureOn(drv);
}

/*
\brief Wait in loop until block trigger occurs.
If block trigger high: return
If block trigger low: wait for hi
Checks only PCIE board no 1
\param board_sel Selects PCIe board. May only work for 1.
\return
	- 0: abortion
	- 1: block triggered
*/
int waitForBlockTrigger( UINT32 board_sel )
{
	//if lo wait for hi
	BOOL triggerState = FALSE;
	while (!triggerState)
	{
		//only btrig_ch 5 is used
		readBlockTriggerState(1, 5, &triggerState);
		switch (checkForPressedKeys())
		{
			//default and no key: stay in while and wait for block trigger
		default:
		case 0:
			break;
			//ESC: return with 0 for abortion
		case 1:
			return 0;
			//Space: return with 1 for triggered
		case 2:
			return 1;
		}
	}
	//return with 1 for triggered
	return 1;
}

/**
\brief Check escape key and stop measurement if pressed or start block on space
\param board_sel PCIe board selector
\return 
	- 0: no key pressed
	- 1: ESC pressed
	- 2: Space pressed
*/
int checkForPressedKeys( )
{
	if (GetAsyncKeyState( VK_ESCAPE ))
		return 1;
	if (GetAsyncKeyState( VK_SPACE ))
	{ //start if Space was pressed
		while (GetAsyncKeyState( VK_SPACE ) & 0x8000) {}; //wait for release
		return 2;
	}
	return 0;
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
es_status_codes ReadFFLoop( UINT32 board_sel )
{
	es_status_codes status = es_no_error;
	WDC_Err("Start ReadFFLoop with board_sel: %u\n", board_sel);
	if (board_sel == 1 || board_sel == 3)
	{
		status = initReadFFLoop(1);
		if (status != es_no_error) return status;
	}
	if (number_of_boards == 2 && (board_sel == 2 || board_sel == 3))
	{
		status = initReadFFLoop(2);
		if (status != es_no_error) return status;
	}
	//WDC_Err("ReadFFLoop: Block Trigger is set to%d\n", blocktrigger);
	//SetThreadPriority()
	for (UINT32 blk_cnt = 0; blk_cnt < Nob; blk_cnt++)
	{//block read function
		if (!waitForBlockTrigger( board_sel ))
			return abortMeasurement( board_sel );
		WDC_Err("Block triggered\n");
		if (board_sel == 1 || board_sel == 3)
		{
			status = countBlocksByHardware( 1 );
			if (status != es_no_error) return status;
			if (board_sel != 3)
			{
				status = setBlockOn( 1 );
				if (status != es_no_error) return status;
				status = StartSTimer( 1 );
				if (status != es_no_error) return status;
				//start scan for first read
				if(useSWTrig) status = SWTrig( 1 );
				if (status != es_no_error) return status;
			}
		}
		if (number_of_boards == 2 && (board_sel == 2 || board_sel == 3))
		{
			status = countBlocksByHardware( 2 );
			if (status != es_no_error) return status;
			if (board_sel != 3)
			{
				status = setBlockOn( 2 );
				if (status != es_no_error) return status;
				status = StartSTimer( 2 );
				if (status != es_no_error) return status;
				//start scan for first read
				if (useSWTrig) status = SWTrig( 2 );
				if (status != es_no_error) return status;
			}
		}
		//for synchronising both cams
		if (board_sel == 3)
		{					 //start Timer !!!
			//StartFFTimer(1, exptus);
			//StartFFTimer(2, exptus);

			UINT32 data1 = 0;
			UINT32 data2 = 0;

			status = ReadLongS0( 1, &data1, S0Addr_XCKLL ); //reset	
			if (status != es_no_error) return status;
			data1 &= 0xF0000000;
			data1 |= 0x40000000;			//set timer on

			status = ReadLongS0( 2, &data2, S0Addr_XCKLL ); //reset	
			if (status != es_no_error) return status;
			data2 &= 0xF0000000;
			data2 |= 0x40000000;			//set timer on

			UINT32	PortOffset = S0Addr_XCKLL + 0x80;

			//faster
			WDC_WriteAddr32( hDev[1], 0, PortOffset, data1 );
			WDC_WriteAddr32( hDev[2], 0, PortOffset, data2 );
		}
		//WDC_Err("before scan loop start\n");
		//main read loop - wait here until nos is reached or ESC key
		//if nos is reached the flag RegXCKMSB:b30 = TimerOn is reset by hardware if flag HWDREQ_EN is TRUE
		//extended to Timer_routine for all variants of one and  two boards
		if (board_sel == 1)
		{
			BOOL timerOneOn = TRUE;
			while (timerOneOn)
			{
				//stop if ESC was pressed
				if (GetAsyncKeyState( VK_ESCAPE ) | (FindCam(1) != es_no_error) | escape_readffloop)
					return abortMeasurement(1);
				status = IsTimerOn(1, &timerOneOn);
				if (status != es_no_error) return status;
			}
		}
		if (number_of_boards == 2 && board_sel == 2)
		{
			BOOL timerTwoOn = TRUE;
			while (timerTwoOn)
			{
				//stop if ESC was pressed
				if (GetAsyncKeyState( VK_ESCAPE ) | (FindCam(2) != es_no_error) | escape_readffloop)
					return abortMeasurement(2);
				status = IsTimerOn(2, &timerTwoOn);
				if (status != es_no_error) return status;
			}
		}
		if (number_of_boards == 2 && board_sel == 3)
		{
			BOOL timerOneOn = TRUE,
				timerTwoOn = TRUE;

			while (timerOneOn || timerTwoOn )
			{
				BOOL return_flag_1 = FALSE;
				BOOL return_flag_2 = FALSE;

				if (!return_flag_1)
				{
					//stop if ESC was pressed
					if (GetAsyncKeyState( VK_ESCAPE ) | (FindCam(1) != es_no_error) | escape_readffloop)
					{
						status = abortMeasurement( 1 );
						if (status != es_no_error) return status;
						return_flag_1 = TRUE;
					}
				}
				if (!return_flag_2)
				{
					//stop if ESC was pressed
					if (GetAsyncKeyState( VK_ESCAPE ) | (FindCam(2) != es_no_error) | escape_readffloop)
					{
						status = abortMeasurement( 2 );
						if (status != es_no_error) return status;
						return_flag_2 = TRUE;
					}
				}
				if (return_flag_1 && return_flag_2) return status;
				status = IsTimerOn(1, &timerOneOn);
				if (status != es_no_error) return status;
				status = IsTimerOn(2, &timerTwoOn);
				if (status != es_no_error) return status;
			}
		}
		if (board_sel == 1 || board_sel == 3)
		{
			status = resetBlockOn( 1 );
			if (status != es_no_error) return status;
		}
		if (number_of_boards == 2 && (board_sel == 2 || board_sel == 3))
		{
			status = resetBlockOn( 2 );
			if (status != es_no_error) return status;
		}
	}//block cnt read function
	if (board_sel == 1 || board_sel == 3)
	{
		status = StopSTimer( 1 );
		if (status != es_no_error) return status;
	}
	if (number_of_boards == 2 && (board_sel == 2 || board_sel == 3))
	{
		status = StopSTimer( 2 );
		if (status != es_no_error) return status;
	}
	if (board_sel == 1 || board_sel == 3)
	{
		status = GetLastBufPart( 1 );
		if (status != es_no_error) return status;
	}
	if (number_of_boards == 2 && (board_sel == 2 || board_sel == 3))
	{
		status = GetLastBufPart( 2 );
		if (status != es_no_error) return status;
	}
	// This sleep is here to prevent the measurement beeing interrupted too early. When operating with 2 cameras the last scan could be cut off without the sleep. This is only a workaround. The problem is that the software is waiting for RSTIMER beeing reset by the hardware before setting measure on and block on to low, but the last DMA is done after RSTIMER beeing reset. BLOCKON and MEASUREON should be reset after all DMAs are done.
	// RSTIMER --------________
	// DMAWRACT _______-----___
	// BLOCKON ---------_______
	// MEASUREON ---------_____
	WaitforTelapsed(100);
	if (board_sel == 1 || board_sel == 3)
	{
		status = resetMeasureOn(1);
		if (status != es_no_error) return status;
	}
	if (number_of_boards == 2 && (board_sel == 2 || board_sel == 3))
	{
		status = resetMeasureOn(2);
		if (status != es_no_error) return status;
	}
	return status;
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
es_status_codes countBlocksByHardware( UINT32 drvno )
{
	UINT32 val = 0x0;
	//make signal on trig out plug via PCIEFLAGS:D4 - needed to count Blocks
	//set TrigStart flag for TRIGO signal to monitor the signal
	es_status_codes status = ReadLongS0( drvno, &val, S0Addr_PCIEFLAGS );
	if (status != es_no_error) return status;
	val |= PCIEFLAGS_bit_BLOCKTRIG;
	//make pulse for BlockTrigger
	status = WriteLongS0( drvno, val, S0Addr_PCIEFLAGS );
	if (status != es_no_error) return status;
	//set R1(4)
	val &= 0xffffffef;
	//reset signal
	status = WriteLongS0( drvno, val, S0Addr_PCIEFLAGS );
	if (status != es_no_error) return status;
	//reset scan counter for next block - or timer is disabled
	return RS_ScanCounter( drvno );
}

/**
\brief Const burst loop with DMA initiated by hardware DREQ.
Read nos lines from FIFO
*/
unsigned int __stdcall ReadFFLoopThread()//threadex
{
	//local declarations
	SetPriority( READTHREADPriority );  //run ReadFFLoop in higher priority
	escape_readffloop = FALSE;
	IsrCounter = 0;
	Running = TRUE;
	if (CONTFFLOOP) //run continiously
	{
		do
		{
			if (GetAsyncKeyState( VK_ESCAPE ))
			{ //stop if ESC was pressed
				if (BOARD_SEL == 1 || BOARD_SEL == 3)
				{
					StopSTimer( 1 );
					//SetIntFFTrig(drv);//disable ext input
					SetDMAReset( 1 );	//Initiator reset
				}
				if (number_of_boards == 2 && (BOARD_SEL == 2 || BOARD_SEL == 3))
				{
					StopSTimer( 2 );
					//SetIntFFTrig(drv);//disable ext input
					SetDMAReset( 2 );	//Initiator reset
				}
				Running = FALSE;
				return 1;
			}
			ReadFFLoop(BOARD_SEL);
			Sleep( CONTPAUSE); // wait or next block is too early and scrambles last XCK, GST20
			IsrCounter = 0;
		}
		while (1);
	}
	else
	{
		ReadFFLoop(BOARD_SEL);
	}
	Running = FALSE;
	//_endthread();//thread
	WDC_Err("End ReadFFLoopThread\n");
	return 1;//endthreadex is called automatically when this returns
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
es_status_codes readBlockTriggerState( UINT32 drv, UINT8 btrig_ch, BOOL* state)
{
	volatile UCHAR val = 0;
	*state = FALSE;
	es_status_codes status = es_no_error;
	switch (btrig_ch)
	{
	default:
	case 0:
		*state = TRUE;
		break;
	case 1: //I
		status = ReadByteS0( drv, &val, S0Addr_CTRLA );
		if (status != es_no_error) return status;
		if ((val & 0x40) > 0) *state = TRUE;
		break;
	case 2: //S1
		status = ReadByteS0( drv, &val, S0Addr_CTRLC );
		if (status != es_no_error) return status;
		if ((val & 0x02) > 0) *state = TRUE;
		break;
	case 3: //S2
		status = ReadByteS0( drv, &val, S0Addr_CTRLC );
		if (status != es_no_error) return status;
		if ((val & 0x04) > 0) *state = TRUE;
		break;
	case 4: // S1&S2
		status = ReadByteS0( drv, &val, S0Addr_CTRLC );
		if (status != es_no_error) return status;
		if ((val & 0x02) == 0) *state = FALSE;
		status = ReadByteS0( drv, &val, S0Addr_CTRLC );
		if (status != es_no_error) return status;
		if ((val & 0x04) == 0) *state = FALSE;
		*state = TRUE;
		break;
	case 5: // TSTART
		status = ReadByteS0( drv, &val, S0Addr_CTRLA );
		if (status != es_no_error) return status;
		//and TSTART with MeasureOn
//		ReadByteS0( drv, &val2, S0Addr_PCIEFLAGS );
//		if (((val & 0x80) > 0) & ((val2 & PCIEFLAGS_bit_MEASUREON) > 0)) return TRUE;
		if ((val & 0x80) > 0) *state = TRUE;
		break;
	}
	return status;
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
es_status_codes StartSTimer( UINT32 drvno )
{
	UINT32 data = 0;
	es_status_codes status = ReadLongS0( drvno, &data, S0Addr_XCKLL );
	if (status != es_no_error) return status;
	data |= 0x40000000;	//set timer on
	return WriteLongS0( drvno, data, S0Addr_XCKLL );
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
es_status_codes StopSTimer( UINT32 drvno )
{
	WDC_Err("Stop S Timer, drv: %u\n", drvno);
	BYTE data = 0;
	es_status_codes status = ReadByteS0(drvno, &data, S0Addr_XCKMSB);
	if (status != es_no_error) return status;
	data &= 0xBF;
	return WriteByteS0(drvno, data, S0Addr_XCKMSB);
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
es_status_codes SetSTimer( UINT32 drvno, UINT32 stime_in_microseconds )
{
	UINT32 data = 0;
	es_status_codes status = ReadLongS0( drvno, &data, S0Addr_XCKLL ); //reset
	if (status != es_no_error) return status;
	data &= 0xF0000000;
	data |= stime_in_microseconds & 0x0FFFFFFF;
	return WriteLongS0( drvno, data, S0Addr_XCKLL );
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
es_status_codes SetBTimer( UINT32 drvno, UINT32 btime_in_microseconds )
{
	if (btime_in_microseconds)
	{
		UINT32 data = btime_in_microseconds | 0x80000000;
		return WriteLongS0( drvno, data, S0Addr_BTIMER );
	}
	else
		return WriteLongS0( drvno, 0, S0Addr_BTIMER );
}

/**
 * \brief Sets slope for block trigger.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param slope 1 for positive slope
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetBSlope( UINT32 drvno, UINT32 slope )
{
	return WriteLongS0( drvno, slope, S0Addr_BSLOPE );
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
es_status_codes SWTrig( UINT32 drvno )
{
	UCHAR reg = 0;
	//	ReadByteS0(drvno,&reg,11);  //enable timer
	//	reg |= 0x40;  
	//	WriteByteS0(drvno,reg,11);	
	es_status_codes status = ReadByteS0( drvno, &reg, S0Addr_BTRIGREG );
	if (status != es_no_error) return status;
	reg |= 0x40;
	status = WriteByteS0( drvno, reg, S0Addr_BTRIGREG ); //set Trigger
	if (status != es_no_error) return status;
	reg &= 0xBF;
	return WriteByteS0( drvno, reg, S0Addr_BTRIGREG ); //reset
}

/**
 * \brief Checks if timer is active (Bit30 of XCK-Reg).
 * 
 * \param drvno board number (=1 if one PCI board)
 * \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes IsTimerOn( UINT32 drvno, BOOL* on )
{
	BYTE data = 0;
	es_status_codes status = ReadByteS0( drvno, &data, S0Addr_XCKMSB );
	if (status != es_no_error) return status;
	data &= 0x40;
	if (data != 0) *on = TRUE;
	else *on = FALSE;
	return status;
}

/**
 * \brief Checks content of FIFO.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param valid Is true (not 0) if FIFO keeps >= 1 complete lines (linecounter>0).
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 */
es_status_codes checkFifoFlags( UINT32 drvno, BOOL* valid )
{	// not empty & XCK = low -> true
	WDC_Err( "checkFifoFlags\n" );
	BYTE data = 0;
	es_status_codes status = ReadByteS0( drvno, &data, S0Addr_FF_FLAGS );
	data &= 0x80;
	if (data > 0) *valid = TRUE;
	else *valid = FALSE;
	return status;
}

/**
\brief Fifo is full
*/
BOOL FFFull( UINT32 drvno )
{
	WDC_Err( "FFFull\n" );
	BYTE data = 0;
	ReadByteS0( drvno, &data, S0Addr_FF_FLAGS );
	data &= 0x20;
	if (data > 0) return TRUE; //empty
	return FALSE;
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
es_status_codes checkFifoOverflow( UINT32 drvno, BOOL* overflow )
{
	WDC_Err( "checkFifoOverflow\n" );
	BYTE data = 0;
	es_status_codes status = ReadByteS0( drvno, &data, S0Addr_FF_FLAGS );
	data &= 0x08; //0x20; if not saved
	if (data > 0) *overflow = TRUE; //empty
	else *overflow = FALSE;
	return status;
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
es_status_codes RSFifo( UINT32 drvno )
{
	BYTE data = 0;
	es_status_codes status = ReadByteS0( drvno, &data, S0Addr_BTRIGREG );
	if (status != es_no_error) return status;
	data |= 0x80;
	status = WriteByteS0( drvno, data, S0Addr_BTRIGREG );
	if (status != es_no_error) return status;
	data &= 0x7F;
	return WriteByteS0( drvno, data, S0Addr_BTRIGREG );
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
es_status_codes SetupVCLKReg( UINT32 drvno, ULONG lines, UCHAR vfreq )
{
	WDC_Err( "Setup VCLK register. drvno: %u, lines: %u, vfreq: %u\n", drvno, lines, vfreq);
	es_status_codes status = WriteLongS0( drvno, lines * 2, S0Addr_VCLKCTRL );// write no of vclks=2*lines
	if (status != es_no_error) return status;
	return WriteByteS0( drvno, vfreq, S0Addr_VCLKFREQ );//  write v freq
}//SetupVCLKReg

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
es_status_codes SetupVPB( UINT32 drvno, UINT32 range, UINT32 lines, BOOL keep )
{
	WDC_Err( "entered SetupVPB with range: 0x%x , lines: 0x%x and keep: %x\n", range, lines, keep );
	ULONG adr = 0;
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
	//TODO make function write word or split in writebytes
	//WriteWordS0(drvno, lines, adr);// write range
	es_status_codes  status = WriteByteS0( drvno, (BYTE)lines, adr );
	if (status != es_no_error) return status;
	return WriteByteS0( drvno, (BYTE)(lines >> 8), adr + 1 );
}// SetupVPB

// thread priority stuff

/**
\brief Converts threadp value (1..31) to process priority class and thread priority level.
*/
BOOL ThreadToPriClass( ULONG threadp, DWORD *priclass, DWORD *prilevel )
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
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// set thread to new priority level
// keep old level in global OLDPRICLASS and OLDTHREADLEVEL
// threadp 1..31 is split in class and level
BOOL SetPriority( ULONG threadp )
{
	//Thread on
	if (!ThreadToPriClass( threadp, &NEWPRICLASS, &NEWPRILEVEL ))
	{
		ErrorMsg( " threadp off range " );
		return FALSE;
	}

	hPROCESS = GetCurrentProcess();
	OLDPRICLASS = GetPriorityClass( hPROCESS );

	if (!SetPriorityClass( hPROCESS, NEWPRICLASS ))
	{
		ErrorMsg( " No Class set " );
		return FALSE;
	}

	hTHREAD = GetCurrentThread();
	OLDTHREADLEVEL = GetThreadPriority( hTHREAD );

	if (!SetThreadPriority( hTHREAD, NEWPRILEVEL ))
	{
		ErrorMsg( " No Thread set " );
		return FALSE;
	}
	return TRUE;
}//SetPriority

// System Timer in Ticks
/**
\brief Converts Large to Int64.
*/
UINT64 LargeToInt( LARGE_INTEGER li )
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
LONGLONG InitHRCounter()
{
	BOOL ifcounter;
	UINT64 tps = 0;
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 2;

	//tps:: ticks per second = freq
	ifcounter = QueryPerformanceFrequency( &freq );
	tps = LargeToInt( freq ); //ticks per second

	if (tps == 0) ErrorMsg( " System Timer Error " );
	WDC_Err( "TPS: %lld\n", tps );

	return tps;
} // InitHRCounter

/**
* \brief Reads system timer.
* 
* Read 2x ticks and calculate the difference between the calls in microsec with DLLTickstous, init timer by calling DLLInitSysTimer before use.
* \return act ticks
*/
LONGLONG ticksTimestamp()
{
	LARGE_INTEGER PERFORMANCECOUNTERVAL = { 0, 0 };

	QueryPerformanceCounter( &PERFORMANCECOUNTERVAL );
	return PERFORMANCECOUNTERVAL.QuadPart;

}//ticksTimestamp

/**
\brief Calc delay in ticks from us. Init high resolution counter and calcs DELAYTICKS from m_belPars.m_belDelayMsec.
*/
UINT64 ustoTicks( ULONG us )
{
	BOOL ifcounter;
	UINT64 delaytks = 0;
	UINT64 tps = 0; //ticks per second
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 0;

	//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency( &freq );
	tps = LargeToInt( freq ); //ticks per second

	if (tps == 0) return FALSE; // no counter available

	delaytks = us;
	delaytks = delaytks * tps;
	delaytks = delaytks / 1000000;
	return delaytks;
} // ustoTicks

/**
 * \brief Translate ticks to micro seconds.
 * \param tks ticks of system timer
 * \return micro seconds of tks
*/
UINT32 Tickstous( UINT64 tks )
{
	BOOL ifcounter;
	UINT64 delay = 0;
	UINT64 tps = 0; //ticks per second
	LARGE_INTEGER freq;
	freq.LowPart = 0;
	freq.HighPart = 0;

	//get tps: ticks per second
	ifcounter = QueryPerformanceFrequency( &freq );
	tps = LargeToInt( freq ); //ticks per second

	if (tps == 0) return 0; // no counter available

	delay = tks * 1000000;
	delay = delay / tps;
	return (UINT32)delay;
} // Tickstous

/**
\brief Reads EOI Signal = D4 CTRLC.
*/
BOOL TempGood( UINT32 drvno, UINT32 ch )
{
	BYTE CtrlC = 0;
	ReadByteS0( drvno, &CtrlC, 6 );

	if (ch == 1)
	{
		if ((CtrlC & 0x10) == 0x10)
		{
			return FALSE;
		}
		else return TRUE;
	}
	if (ch == 2)
	{
		if ((CtrlC & 0x20) == 0x20)
		{
			return FALSE;
		}
		else return TRUE;
	}
	if (ch == 3)
	{
		if ((CtrlC & 0x40) == 0x40)
		{
			return FALSE;
		}
		else return TRUE;
	}
	if (ch == 4)
	{
		if ((CtrlC & 0x80) == 0x80)
		{
			return FALSE;
		}
		else return TRUE;
	}

	return FALSE;
}//TempGood

/**
 * \brief Set temperature level for cooled cameras.
 * 
 * \param drvno board number (=1 if one PCI board)
 * \param level level 0..7 / 0=off, 7=min -> see cooling manual
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes SetTemp( UINT32 drvno, UINT8 level )
{
	if (level >= 8) level = 0;
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_coolTemp, level );
}

/**
 * \brief RS scan counter. Is read only - but highest bit=reset.
 * 
 * \param drv PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes RS_ScanCounter( UINT32 drv )
{
	UINT32 dwdata = 0x80000000; //set
	es_status_codes status = WriteLongS0( drv, dwdata, S0Addr_ScanIndex );
	if (status != es_no_error) return status;
	dwdata &= 0x7fffffff; //reset
	return WriteLongS0( drv, dwdata, S0Addr_ScanIndex );
}//RS_ScanCounter

/**
 * \brief Iis read only - but highest bit=reset.
 * 
 * \param drv PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
*/
es_status_codes RS_BlockCounter( UINT32 drv )
{
	UINT32 dwdata = 0x80000000; //set
	es_status_codes status = WriteLongS0( drv, dwdata, S0Addr_BLOCKINDEX );
	if (status != es_no_error) return status;
	dwdata &= 0x7fffffff; //reset
	return WriteLongS0( drv, dwdata, S0Addr_BLOCKINDEX );
}//RS_BlockCounter

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
es_status_codes RS_DMAAllCounter( UINT32 drv, BOOL hwstop )
{
	UINT32 dwdata32 = 0;
	BYTE dwdata8 = 0;
	//Problem: erste scan löst INTR aus
	//aber ohne: erste Block ist 1 zu wenig!0, -> in hardware RS to 0x1
	es_status_codes status = ReadLongS0( drv, &dwdata32, S0Addr_DMAsPerIntr );
	if (status != es_no_error) return status;
	dwdata32 |= 0x80000000;
	status = WriteLongS0( drv, dwdata32, S0Addr_DMAsPerIntr );
	if (status != es_no_error) return status;
	dwdata32 &= 0x7fffffff;
	status = WriteLongS0( drv, dwdata32, S0Addr_DMAsPerIntr );
	if (status != es_no_error) return status;
	//reset the internal block counter - is not BLOCKINDEX!
	status = ReadLongS0( drv, &dwdata32, S0Addr_DmaBufSizeInScans );
	if (status != es_no_error) return status;
	dwdata32 |= 0x80000000;
	status = WriteLongS0( drv, dwdata32, S0Addr_DmaBufSizeInScans );
	if (status != es_no_error) return status;
	dwdata32 &= 0x7fffffff;
	status = WriteLongS0( drv, dwdata32, S0Addr_DmaBufSizeInScans );
	if (status != es_no_error) return status;
	//reset the scan counter
	status = RS_ScanCounter( drv );
	if (status != es_no_error) return status;
	status = RS_BlockCounter( drv );
	if (hwstop)
	{
		//set Block end stops timer:
		//when SCANINDEX reaches NOS, the timer is stopped by hardware.
		status = ReadByteS0( drv, &dwdata8, S0Addr_PCIEFLAGS );
		if (status != es_no_error) return status;
		dwdata8 |= PCIEFLAGS_bit_ENRSTIMERHW; //set bit2 for
		status = WriteByteS0( drv, dwdata8, S0Addr_PCIEFLAGS );
	}
	else
	{
		//stop only with write to RS_Timer Reg
		status = ReadByteS0( drv, &dwdata8, S0Addr_PCIEFLAGS );
		if (status != es_no_error) return status;
		dwdata8 &= 0xFB; //bit2
		status = WriteByteS0( drv, dwdata8, S0Addr_PCIEFLAGS );
	}
	return status;
}//RS_DMAAllCounter

/**
 * \brief Test if SFP module is there and fiber is linked up.
 * 
 * \param drv PCIe board identifier.
 * \return es_status_codes:
 * 		- es_no_error
 * 		- es_register_read_failed
 *		- es_camera_not_found
 */
es_status_codes FindCam( UINT32 drv )
{
	UINT32 dwdata = 0;
	es_status_codes status = ReadLongS0( drv, &dwdata, 0x40 );  // read in PCIEFLAGS register
	if (status != es_no_error) return status;
	if ((dwdata & 0x80000000) > 0)
	{ //SFP error
		ErrorMsg( "Fiber or Camera error" );
		return es_camera_not_found;
	}
	/*
	if ((dwdata & 0x40000000) == 0)
	{
		ErrorMsg( "Fiber connection error" );
		return FALSE;
	}
	*/
	return status;
}//FindCam

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
es_status_codes SetADGain( UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8 )
{
	UINT16 data = 0;
	BYTE a, b, c, d, e, f, g, h;

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
}//SetGain

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
es_status_codes SendFLCAM_DAC( UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature )
{
	UINT16	hi_bytes = 0,
		lo_bytes = 0;
	BYTE	maddr_DAC = 0b11,
		hi_byte_addr = 0x01,
		lo_byte_addr = 0x02;

	if (ctrl & 0x10) //4 ctrl bits => only lower 4 bits allowed
	{
		ErrorMsg( "SendFLCAM_DAC: Only values between 0 and 15 are allowed for control bits." );
		return es_parameter_out_of_range;
	}
	if (addr & 0x10) //4 addr bits => only lower 4 bits allowed
	{
		ErrorMsg( "SendFLCAM_DAC: Only values between 0 and 15 are allowed for address bits." );
		return es_parameter_out_of_range;
	}
	if (feature & 0x10) //4 ctrl bits => only lower 4 bits allowed
	{
		ErrorMsg( "SendFLCAM_DAC: Only values between 0 and 15 are allowed for feature bits." );
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
es_status_codes DAC_setOutput( UINT32 drvno, UINT8 channel, UINT16 output )
{
	//ctrl 3: write and update DAC register
	WDC_Err("DAC ch%u = %u", channel, output);
	return SendFLCAM_DAC( drvno, 3, channel, output, 0 );
}

/**
\brief Get the free and installed memory info.
\param pmemory_all how much is installed
\param pmemory_free how much is free
\return none
*/
void FreeMemInfo( UINT64 *pmemory_all, UINT64 *pmemory_free )
{
	// Use to convert bytes to KB
#define DIV 1024
	// Specify the width of the field in which to print the numbers. 
	// The asterisk in the format specifier "%*I64d" takes an integer 
	// argument and uses it to pad and right justify the number.
#define WIDTH 7
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof( statex );
	GlobalMemoryStatusEx( &statex );
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

void GetRmsVal( UINT32 nos, UINT16 *TRMSVals, double *mwf, double *trms )
{
	*trms = 0.0;
	*mwf = 0.0;
	double sumvar = 0.0;

	for (UINT32 i = 0; i < nos; i++)
	{//get mean val
		*mwf += TRMSVals[i];//for C-Noobs: this is the same like *(TRMSVals+1)
	}
	*mwf /= nos;
	for (UINT32 i = 0; i < nos; i++)
	{// get varianz
		*trms = TRMSVals[i];
		*trms = *trms - *mwf;
		*trms *= *trms;
		sumvar += *trms;
	}
	*trms = sumvar / (nos + 1);
	*trms = sqrt( *trms );
	return;
}//GetRmsVal

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
es_status_codes CalcTrms( UINT32 drvno, UINT32 firstSample, UINT32 lastSample, UINT32 TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms )
{
	if (firstSample >= lastSample || lastSample > *Nospb)
	{
		//error: firstSample must be smaller than lastSample
		WDC_Err("Calc Trms failed. lastSample must be greater than firstSample and both in bounderies of nos, drvno: %u, firstSample: %u, lastSample: %u, TRMS_pixel: %u, CAMpos: %u, Nospb: %u\n",drvno, firstSample, lastSample, TRMS_pixel, CAMpos, *Nospb);
		*mwf = -1;
		*trms = -1;
		return es_parameter_out_of_range;
	}
	UINT32 samples = lastSample - firstSample;
	UINT16 *TRMS_pixels = calloc(samples, sizeof(UINT16));
	if (!TRMS_pixels) return es_allocating_memory_failed;
	//storing the values of one pix for the rms analysis
	for (int scan = 0; scan < samples; scan++)
	{
		UINT64 TRMSpix_of_current_scan = 0;
		GetIndexOfPixel( drvno, TRMS_pixel, scan+firstSample, 0, CAMpos, &TRMSpix_of_current_scan);
		TRMS_pixels[scan] = userBuffer[drvno][TRMSpix_of_current_scan];
	}
	//rms analysis
	GetRmsVal( samples, TRMS_pixels, mwf, trms );
	free(TRMS_pixels);
	return es_no_error;
}//CalcTrms

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
es_status_codes GetIndexOfPixel( UINT32 drvno, UINT16 pixel, UINT32 sample, UINT32 block, UINT16 CAM, UINT64* pIndex )
{
	if (pixel >= aPIXEL[drvno] || sample >= *Nospb || block >= Nob || CAM >= aCAMCNT[drvno])
		return es_parameter_out_of_range;
	//init index with base position of pixel
	UINT64 index = pixel;
	//position of index at CAM position
	index += (UINT64)CAM *((UINT64)aPIXEL[drvno] + 4);  //GS! offset of 4 pixel via pipelining from CAM1 to CAM2
	//position of index at sample
	index += (UINT64)sample * (UINT64)aCAMCNT[drvno] * (UINT64)aPIXEL[drvno];
	//position of index at block
	index += (UINT64)block * (UINT64)(*Nospb) * (UINT64)aCAMCNT[drvno] * (UINT64)aPIXEL[drvno];
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
es_status_codes GetAddressOfPixel( UINT32 drvno, UINT16 pixel, UINT32 sample, UINT32 block, UINT16 CAM, UINT16** address )
{
	UINT64 index = 0;
	es_status_codes status = GetIndexOfPixel(drvno, pixel, sample, block, CAM, &index);
	if (status != es_no_error) return status;
	*address = &userBuffer[drvno][index];
	return status;
}

/**
\brief This functions returns after a time given in microseconds. The time is measured in CPU ticks. The function is escable by pressing ESC.
\param musec Time to wait in microseconds.
\return 1 when success, 0 when aborted by ESC or failure
*/
UINT8 WaitforTelapsed( LONGLONG musec )
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
		if (GetAsyncKeyState( VK_ESCAPE ) | (FindCam(1) != es_no_error) | escape_readffloop) return 0; // check for kill ?
	}
	//WDC_Err("Endzeit:  %lld\n", ticksTimestamp());
	return 1;
}//WaitforTelapsed


/**
 * \brief General init routine for all Camera Systems.
 * 
 * 	Sets register in camera.
 * \param drvno selects PCIe board
 * \param pixel pixel count of camera
 * \param trigger_input for Camera Control (CC): selects CC trigger input. 0 - XCK, 1 - EXTTRIG connector, 2 - DAT
 * \param IS_FFT =1 vclk on, =0 vclk off
 * \param IS_AREA =1 area mode on, =0 area mode off
 * \param IS_COOLED =1 disables PCIe FIFO when cool cam transmits cool status
 * \param led_on 1 led on, 0 led off
 * \param gain_high 1 gain on, 0 gain off
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes InitCameraGeneral( UINT32 drvno, UINT16 pixel, UINT16 cc_trigger_input, UINT8 IS_FFT, UINT8 IS_AREA, UINT8 IS_COOLED )
{
	es_status_codes status = es_no_error;
	// when TRUE: disables PCIe FIFO when cool cam transmits cool status
	if (IS_COOLED)
		status = Use_ENFFW_protection( drvno, TRUE );
	else
		status = Use_ENFFW_protection( drvno, FALSE );
	if (status != es_no_error) return status;
	//set camera pixel register
	status = SendFLCAM( drvno, maddr_cam, cam_adaddr_pixel, pixel );
	if (status != es_no_error) return status;
	//set trigger input
	status = SendFLCAM( drvno, maddr_cam, cam_adaddr_trig_in, cc_trigger_input );
	if (status != es_no_error) return status;
	//select vclk and Area mode on
	//TODO: left shit 15 is greater than type UINT8 -> undefined behaviour
	IS_AREA <<= 15;
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_vclk, (UINT16) (IS_FFT | IS_AREA) );
}

/**
 * \brief Init routine for Camera System 3001.
 * 
 * 	Sets register in camera.
 * \param drvno selects PCIe board
 * \param pixel pixel count of camera
 * \param trigger_input for CC: selects trigger input. 0 - XCK, 1 - EXTTRIG, 2 - DAT
 * \param IS_FFT =1 vclk on, =0 vclk off
 * \param IS_AREA =1 area mode on, =0 area mode off
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes InitCamera3001( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT16 IS_FFT, UINT16 IS_AREA )
{
	es_status_codes status = Use_ENFFW_protection( drvno, TRUE );
	if (status != es_no_error) return status;
	//set camera pixel register
	status = SendFLCAM( drvno, maddr_cam, cam_adaddr_pixel, pixel );
	if (status != es_no_error) return status;
	//set trigger input
	status = SendFLCAM( drvno, maddr_cam, cam_adaddr_trig_in, trigger_input );
	if (status != es_no_error) return status;
	//select vclk and Area mode on
	IS_AREA <<= 15;
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_vclk, IS_FFT | IS_AREA );
}

/**
 * \brief Init routine for Camera System 3010.
 * 
 * 	Sets registers in camera and ADC LTC2271.
 * 	FL3010 is intended for sensor S12198 !
 * 	with frame rate 8kHz = min. 125µs exp time
 * \param drvno selects PCIe board
 * \param pixel pixel amount of camera
 * \param trigger_input selects trigger input. 0 - XCK, 1 - EXTTRIG, 2 - DAT
 * \param adc_mode 0: normal mode, 2: custom pattern
 * \param custom_pattern fixed output for testmode, ignored when testmode FALSE
 * \param led_on 1 led on, 0 led off
 * \param gain_high 1 gain on, 0 gain off
 * \return void
 */
es_status_codes InitCamera3010( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern )
{
	//Use_ENFFW_protection( drvno, FALSE );
	es_status_codes status = Cam3010_ADC_reset( drvno );
	if (status != es_no_error) return status;
	return Cam3010_ADC_setMode( drvno, adc_mode, custom_pattern );
	//set camera pixel register
	//SendFLCAM( drvno, maddr_cam, cam_adaddr_pixel, pixel );
	//set gain and led
	//SendFLCAM( drvno, maddr_cam, cam_adaddr_gain, led_on << 4 & gain_high );
	//set trigger input
	//SendFLCAM( drvno, maddr_cam, cam_adaddr_trig_in, trigger_input );
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
es_status_codes Cam3010_ADC_reset( UINT32 drvno )
{
	return SendFLCAM( drvno, maddr_adc, adc_ltc2271_regaddr_reset, adc_ltc2271_msg_reset );
}

/**
 * \brief ADC mode set (normal or pattern) for Camera System 3010.
 * 
 * 	Lets ADC send sample data or a custom pattern (value).
 * 	Output Mode Register A2, address 02h:
 * 	LVDS output current bits (D7:D5) = 000, (3.5 mA)
 * 	Internal terminator bit D4 = 0, (off)
 * 	Output enable bit D3 = 0, (enabled)
 * 	Output test pattern bit D2 = 1, (on/off)
 * 	Number of output lanes bits (D1:D0) = 01, (4 lanes)
 * 	Address = 02h;
 * 	Data = 1h: ADC sends sample data
 * 	Data = 5h: ADC sends test pattern (a contant defined
 * 	in frames 3 and 4)
 * 	Called by InitCamera3010.
 * \param drvno selects PCIe board
 * \param adc_mode 0: normal, 2: custom pattern
 * \param custom_pattern (only used when adc_mode = 2)
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 */
es_status_codes Cam3010_ADC_setMode( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern )
{
	es_status_codes status = es_no_error;
	switch (adc_mode)
	{
	case 2:
		status = SendFLCAM( drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_custompattern );
		if (status != es_no_error) return status;
		//Test pattern MSB regsiter A3 (TP15:TP8) Address = 03h, Data = custom (8 bit)
		status = SendFLCAM( drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_msb, custom_pattern >> 8 );
		if (status != es_no_error) return status;
		//Test pattern LSB regsiter A4 (TP7:TP0)	Address = 04h, Data = custom (8 bit)
		status = SendFLCAM( drvno, maddr_adc, adc_ltc2271_regaddr_custompattern_lsb, custom_pattern & 0x00FF );
		if (status != es_no_error) return status;
		break;
	case 0:
	default:
		status = SendFLCAM( drvno, maddr_adc, adc_ltc2271_regaddr_outmode, adc_ltc2271_msg_normal_mode );
		if (status != es_no_error) return status;
		break;
	}
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
es_status_codes InitCamera3030( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain )
{
	//Use_ENFFW_protection( drvno, FALSE );
	es_status_codes status = Cam3030_ADC_reset( drvno );
	if (status != es_no_error) return status;
	status = Cam3030_ADC_twoWireModeEN( drvno ); //two wire mode output interface for pal versions P209_2 and above
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
es_status_codes Cam3030_ADC_reset( UINT32 drvno )
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
es_status_codes Cam3030_ADC_twoWireModeEN( UINT32 drvno )
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
es_status_codes Cam3030_ADC_SetGain( UINT32 drvno, UINT8 gain )
{
	return SetADGain( drvno, 1, gain, gain, gain, gain, gain, gain, gain, gain );
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
es_status_codes Cam3030_ADC_RampOrPattern( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern )
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
es_status_codes Use_ENFFW_protection( UINT32 drvno, BOOL USE_ENFFW_PROTECT )
{
	if (USE_ENFFW_PROTECT)
		return SetS0Bit( 3, S0Addr_PCIEFLAGS, drvno );
	else
		return ResetS0Bit( 3, S0Addr_PCIEFLAGS, drvno );
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
es_status_codes SetGPXCtrl( UINT32 drvno, UINT8 GPXAddress, UINT32 GPXData )
{
	UINT32 regData = 0,
		tempData = 0;
	//Read old data of ctrl gpx reg
	es_status_codes status = ReadLongS0(drvno, &regData, S0Addr_TDCCtrl);
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
	status = WriteLongS0(drvno, regData, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return WriteLongS0( drvno, GPXData, S0Addr_TDCData );
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
es_status_codes ReadGPXCtrl( UINT32 drvno, UINT8 GPXAddress, UINT32* GPXData )
{
	UINT32 regData = 0,
		tempData = 0;
	//Read old data of ctrl gpx reg
	es_status_codes status = ReadLongS0( drvno, &regData, S0Addr_TDCCtrl );
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
	status = WriteLongS0(drvno, regData, S0Addr_TDCCtrl);
	if (status != es_no_error) return status;
	return ReadLongS0( drvno, GPXData, S0Addr_TDCData );
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
es_status_codes InitGPX( UINT32 drvno, UINT32 delay )
{
	UINT32 regData, err_cnt = 0;
	UINT32 mask = 0x3FFFF;
	delay &= mask;
	UINT32 regVal = 0x08200000 | delay;
	UINT32 RegData[12][2] = {
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
	es_status_codes status = ReadLongS0( drvno, &regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	regData |= 0x01;
	status = WriteLongS0( drvno, regData, S0Addr_TDCCtrl );
	if (status != es_no_error) return status;
	regData &= 0xFFFFFFFE;
	status = WriteLongS0( drvno, regData, S0Addr_TDCCtrl ); //reset bit
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
 * \brief Reads registers 0 to 12 of TDC-GPX chip. Time delay counter option.
 * 
 * \param drvno PCIe board identifier
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
es_status_codes AboutGPX( UINT32 drvno )
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

	j = sprintf( fn, "GPX- registers   \n" );
	es_status_codes status = es_no_error;
	for (i = 0; i < 8; i++)
	{
		status = ReadGPXCtrl( drvno, i, &regData );
		if (status != es_no_error) return status;
		j += sprintf( fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], regData );
	}

	for (i = 11; i < 13; i++)
	{
		status = ReadGPXCtrl( drvno, i, &regData );
		if (status != es_no_error) return status;
		j += sprintf( fn + j, "%s \t: 0x%I32x\n", LUTS0Reg[i], regData );
	}
	MessageBox( hWnd, fn, "GPX regs", MB_OK );
	j = sprintf( fn, "delay- registers   \n" );
	i = 0;
	abbr = FALSE;
	empty = FALSE;
	while (!abbr)
	{
		//master reset - 
//		SetGPXCtrl(drvno, 4, 0); // write to reg4
//		WriteLongS0(drvno, 0x02800000, 0x5C);
		status = WaitTrigger( 1, FALSE, &space, &abbr );
		if (status != es_no_error) return status;
		irf = FALSE;
		//		while (!irf & !abbr)
		{	//wait for IR
//			ReadLongS0(drvno, &regData, 0x58);
//			if ((regData & 0x02) > 0) { irf = TRUE; }; //02=IR , 04=LF, 08=empty
//			WaitTrigger(1, FALSE, &space, &abbr);
		}
		i = 0;
		j = sprintf( fn, "read- regs   \n" );
		//		while (!abbr & i < 1)
		{	//read 2 vals
			i += 1;
			//		ReadLongS0(drvno, &regData, 0x58);
			//		if (regData &= 0x08 != 0) { empty = TRUE; };
			status = ReadGPXCtrl( drvno, 8, &regData ); //lege addr 8 an bus !!!!
			if (status != es_no_error) return status;
			j += sprintf( fn + j, "%d \t: 0x%I32x\n", i, regData );

			i += 1;
			//		ReadLongS0(drvno, &regData, 0x58);
			//		if (regData &= 0x08 != 0) { empty = TRUE; };
			status = ReadGPXCtrl( drvno, 9, &regData ); //lege addr 9 an bus !!!!
			if (status != es_no_error) return status;
			j += sprintf( fn + j, "%d \t: 0x%I32x\n", i, regData );
		}
		MessageBox( hWnd, fn, "GPX regs", MB_OK );
	}
	status = ReadGPXCtrl( drvno, 11, &regData );
	if (status != es_no_error) return status;
	j += sprintf( fn + j, "%s \t: 0x%I32x\n", " stop hits", regData );
	status = ReadGPXCtrl( drvno, 12, &regData );
	if (status != es_no_error) return status;
	j += sprintf( fn + j, "%s \t: 0x%I32x\n", " flags", regData );
	MessageBox( hWnd, fn, "GPX regs", MB_OK );
	return ReadGPXCtrl( drvno, 8, &regData ); //read access follows                 set addr 8 to bus !!!!
	//master reset
//	SetGPXCtrl(drvno, 4, 0); // write to reg4
//	WriteLongS0(drvno, 0x06400300, 0x5C);
}

/**
 * \brief Calculate needed RAM in MB for given nos and nob.
 * 
 * \param nos number of samples
 * \param nob number of blocks
 * \return RAM in MB
 */
double CalcRamUsageInMB( UINT32 nos, UINT32 nob )
{
	double ramUsage = 0;
	for (int i = 0; i < number_of_boards; i++)
		ramUsage += (UINT64)nos * (UINT64)nob * (UINT64)aPIXEL[i + 1] * (UINT64)aCAMCNT[i + 1] * sizeof( UINT16 );
	ramUsage = ramUsage / 1048576;
	WDC_Err( "ram usage: %f", ramUsage );
	return ramUsage;
}

/**
 * \brief Calculate the theoretical time needed for one measurement.
 * 
 * \param nos number of samples
 * \param nob number of blocks
 * \param exposure_time_in_ms exposure time in ms
 * \return time in seconds
 */
double CalcMeasureTimeInSeconds( UINT32 nos, UINT32 nob, double exposure_time_in_ms )
{
	double measureTime = (double)nos * (double)nob * exposure_time_in_ms / 1000;
	return measureTime;
}


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
es_status_codes SetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq )
{
	es_status_codes status = SetupVCLKReg( drvno, lines, vfreq );
	if (status != es_no_error) return status;
	return ResetPartialBinning( drvno );
}

/**
\brief Turn partial binning on or off.
\param drvno PCIe board identifier.
\param number_of_regions =0 to turn partial binning off. !=0 to turn on.
\return True for success.
*/
BOOL SetPartialBinning( UINT32 drvno, UINT16 number_of_regions )
{
	BOOL success = WriteLongS0( drvno, number_of_regions, S0Addr_ARREG );
	success &= SetS0Bit( 15, S0Addr_ARREG, drvno );//this turns ARREG on and therefore partial binning too
	return success;
}

/**
 * \brief
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes
 *		- es_no_error
 */
es_status_codes ResetPartialBinning( UINT32 drvno )
{
	return ResetS0Bit( 15, S0Addr_ARREG, drvno );//this turns ARREG off and therefore partial binning too
}

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
	DLLInitGlobals( g );
	return;
}

/**
\brief Checks if the dam routine was already called.
\param drvno PCIe board identifier.
\return True when DMA is set.
*/
BOOL isDmaSet( UINT32 drvno )
{
	return WDC_IntIsEnabled( hDev[drvno] );
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
es_status_codes isMeasureOn( UINT32 drvno, BOOL* measureOn )
{
	UINT32 data = 0;
	es_status_codes status = ReadLongS0( drvno, &data, S0Addr_PCIEFLAGS );
	if (status != es_no_error) return status;
	//Check for measure on bit
	if (PCIEFLAGS_bit_MEASUREON & data)
		*measureOn = TRUE;
	else
		*measureOn = FALSE;
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
es_status_codes isBlockOn( UINT32 drvno, BOOL* blockOn )
{
	UINT32 data = 0;
	es_status_codes status = ReadLongS0( drvno, &data, S0Addr_PCIEFLAGS );
	if (status != es_no_error) return status;
	//Check for measure on bit
	if (PCIEFLAGS_bit_BLOCKON & data)
		*blockOn = TRUE;
	else
		*blockOn = FALSE;
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
es_status_codes waitForMeasureReady( UINT32 drvno )
{
	BOOL measureOn = TRUE;
	es_status_codes status = es_no_error;
	while (measureOn)
	{
		status = isMeasureOn(drvno, &measureOn);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Returns when block on bit is 0.
 * 
 * \param drvno PCIe board identifier.
 *  \return es_status_codes:
 *		- es_no_error
 * 		- es_register_read_failed
 */
es_status_codes waitForBlockReady( UINT32 drvno )
{
	BOOL blockOn = TRUE;
	es_status_codes status = es_no_error;
	while (blockOn)
	{
		status = isBlockOn(drvno, &blockOn);
		if (status != es_no_error) return status;
	}
	return status;
}

/**
 * \brief Sets BlockOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes setBlockOn( UINT32 drvno )
{
	notifyBlockStart( drvno );
	return SetS0Bit( PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS, drvno );
}

/**
 * \brief Resets BlockOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes resetBlockOn( UINT32 drvno )
{
	notifyBlockDone( drvno );
	return ResetS0Bit( PCIEFLAGS_bitindex_BLOCKON, S0Addr_PCIEFLAGS, drvno );
}

/**
 * \brief Sets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes setMeasureOn( UINT32 drvno )
{
	notifyMeasureStart( drvno );
	return SetS0Bit( PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS, drvno );
}

/**
 * \brief Resets setMeasureOn bit in PCIEFLAGS and notifies UI about it.
 * 
 * \param drvno PCIe board identifier.
 * \return es_status_codes:
 *		- es_no_error
 */
es_status_codes resetMeasureOn( UINT32 drvno )
{
	notifyMeasureDone( drvno );
	return ResetS0Bit( PCIEFLAGS_bitindex_MEASUREON, S0Addr_PCIEFLAGS, drvno );
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
es_status_codes SetBTI( UINT32 drvno, UINT8 bti_mode )
{
	return SetS0Reg( bti_mode << CTRLB_bitindex_BTI0, CTRLB_bit_BTI0 | CTRLB_bit_BTI1 | CTRLB_bit_BTI2, S0Addr_CTRLB, drvno );
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
es_status_codes SetSTI( UINT32 drvno, UINT8 sti_mode )
{
	return SetS0Reg( sti_mode, CTRLB_bit_STI0 | CTRLB_bit_STI1 | CTRLB_bit_STI2, S0Addr_CTRLB, drvno );
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
es_status_codes SetMeasurementParameters( UINT32 drvno, UINT32 nos, UINT32 nob )
{
	Nob = nob;
	*Nospb = nos;
	WDC_Err( "entered SetMeasurementParameters with drv: %i nos: %i and nob: %i and camcnt: %i\n", drvno, nos, nob, aCAMCNT[drvno] );
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
	ULONG dmaBufferPartSizeInScans = DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS; //500
	if (BOARD_SEL > 2)
		numberOfInterrupts = Nob * (*Nospb) * aCAMCNT[drvno] * number_of_boards / dmaBufferPartSizeInScans - 2;//- 2 because intr counter starts with 0
	else
		numberOfInterrupts = Nob * (*Nospb) * aCAMCNT[drvno] / dmaBufferPartSizeInScans - 1;//- 1 because intr counter starts with 0
	WDC_Err("numberOfInterrupts: 0x%x \n", numberOfInterrupts);
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
es_status_codes SetGain( UINT32 drvno, UINT16 gain_value )
{
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_gain, gain_value );
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
es_status_codes LedOff( UINT32 drvno, UINT8 LED_OFF )
{
	return SendFLCAM( drvno, maddr_cam, cam_adaddr_LEDoff, (UINT16) LED_OFF );
}

/**
 * \brief Copies one frame of pixel data to pdest.
 * 
 * \param drv indentifier of PCIe card
 * \param curr_nos position in samples (0...(nos-1))
 * \param curr_nob position in blocks (0...(nob-1))
 * \param curr_cam position in camera count (0...(CAMCNT-1))
 * \param pdest address where data is written, should be buffer with size: length in pixel * sizeof( UINT16 )
 * \param length lenght of frame in pixel, typically pixel count (1088)
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
es_status_codes ReturnFrame( UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16 *pdest, UINT32 length )
{
	UINT16* pframe = NULL;
	//UINT16** pframe;// = &userBuffer[1][0];
	es_status_codes status = GetAddressOfPixel( drv, 0, curr_nos, curr_nob, curr_cam, &pframe);
	if (status != es_no_error) return status;
	memcpy( pdest, pframe, length * sizeof( UINT16 ) );  // length in bytes
	/*
	WDC_Err( "RETURN FRAME: drvno: %u, curr_nos: %u, curr_nob: %u, curr_cam: %u, _PIXEL: %u, length: %u\n", drvno, curr_nos, curr_nob, curr_cam, _PIXEL, length );
	WDC_Err("FRAME2: address Buff: 0x%x \n", userBuffer[drvno]);
	WDC_Err("FRAME2: address pdio: 0x%x \n", pdioden);
	WDC_Err("FRAME3: pix42 of ReturnFrame: %d \n", *((USHORT*)pdioden + 420));
	WDC_Err("FRAME3: pix43 of ReturnFrame: %d \n", *((USHORT*)pdioden + 422));
	*/
	return status;
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
es_status_codes abortMeasurement( UINT32 drv )
{
	WDC_Err("Abort Measurement\n");
	es_status_codes status = StopSTimer( drv );
	if (status != es_no_error) return status;
	status = resetBlockOn(drv);
	if (status != es_no_error) return status;
	status = resetMeasureOn(drv);
	if (status != es_no_error) return status;
	return SetDMAReset( drv );
}
