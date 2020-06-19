/*
This file is part of ESLSCDLL.

ESLSCDLL is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ESLSCDLL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see < http://www.gnu.org/licenses/>.

Copyright 2020 Entwicklungsbuero G. Stresing (http://www.stresing.de/)
*/

#include "ESLSCDLL.h"

volatile struct ffloopparams params, params2;

/**
\brief DllMain entry point

An optional entry point into a dynamic-link library (DLL). When the system starts or terminates a process or thread, it calls the entry-point function for each loaded DLL using the first thread of the process. The system also calls the entry-point function for a DLL when it is loaded or unloaded using the LoadLibrary and FreeLibrary functions. More information: https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain
\param[in] hinstDLL A handle to the DLL module. The value is the base address of the DLL. The HINSTANCE of a DLL is the same as the HMODULE of the DLL, so hinstDLL can be used in calls to functions that require a module handle.
\param[in] fdwReason	The reason code that indicates why the DLL entry-point function is being called. This parameter can be one of the following values:
	- DLL_PROCESS_ATTACH 1: The DLL is being loaded into the virtual address space of the current process as a result of the process starting up or as a result of a call to LoadLibrary. DLLs can use this opportunity to initialize any instance data or to use the TlsAlloc function to allocate a thread local storage (TLS) index. The lpReserved parameter indicates whether the DLL is being loaded statically or dynamically.
	- DLL_PROCESS_DETACH 0: The DLL is being unloaded from the virtual address space of the calling process because it was loaded unsuccessfully or the reference count has reached zero (the processes has either terminated or called FreeLibrary one time for each time it called LoadLibrary). The lpReserved parameter indicates whether the DLL is being unloaded as a result of a FreeLibrary call, a failure to load, or process termination. The DLL can use this opportunity to call the TlsFree function to free any TLS indices allocated by using TlsAlloc and to free any thread local data. Note that the thread that receives the DLL_PROCESS_DETACH notification is not necessarily the same thread that received the DLL_PROCESS_ATTACH notification.
	- DLL_THREAD_ATTACH 2: The current process is creating a new thread. When this occurs, the system calls the entry-point function of all DLLs currently attached to the process. The call is made in the context of the new thread. DLLs can use this opportunity to initialize a TLS slot for the thread. A thread calling the DLL entry-point function with DLL_PROCESS_ATTACH does not call the DLL entry-point function with DLL_THREAD_ATTACH. Note that a DLL's entry-point function is called with this value only by threads created after the DLL is loaded by the process. When a DLL is loaded using LoadLibrary, existing threads do not call the entry-point function of the newly loaded DLL.
	- DLL_THREAD_DETACH 3: A thread is exiting cleanly. If the DLL has stored a pointer to allocated memory in a TLS slot, it should use this opportunity to free the memory. The system calls the entry-point function of all currently loaded DLLs with this value. The call is made in the context of the exiting thread.
\param[in] lpvReserved If fdwReason is DLL_PROCESS_ATTACH, lpvReserved is NULL for dynamic loads and non-NULL for static loads. If fdwReason is DLL_PROCESS_DETACH, lpvReserved is NULL if FreeLibrary has been called or the DLL load failed and non-NULL if the process is terminating.
\return When the system calls the DllMain function with the DLL_PROCESS_ATTACH value, the function returns TRUE if it succeeds or FALSE if initialization fails. If the return value is FALSE when DllMain is called because the process uses the LoadLibrary function, LoadLibrary returns NULL. (The system immediately calls your entry-point function with DLL_PROCESS_DETACH and unloads the DLL.) If the return value is FALSE when DllMain is called during process initialization, the process terminates with an error. To get extended error information, call GetLastError. When the system calls the DllMain function with any value other than DLL_PROCESS_ATTACH, the return value is ignored.
*/
BOOL WINAPI DLLMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// DLL initialization code goes here. Formerly this 
		// would be in the LibMain function of a 16-bit DLL.
		//..................................................
		nProcessCount++;
		return(TRUE);
	case DLL_PROCESS_DETACH:
		// DLL cleanup code goes here. Formerly this would
		// be in the WEP function of a 16-bit DLL.
		//................................................
	 //	CCDDrvExit(DRV);
		nProcessCount--;
		break;
	case DLL_THREAD_ATTACH:
		// Special initialization code for new threads goes here.
		// This is so the DLL can "Thread Protect" itself.
		//.......................................................
		nThreadCount++;
		break;
	case DLL_THREAD_DETACH:
		// Special cleanup code for threads goes here.
		//............................................
		nThreadCount--;
		break;
	}
	return(FALSE);
}

/**
\brief Function for multithreading.
*/
DllAccess int DLLGetProcessCount()
{
	return(nProcessCount);
}

/**
\brief Function for multithreading.
*/
DllAccess int DLLGetThreadCount()
{
	return(nThreadCount);
}

/**
\copydoc ErrMsgBoxOn
*/
DllAccess  void DLLErrMsgBoxOn( void )
{
	ErrMsgBoxOn();
	return;
}

/**
\copydoc ErrMsgBoxOff
*/
DllAccess  void DLLErrMsgBoxOff( void )
{
	ErrMsgBoxOff();
	return;
}

/**
\brief Initialize the driver. Must be called before any other function.
	Is called automatically for 2 boards.
\return Is true (not 0) if driver was found.
*/
DllAccess UINT8 nDLLCCDDrvInit( void )
{
	newDLL = 1;
	if (CCDDrvInit())
	{
		WDC_Err( "finished DRVInit back in DLL\n" );
		return number_of_boards;
	}
	return 0;
}

/**
\copydoc CCDDrvExit
*/
DllAccess void DLLCCDDrvExit( UINT32 drvno )
{
	CCDDrvExit( drvno );
	return;
}

/**
\brief Initialize the PCIe board. Must be called once at the start.
	Is called automatically for 2 boards.
\param drvno board number (=1 if one PCI board)
\param camcnt amount of cameras
\param pixel number of all pixel (active + dummy pixel)
\param flag816 =1 if AD resolution 12 to 16 bit, =2 if 8bit
\param pclk =0 pixelclock, not used here
\param xckdelay =3, depends on sensor, sets a delay after xck goes high, =7 for Sony sensors
\return true if success
*/
DllAccess UINT8 n2DLLInitBoard( UINT32 drv, UINT32 camcnt, UINT32 pixel, UINT32 pclk, UINT32 xckdelay )
{								
 //if (!InitBoard(drvno)) return 0; //must be called once before any other
 //if FIFO pclk=waits : read frequency; waits is set = 0 for max. FIFO read frequency
 // NO FIFO version: pclk not used.
	InitBoard( drv );
	if (!SetBoardVars( drv, camcnt, pixel, xckdelay )) return 0; //sets data for transfer
	aPIXEL[drv] = pixel; // set globals
	ADRDELAY = xckdelay;
	// AboutS0(drvno);
	return 1; // no error
}

/**
\copydoc ReadByteS0
*/
DllAccess UINT8 DLLReadByteS0( UINT32 drvno, UINT8 *data, UINT32 PortOff )
{
	if (!ReadByteS0( drvno, data, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc WriteByteS0
*/
DllAccess UINT8 DLLWriteByteS0( UINT32 drv, UINT8 DataByte, UINT32 PortOff )
{
	if (!WriteByteS0( drv, DataByte, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc ReadLongS0
*/
DllAccess UINT8 DLLReadLongS0( UINT32 drvno, UINT32 * DWData, UINT32 PortOff )
{
	if (!ReadLongS0( drvno, DWData, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc WriteLongS0
*/
DllAccess UINT8 DLLWriteLongS0( UINT32 drvno, UINT32 DWData, UINT32 PortOff )
{
	if (!WriteLongS0( drvno, DWData, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc ReadLongDMA
*/
DllAccess UINT8 DLLReadLongDMA( UINT32 drvno, UINT32* pDWData, UINT32 PortOff )
{
	if (!ReadLongDMA( drvno, pDWData, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc WriteLongDMA
*/
DllAccess UINT8 DLLWriteLongDMA( UINT32 drvno, UINT32 DWData, UINT32 PortOff ) 
{
	if (!WriteLongDMA( drvno, DWData, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc ReadLongIOPort
*/
DllAccess UINT8 DLLReadLongIOPort( UINT32 drvno, UINT32 * DWData, UINT32 PortOff )
{
	if (!ReadLongIOPort( drvno, DWData, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc WriteLongIOPort
*/
DllAccess UINT8 DLLWriteLongIOPort( UINT32 drvno, UINT32 DataL, UINT32 PortOff )
{
	if (!WriteLongIOPort( drvno, DataL, PortOff )) { return 0; }
	return 1;
}

/**
\copydoc AboutDrv
*/
DllAccess void DLLAboutDrv( UINT32 drvno )
{
	AboutDrv( drvno );
	if (number_of_boards == 2) AboutDrv( 2 );
	return;
}

/**
\copydoc CalcRamUsageInMB
*/
DllAccess double DLLCalcRamUsageInMB( UINT32 nos, UINT32 nob )
{
	return CalcRamUsageInMB( nos, nob );
}

/**
\copydoc CalcMeasureTimeInSeconds
*/
DllAccess double DLLCalcMeasureTimeInSeconds( UINT32 nos, UINT32 nob, double exposure_time_in_ms )
{
	return CalcMeasureTimeInSeconds( nos, nob, exposure_time_in_ms );
}

/**
\copydoc HighSlope
*/
DllAccess void DLLHighSlope( UINT32 drvno )
{
	HighSlope( drvno );
	return;
}

/**
\copydoc LowSlope
*/
DllAccess void DLLLowSlope( UINT32 drvno )
{
	LowSlope( drvno );
	return;
}

/**
\copydoc BothSlope
*/
DllAccess void DLLBothSlope( UINT32 drvno )
{
	BothSlope( drvno );
	return;
}

/**
\copydoc OutTrigHigh
*/
DllAccess void DLLOutTrigHigh( UINT32 drvno )
{
	OutTrigHigh( drvno );
	return;
}

/**
\copydoc OutTrigLow
*/
DllAccess void DLLOutTrigLow( UINT32 drvno )
{
	OutTrigLow( drvno );
	return;
}

/**
\copydoc OutTrigPulse
*/
DllAccess void DLLOutTrigPulse( UINT32 drvno, UINT32 PulseWidth )
{
	OutTrigPulse( drvno, PulseWidth );
	return;
}

/**
\copydoc OpenShutter
*/
DllAccess void DLLOpenShutter( UINT32 drvno )
{
	OpenShutter( drvno );
	return;
}

/**
\copydoc CloseShutter
*/
DllAccess void DLLCloseShutter( UINT32 drvno )
{
	CloseShutter( drvno );
	return;
}

/**
\copydoc SWTrig
*/
DllAccess void DLLSWTrig( UINT32 drvno )
{
	SWTrig( drvno );
	return;
}

/**
\copydoc FFValid
*/
DllAccess UINT8 DLLFFValid( UINT32 drvno )
{
	if (FFValid( drvno ) == TRUE) { return 1; }
	else return 0;
}

/**
\copydoc SetExtFFTrig
*/
DllAccess void DLLSetExtTrig( UINT32 drvno )
{
	SetExtFFTrig( drvno );
	return;
}

/**
\copydoc SetIntFFTrig
*/
DllAccess void DLLSetIntTrig( UINT32 drvno )
{
	SetIntFFTrig( drvno );// set hw register
	return;
}

/**
\copydoc FFOvl
*/
DllAccess UINT8 DLLFFOvl( UINT32 drvno )
{
	if (FFOvl( drvno ) == TRUE) { return 1; }
	else return 0;
}

/**
\copydoc SetupVCLKReg
*/
DllAccess UINT8 DLLSetupVCLK( UINT32 drvno, UINT32 lines, UINT8 vfreq )
{
	return SetupVCLKReg( drvno, lines, vfreq );
}

/**
\copydoc BlockTrig
*/
DllAccess UINT8 DLLBlockTrig( UINT32 drv, UCHAR btrig_ch )
{//get trigger state ext input
	if (BlockTrig( drv, btrig_ch ) == TRUE) { return 1; }
	else return 0;
}

/**
\brief For test purposes only: output of 2 strings.
\param testMsg1 string1
\param testMsg2 string2
\return none
*/  
void TestMsg( char testMsg1[20], char testMsg2[20] )
{
	if (MessageBox( GetActiveWindow(), testMsg1, testMsg2, MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	return;
}

/**
\copydoc SetS0Bit
*/
DllAccess UINT8 DLLSetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno )
{
	return SetS0Bit( bitnumber, Address, drvno );
}

/**
\copydoc ResetS0Bit
*/
DllAccess UINT8 DLLResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno )
{
	return ResetS0Bit( bitnumber, Address, drvno );
}

/**
\copydoc ticksTimestamp
*/
DllAccess UINT64 DLLTicksTimestamp( void )
{
	WDC_Err( "entered tickstimestamp\n" );
	return ticksTimestamp();
}

/**
\brief Translate ticks to micro seconds.
\param tks ticks of system timer
\return micro seconds of tks
*/
DllAccess UINT32 DLLTickstous( UINT64 tks )
{
	return Tickstous( tks );
}

/**
\brief Setup DMA initiated by hardware DREQ

Call this func once as it takes time to allocate the resources.
But be aware: the buffer size and nos is set here and may not be changed later.
If size changes: DLLClenupDMA and DLLSetupDMA must be called.
Read nos lines from FIFO, copy to just just one very big contigous block: pdioden.
\param drvno PCIe board identifier.
\param pdioden Pointer to destination.
\param nos number of samples
\param nob number of blocks
\return none
*/
DllAccess void DLLSetupDMA( UINT32 drv, void* pdioden, UINT32 nos, UINT32 nob )
{
 //local declarations
 //	char string[20] = "";
 //	void *dummy = NULL;
	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	BOOL ExTrig = FALSE;
	ULONG  lcnt = 0;
	PUSHORT pdest;
	BYTE	cnt = 0;
	int i = 0;
	ULONG dwdata;

	WDC_Err( "entered DLLSetupDMA\n" );

	if (!DBGNOCAM)
	{
		//Check if Camera there
		if (!FindCam( drv ))
		{
			ErrorMsg( "no Camera found" );
			return;
		}
	}

	//stop all and clear FIFO
	StopFFTimer( drv );
	SetIntFFTrig( drv );
	RSFifo( drv );

	//pass mem pointer to DMA ISR via global pDMABigBuf before calling SetupPCIE_DMA!
	//pDMABigBuf = pdioden;
	pDMABigBufBase[drv] = pdioden;
						   //ErrorMsg(" Camera found"); //without this message is a crash in the first call ...
						   //must before the functionSetupPCIE_DMA or after DLLDrvInit
						   //Sleep(1000);
	if (!SetupPCIE_DMA( drv, nos, nob ))  //get also buffer address
	{
		ErrorMsg( "Error in SetupPCIE_DMA" );
		return;
	}
	return;
}//DLLSetupDMA

/**
\brief Setup DMA initiated by hardware DREQ.

Call this func once as it takes time to allocate the resources.
But be aware: the buffer size and nos is set here and may not be changed later.
If size changes: DLLClenupDMA and DLLSetupDMA must be called.
Read nos lines from FIFO, copy to just  one very big contigous block: pDMABigBufBase.
\param drvno PCIe board identifier.
\param nos number of samples
\param nob number of blocks
\return none
*/
DllAccess void nDLLSetupDMA( UINT32 drv, UINT32 nos, UINT32 nob )
{
	//local declarations
//	char string[20] = "";
//	void *dummy = NULL;
	BOOL Abbruch = FALSE;
	BOOL Space = FALSE;
	BOOL ExTrig = FALSE;
	ULONG  lcnt = 0;
	PUSHORT pdest;
	BYTE	cnt = 0;
	int i = 0;
	ULONG dwdata;
	UINT64 memory_all_tmp = 0;
	UINT64 memory_free_tmp = 0;
	UINT64* memory_all = &memory_all_tmp;//needed to be initialized for errorfree working
	UINT64* memory_free = &memory_free_tmp;
	INT64 needed_mem;
	INT64 needed_mem_mb;
	INT64 memory_free_mb = 0;
	Nob = nob;
	Nospb = nos;

	WDC_Err( "entered nDLLSetupDMA with drv: %i nos: %i and nob: %i and camcnt: %i\n", drv, nos, nob, aCAMCNT[drv] );

	//checks if the dam routine was already called
	if (isDmaSet( drv ))
	{
		CleanupPCIE_DMA( drv );
	}

	/*if (!DBGNOCAM)
	{
		//Check if Camera there
		if (!FindCam(drvno))
		{
			ErrorMsg("no Camera found");
			return;
		}
	}*/

	//stop all and clear FIFO
	StopFFTimer( drv );
	SetIntFFTrig( drv );
	RSFifo( drv );

	//pass mem pointer to DMA ISR via global pDMABigBuf before calling SetupPCIE_DMA!
	//pDMABigBuf = pdioden;

	FreeMemInfo( memory_all, memory_free );
	memory_free_mb = *memory_free / (1024 * 1024);
	needed_mem = (INT64)aCAMCNT[drv] * (INT64)nob * (INT64)nos * (INT64)aPIXEL[drv] * (INT64)sizeof( USHORT );
	needed_mem_mb = needed_mem / (1024 * 1024);

	//check if enough space is available in the physical ram
	if (*memory_free > (UINT64)needed_mem)
	{
		pDMABigBufBase[drv] = calloc( aCAMCNT[drv] * (nos)*nob * aPIXEL[drv], sizeof( USHORT ) );   // +1 oder *2 weil sonst absturz im continuous mode
		// sometimes it makes one ISR more, so better to allocate nos+1 thaT IN THIS CASE THE ADDRESS pDMAIndex is valid
		WDC_Err( "available memory:%lld MB\n \tmemory needed: %lld MB\n", memory_free_mb, needed_mem_mb );
	}
	else
	{
		ErrorMsg( "Not enough physical RAM available!" );
		WDC_Err( "ERROR for buffer %d: available memory: %lld MB \n \tmemory needed: %lld MB\n", number_of_boards, memory_free_mb, needed_mem_mb );
	}
	//pDIODEN = (pArrayT)calloc(nob, nospb * _PIXEL * sizeof(ArrayT));
	//pDMABigBufBase[drvno] = pdioden;
	//ErrorMsg(" Camera found"); //without this message is a crash in the first call ...
	//must before the functionSetupPCIE_DMA or after DLLDrvInit
	//Sleep(1000);

	if (!SetupPCIE_DMA( drv, nos, nob ))  //get also buffer address
	{
		ErrorMsg( "Error in SetupPCIE_DMA" );
		return;
	}
	return;
}//nDLLSetupDMA

/**
\brief Copies one frame of pixel data to pdioden.
\param drvno indentifier of PCIe card
\param curr_nos position in samples (0...nos)
\param curr_nob position in blocks (0...nob)
\param curr_cam position in camera count (0...CAMCNT)
\param pdioden address where data is written, should be buffer with size length * sizeof( USHORT )
\param length lenght of frame, typically pixel count (1088)
\return void
*/
DllAccess void DLLReturnFrame( UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16 *pdioden, UINT32 length )
{
	void* pframe = GetAddressOfPixel( drv, 0, curr_nos, curr_nob, curr_cam );
	memcpy( pdioden, pframe, length * sizeof( USHORT ) );  // length in bytes
	/*
	WDC_Err( "RETURN FRAME: drvno: %u, curr_nos: %u, curr_nob: %u, curr_cam: %u, _PIXEL: %u, length: %u\n", drvno, curr_nos, curr_nob, curr_cam, _PIXEL, length );
	WDC_Err("FRAME2: address Buff: 0x%x \n", pDMABigBufBase[drvno]);
	WDC_Err("FRAME2: address pdio: 0x%x \n", pdioden);
	WDC_Err("FRAME3: pix42 of ReturnFrame: %d \n", *((USHORT*)pdioden + 420));
	WDC_Err("FRAME3: pix43 of ReturnFrame: %d \n", *((USHORT*)pdioden + 422));
	*/
	return;
}

/**
\brief Read nos lines from FIFO. Const burst loop with DMA initiated by hardware DREQ. Is called automatically for 2 boards.
\param board_sel board number (=1 if one PCI board)
\param exptus exposure time in micro sec. If this entry is used, freq must be set to 0
\param exttrig true (not 0) if external trigger for each scan, 0 else
\param blocktrigger true (not 0) if one external trigger starts block of nos scans which run with internal timer
\param btrig_ch 
	- btrig_ch=0 -> no read of state is performed
	- btrig_ch=1 is pci tig in
	- btrig_ch=2 is opto1
	- btrig_ch=3 is opto2
\return none
*/
DllAccess void nDLLReadFFLoop( UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch )
{
	params.board_sel = board_sel;
	params.exptus = exptus;
	params.exttrig = exttrig;
	params.blocktrigger = blocktrigger;
	params.btrig_ch = btrig_ch;

	//thread wit prio 15
	_beginthreadex( 0, 0, &ReadFFLoopThread, &params, 0, 0 );//cam_thread[0] = (HANDLE)_beginthreadex(0, 0, &ReadFFLoopThread, &params, 0, 0);//threadex
//}
/*
	if (number_of_boards == 2 && (cam_sel == 2 || cam_sel == 3)){
		//struct has to be volatile, if not readffloop is always called with drvno=1
		params2.drvno = 2;
		params2.exptus = exptus;
		params2.exttrig = exttrig;
		params2.blocktrigger = blocktrigger;
		params2.btrig_ch = btrig_ch;

		//_beginthread(ReadFFLoopThread, 0, &params2);//thread
		_beginthreadex(0, 0, &ReadFFLoopThread, &params2, 0, 0);//cam_thread[1] = (HANDLE)_beginthreadex(0, 0, &ReadFFLoopThread , &params2, 0, 0); //threadex
	}
	*/
	//Sleep(100);//thread
	//wait until readprocess is finished
	//WaitForMultipleObjects(2, cam_thread, TRUE, INFINITE);//threadex

	//CloseHandle(cam_thread[0]);//threadex
	//CloseHandle(cam_thread[1]);//threadex
	return;
}//DLLReadFFLoop

/**
\brief Abort measurement.
\return none
*/
DllAccess void DLLStopFFLoop( void )
{
	escape_readffloop = TRUE;
	return;
}

/**
\brief Activate or deactivate continuous read.
\param activate 0 - deactivate, 1 - activate
\return none
*/
DllAccess void DLLSetContFFLoop( UINT8 activate )
{
	contffloop = activate;//0 or 1
	return;
}

/**
\copydoc SetTemp
*/
DllAccess void DLLSetTemp( UINT32 drvno, UINT8 level )
{
	SetTemp( drvno, level );
	return;
}

/**
\copydoc SetEC
*/
DllAccess void DLLSetEC( UINT32 drvno, UINT64 ecin100ns )
{
	SetEC( drvno, ecin100ns );
	return;
}

/**
\copydoc ResetEC
*/
DllAccess void DLLResetEC( UINT32 drvno )
{
	ResetEC( drvno );
	return;
}

/**
\copydoc SetTORReg
*/
DllAccess void DLLSetTORReg( UINT32 drvno, UINT8 fkt )
{
	SetTORReg( drvno, fkt );
	return;
}


/**
\copydoc SetISPDA
*/
DllAccess void DLLSetISPDA( UINT32 drvno, UINT8 set )
{
	if (set == 0)
	{
		SetISPDA( drvno, FALSE );
	}
	else SetISPDA( drvno, TRUE );
	return;
}

/**
\copydoc SetPDAnotFFT
*/
DllAccess void DLLSetPDAnotFFT( UINT32 drvno, UINT8 set )
{
	if (set == 0)
	{
		SetPDAnotFFT( drvno, FALSE );
	}
	else SetPDAnotFFT( drvno, TRUE );
	return;
}

/**
\copydoc SetISFFT
*/
DllAccess void DLLSetISFFT( UINT32 drvno, UINT8 set )
{
	if (set == 0)
	{
		SetISFFT( drvno, FALSE );
	}
	else SetISFFT( drvno, TRUE );
	return;
}

/**
\copydoc RsTOREG
*/
DllAccess void DLLRsTOREG( UINT32 drvno )
{
	RsTOREG( drvno );
	return;
}

/**
\copydoc SetupVPB
*/
DllAccess void DLLSetupVPB( UINT32 drvno, UINT32 range, UINT32 lines, UINT8 keep )
{
	if (keep != 0)
	{
		SetupVPB( drvno, range, lines, TRUE );
	}
	else
		SetupVPB( drvno, range, lines, FALSE );
	return;
}

/**
\copydoc AboutS0
*/
DllAccess void DLLAboutS0( UINT32 drvno )
{
	AboutS0( drvno );
	return;
}

/**
\copydoc SendFLCAM
*/
DllAccess void DLLSendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data )
{
	SendFLCAM( drvno, maddr, adaddr, data );
	return;
}

/**
\copydoc SendFLCAM_DAC
*/
DllAccess void DLLSendFLCAM_DAC( UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature )
{
	SendFLCAM_DAC( drvno, ctrl, addr, data, feature );
	return;
}

/**
\copydoc DAC_setOutput
*/
DllAccess void DLLDAC_setOutput( UINT32 drvno, UINT8 channel, UINT16 output )
{
	DAC_setOutput( drvno, channel, output );
	return;
}

/**
\copydoc FreeMemInfo
*/
DllAccess void DLLFreeMemInfo( UINT64 * pmemory_all, UINT64 * pmemory_free )
{
	FreeMemInfo( pmemory_all, pmemory_free );
	return;
}

/**
\copydoc ErrorMsg
*/
DllAccess void DLLErrorMsg( char ErrMsg[20] )
{
	ErrorMsg( ErrMsg );
	return;
}

/**
\copydoc CalcTrms
*/
DllAccess void DLLCalcTrms( UINT32 drvno, UINT32 nos, ULONG TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms )
{
	CalcTrms( drvno, nos, TRMS_pixel, CAMpos, mwf, trms );
	return;
}

/**
\copydoc InitGPX
*/
DllAccess void DLLInitGPX( UINT32 drvno, UINT32 delay )
{
	InitGPX( drvno, delay );
	return;
}

/**
\copydoc AboutGPX
*/
DllAccess void DLLAboutGPX( UINT32 drvno )
{
	AboutGPX( drvno );
	return;
}

/**
\copydoc InitCamera3001
*/
DllAccess void DLLInitCamera3001( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT16 IS_FFT, UINT16 IS_AREA )
{
	InitCamera3001( drvno, pixel, trigger_input, IS_FFT, IS_AREA );
	return;
}

/**
\copydoc InitCamera3010
*/
DllAccess void DLLInitCamera3010( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT8 adc_mode, UINT16 custom_pattern, UINT16 led_on, UINT16 gain_high )
{
	InitCamera3010( drvno, pixel, trigger_input, adc_mode, custom_pattern, led_on, gain_high );
	return;
}

/**
\copydoc InitCamera3030
*/
DllAccess void DLLInitCamera3030( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain )
{
	InitCamera3030( drvno, adc_mode, custom_pattern, gain );
	return;
}

/**
\copydoc BlockSyncStart
*/
DllAccess void DLLBlockSyncStart( UINT32 drvno, UINT8 S1, UINT8 S2 )
{
	BlockSyncStart( drvno, S1, S2 );
	return;
}

/**
\copydoc InitProDLL
*/
DllAccess void DLLInitProDLL()
{
	InitProDLL();
	return;
}

/**
\copydoc SetupFullBinning
*/
DllAccess UINT8 DLLSetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq )
{
	return SetupFullBinning( drvno, lines, vfreq );
}