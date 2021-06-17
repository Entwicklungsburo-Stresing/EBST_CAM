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

LVUserEventRef measureStartLVEvent;
LVUserEventRef measureDoneLVEvent;
LVUserEventRef blockStartLVEvent;
LVUserEventRef blockDoneLVEvent;
int nProcessCount = 0;
int nThreadCount = 0;

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
DllAccess void DLLErrMsgBoxOn()
{
	ErrMsgBoxOn();
	return;
}

/**
\copydoc ErrMsgBoxOff
*/
DllAccess void DLLErrMsgBoxOff()
{
	ErrMsgBoxOff();
	return;
}

/**
 * \copydoc CCDDrvInit
 * \param _number_of_boards Pointer for returning recognized number of PCIe boards.
 */
DllAccess es_status_codes DLLCCDDrvInit( UINT8* _number_of_boards )
{
	es_status_codes status = CCDDrvInit();
	if (status == es_no_error)
		*_number_of_boards = number_of_boards;
	return status;
}

/**
 * \copydoc CCDDrvExit
 */
DllAccess es_status_codes DLLCCDDrvExit( UINT32 drvno )
{
	return CCDDrvExit( drvno );
}

/**
 * \copydoc InitBoard
 */
DllAccess es_status_codes DLLInitBoard( UINT32 drv )
{
	return InitBoard(drv);
}

/**
\copydoc ReadByteS0
*/
DllAccess es_status_codes DLLReadByteS0( UINT32 drvno, UINT8 *data, UINT32 PortOff )
{
	return ReadByteS0( drvno, data, PortOff );
}

/**
\copydoc WriteByteS0
*/
DllAccess es_status_codes DLLWriteByteS0( UINT32 drv, UINT8 DataByte, UINT32 PortOff )
{
	return WriteByteS0( drv, DataByte, PortOff );
}

/**
\copydoc ReadLongS0
*/
DllAccess es_status_codes DLLReadLongS0( UINT32 drvno, UINT32 * DWData, UINT32 PortOff )
{
	return ReadLongS0( drvno, DWData, PortOff );
}

/**
\copydoc WriteLongS0
*/
DllAccess es_status_codes DLLWriteLongS0( UINT32 drvno, UINT32 DWData, UINT32 PortOff )
{
	return WriteLongS0( drvno, DWData, PortOff );
}

/**
\copydoc ReadLongDMA
*/
DllAccess es_status_codes DLLReadLongDMA( UINT32 drvno, UINT32* DWData, UINT32 PortOff )
{
	return ReadLongDMA( drvno, DWData, PortOff );
}

/**
\copydoc WriteLongDMA
*/
DllAccess es_status_codes DLLWriteLongDMA( UINT32 drvno, UINT32 DWData, UINT32 PortOff )
{
	return WriteLongDMA( drvno, DWData, PortOff );
}

/**
\copydoc ReadLongIOPort
*/
DllAccess es_status_codes DLLReadLongIOPort( UINT32 drvno, UINT32 * DWData, UINT32 PortOff )
{
	return ReadLongIOPort( drvno, DWData, PortOff );
}

/**
\copydoc WriteLongIOPort
*/
DllAccess es_status_codes DLLWriteLongIOPort( UINT32 drvno, UINT32 DataL, UINT32 PortOff )
{
	return WriteLongIOPort( drvno, DataL, PortOff );
}

/**
 * \copydoc AboutDrv
 */
DllAccess es_status_codes DLLAboutDrv( UINT32 drvno )
{
	es_status_codes status = AboutDrv( drvno );
	if (status != es_no_error) return status;
	if (number_of_boards == 2)
		status = AboutDrv( 2 );
	return status;
}

/**
 * \copydoc CalcRamUsageInMB
 */
DllAccess double DLLCalcRamUsageInMB( UINT32 nos, UINT32 nob )
{
	return CalcRamUsageInMB( nos, nob );
}

/**
 * \copydoc CalcMeasureTimeInSeconds
 */
DllAccess double DLLCalcMeasureTimeInSeconds( UINT32 nos, UINT32 nob, double exposure_time_in_ms )
{
	return CalcMeasureTimeInSeconds( nos, nob, exposure_time_in_ms );
}

/**
 * \copydoc SetSSlope
 */
DllAccess es_status_codes DLLSetSSlope( UINT32 drvno, UINT32 sslope )
{
	return SetSSlope( drvno, sslope );
}

/**
 * \copydoc OutTrigHigh
 */
DllAccess es_status_codes DLLOutTrigHigh( UINT32 drvno )
{
	return OutTrigHigh( drvno );
}

/**
 * \copydoc OutTrigLow
 */
DllAccess es_status_codes DLLOutTrigLow( UINT32 drvno )
{
	return OutTrigLow( drvno );
}

/**
 * \copydoc OutTrigPulse
 */
DllAccess es_status_codes DLLOutTrigPulse( UINT32 drvno, UINT32 PulseWidth )
{
	return OutTrigPulse(drvno, PulseWidth);
}

/**
 * \copydoc OpenShutter
 */
DllAccess es_status_codes DLLOpenShutter( UINT32 drvno )
{
	return OpenShutter( drvno );
}

/**
 * \copydoc CloseShutter
 */
DllAccess es_status_codes DLLCloseShutter( UINT32 drvno )
{
	return CloseShutter( drvno );
}

/**
 * \copydoc SWTrig
 */
DllAccess es_status_codes DLLSWTrig( UINT32 drvno )
{
	return SWTrig( drvno );
}

/**
\copydoc checkFifoFlags
*/
DllAccess es_status_codes DLLFFValid(UINT32 drvno, UINT8* valid)
{
	return checkFifoFlags( drvno, valid );
}

/**
 * \copydoc checkFifoOverflow
 */
DllAccess es_status_codes DLLFFOvl(UINT32 drvno, UINT8* overflow)
{
	return checkFifoOverflow(drvno, overflow);
}

/**
 * \copydoc SetupVCLKReg
 */
DllAccess es_status_codes DLLSetupVCLK( UINT32 drvno, UINT32 lines, UINT8 vfreq )
{
	return SetupVCLKReg( drvno, lines, vfreq );
}

/**
 * \copydoc readBlockTriggerState
 */
DllAccess es_status_codes DLLreadBlockTriggerState( UINT32 drv, UCHAR btrig_ch, BOOL* state )
{
	return readBlockTriggerState( drv, btrig_ch, state );
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
 * \copydoc SetS0Bit
 */
DllAccess es_status_codes DLLSetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno )
{
	return SetS0Bit( bitnumber, Address, drvno );
}

/**
 * \copydoc ResetS0Bit
 */
DllAccess es_status_codes DLLResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno )
{
	return ResetS0Bit( bitnumber, Address, drvno );
}

/**
 * \copydoc ticksTimestamp
 */
DllAccess UINT64 DLLTicksTimestamp( void )
{
	WDC_Err( "entered tickstimestamp\n" );
	return ticksTimestamp();
}

/**
 * \copydoc Tickstous
 */
DllAccess UINT32 DLLTickstous( UINT64 tks )
{
	return Tickstous( tks );
}

/**
 * \copydoc SetMeasurementParameters
 */
DllAccess es_status_codes DLLSetMeasurementParameters( UINT32 drvno, UINT32 nos, UINT32 nob )
{
	return SetMeasurementParameters( drvno, nos, nob );
}

/**
 * \copydoc ReturnFrame
 */
DllAccess es_status_codes DLLReturnFrame( UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16 *pdest, UINT32 length )
{
	return ReturnFrame( drv, curr_nos, curr_nob, curr_cam, pdest, length );
}

/**
 * \brief Copies all pixel data to pdest
 * 
 * \param drv indentifier of PCIe card
 * \param pdest address where data is written, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( UINT16 )
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLCopyAllData( UINT32 drv, UINT16 *pdest )
{
	UINT16* pframe = NULL;
	es_status_codes status = GetAddressOfPixel(drv, 0, 0, 0, 0, &pframe);
	if (status != es_no_error) return status;
	memcpy( pdest, pframe, (UINT64)(*Nospb) * (UINT64)Nob * (UINT64)aCAMCNT[drv] * (UINT64)aPIXEL[drv] * sizeof( UINT16 ) );  // length in bytes
	return status;
}

/**
 * \brief Copies one block of pixel data to pdest
 * 
 * \param drv indentifier of PCIe card
 * \param block Selects which block to copy.
 * \param pdest address where data is written, should be a buffer with size: nos * camcnt * pixel * sizeof( UINT16 )
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLCopyOneBlock( UINT32 drv, UINT16 block, UINT16 *pdest )
{
	UINT16* pframe = NULL;
	es_status_codes status = GetAddressOfPixel( drv, 0, 0, block, 0, &pframe);
	if (status != es_no_error) return status;
	memcpy( pdest, pframe, (UINT64)(*Nospb) * (UINT64)aCAMCNT[drv] * (UINT64)aPIXEL[drv] * sizeof( UINT16 ) );  // length in bytes
	return status;
}

/**
 * \brief Read nos lines from FIFO. Const burst loop with DMA initiated by hardware DREQ. Is called automatically for 2 boards.
 * 
 * \param board_sel board number (=1 if one PCI board)
 * \return none
 */
DllAccess void DLLReadFFLoop( UINT32 board_sel )
{
	BOARD_SEL = board_sel;
	//thread wit prio 15
	_beginthreadex( 0, 0, &ReadFFLoopThread, 0, 0, 0 );
	return;
}//DLLReadFFLoop

/**
 * \brief Abort measurement.
 * \return none
 */
DllAccess void DLLStopFFLoop()
{
	escape_readffloop = TRUE;
	return;
}

/**
 * \brief Activate or deactivate continuous read.
 * \param activate 0 - deactivate, 1 - activate
 * \param pause - time in ms before next loop starts - should be >=1
 * \return none
 */
DllAccess void DLLSetContFFLoop( UINT8 activate , UINT32 pause)
{
	CONTFFLOOP = activate;//0 or 1
	CONTPAUSE = pause;
	return;
}

/**
 * \copydoc SetTemp
 */
DllAccess es_status_codes DLLSetTemp( UINT32 drvno, UINT8 level )
{
	return SetTemp(drvno, level);
}

/**
 * \copydoc SetSEC
 */
DllAccess es_status_codes DLLSetSEC( UINT32 drvno, UINT64 ecin100ns )
{
	return SetSEC( drvno, ecin100ns );
}

/**
 * \copydoc SetBEC
 */
DllAccess es_status_codes DLLSetBEC( UINT32 drvno, UINT64 ecin100ns )
{
	return SetBEC( drvno, ecin100ns );
}

/**
 * \copydoc SetTORReg
 */
DllAccess es_status_codes DLLSetTORReg( UINT32 drvno, UINT8 fkt )
{
	return SetTORReg( drvno, fkt );
}

/**
 * \copydoc SetSensorType
 */
DllAccess es_status_codes DLLSetSensorType( UINT32 drvno, UINT8 sensor_type )
{
	return SetSensorType( drvno, sensor_type );
}

/**
 * \copydoc SetupVPB
 */
DllAccess es_status_codes DLLSetupVPB( UINT32 drvno, UINT32 range, UINT32 lines, UINT8 keep )
{
	if (keep != 0)
		return SetupVPB( drvno, range, lines, TRUE );
	else
		return SetupVPB( drvno, range, lines, FALSE );
}

/**
 * \copydoc AboutS0
 */
DllAccess es_status_codes DLLAboutS0( UINT32 drvno )
{
	return AboutS0( drvno );
}

/**
 * \copydoc SendFLCAM
 */
DllAccess es_status_codes DLLSendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data )
{
	return SendFLCAM( drvno, maddr, adaddr, data );
}

/**
 * \copydoc SendFLCAM_DAC
 */
DllAccess es_status_codes DLLSendFLCAM_DAC( UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature )
{
	return SendFLCAM_DAC( drvno, ctrl, addr, data, feature );
}

/**
 * \copydoc DAC_setOutput
 */
DllAccess es_status_codes DLLDAC_setOutput( UINT32 drvno, UINT8 channel, UINT16 output )
{
	return DAC_setOutput( drvno, channel, output );
}

/**
 * \copydoc FreeMemInfo
 */
DllAccess void DLLFreeMemInfo( UINT64 * pmemory_all, UINT64 * pmemory_free )
{
	FreeMemInfo( pmemory_all, pmemory_free );
	return;
}

/**
 * \copydoc ErrorMsg
 */
DllAccess void DLLErrorMsg( char ErrMsg[20] )
{
	ErrorMsg( ErrMsg );
	return;
}

/**
 * \copydoc CalcTrms
 */
DllAccess es_status_codes DLLCalcTrms( UINT32 drvno, UINT32 firstSample, UINT32 lastSample, UINT32 TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms )
{
	return CalcTrms( drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf, trms );
}

/**
 * \copydoc InitGPX
 */
DllAccess es_status_codes DLLInitGPX( UINT32 drvno, UINT32 delay )
{
	return InitGPX( drvno, delay );
}

/**
 * \copydoc AboutGPX
 */
DllAccess es_status_codes DLLAboutGPX( UINT32 drvno )
{
	return AboutGPX( drvno );
}

/**
 * \copydoc InitCamera3001
 */
DllAccess es_status_codes DLLInitCameraGeneral( UINT32 drvno, UINT16 pixel, UINT16 cc_trigger_input, UINT8 IS_FFT, UINT8 IS_AREA, UINT8 IS_COOLED, UINT16 led_off )
{
	return InitCameraGeneral( drvno, pixel, cc_trigger_input, IS_FFT, IS_AREA, IS_COOLED, led_off );
}

/**
 * \copydoc InitCamera3001
 */
DllAccess es_status_codes DLLInitCamera3001( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT16 IS_FFT, UINT16 IS_AREA )
{
	return InitCamera3001( drvno, pixel, trigger_input, IS_FFT, IS_AREA );
}

/**
 * \copydoc InitCamera3010
 */
DllAccess es_status_codes DLLInitCamera3010( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT16 gain_switch )
{
	return InitCamera3010( drvno, adc_mode, custom_pattern, gain_switch );
}

/**
 * \copydoc InitCamera3030
 */
DllAccess es_status_codes DLLInitCamera3030( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain )
{
	return InitCamera3030( drvno, adc_mode, custom_pattern, gain );
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
 * \copydoc SetupFullBinning
 */
DllAccess es_status_codes DLLSetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq )
{
	return SetupFullBinning( drvno, lines, vfreq );
}

/**
 * \copydoc isMeasureOn
 */
DllAccess es_status_codes DLLisMeasureOn( UINT32 drvno, UINT8* measureOn )
{
	return isMeasureOn( drvno, measureOn );
}

/**
 * \copydoc isBlockOn
 */
DllAccess es_status_codes DLLisBlockOn( UINT32 drvno, UINT8* blockOn )
{
	return isBlockOn( drvno, blockOn );
}

/**
 * \copydoc waitForMeasureReady
 */
DllAccess es_status_codes DLLwaitForMeasureReady( UINT32 drvno )
{
	return waitForMeasureReady( drvno );
}

/**
 * \copydoc waitForBlockReady
 */
DllAccess es_status_codes DLLwaitForBlockReady( UINT32 drvno )
{
	return waitForBlockReady( drvno );
}

/**
 * \copydoc SetBTI
 */
DllAccess es_status_codes DLLSetBTI( UINT32 drvno, UINT8 bti_mode )
{
	return SetBTI(drvno, bti_mode);
}

/**
 * \copydoc SetSTI
 */
DllAccess es_status_codes DLLSetSTI( UINT32 drvno, UINT8 sti_mode )
{
	return SetSTI( drvno, sti_mode );
}

/**
\copydoc ClearAllUserRegs
*/
DllAccess es_status_codes DLLClearAllUserRegs( UINT32 drv )
{
	return ClearAllUserRegs( drv );
}

/**
 * \copydoc SetSTimer
 */
DllAccess es_status_codes DLLSetSTimer( UINT32 drvno, UINT32 stime_in_microseconds )
{
	return SetSTimer(drvno, stime_in_microseconds);
}

/**
 * \copydoc SetBTimer
 */
DllAccess es_status_codes DLLSetBTimer( UINT32 drvno, UINT32 btime_in_microseconds )
{
	return SetBTimer( drvno, btime_in_microseconds );
}

/**
 * \copydoc SetBSlope
 */
DllAccess es_status_codes DLLSetBSlope( UINT32 drvno, UINT32 slope )
{
	return SetBSlope( drvno, slope );
}

/**
 * \copydoc LedOn
 */
DllAccess es_status_codes DLLSetGain( UINT32 drvno, UINT16 gain_value )
{
	return SetGain( drvno, gain_value );
}

/**
 * \copydoc LedOn
 */
DllAccess es_status_codes DLLLedOff( UINT32 drvno, UINT8 LED_OFF )
{
	return LedOff( drvno, LED_OFF );
}

/**
 * \brief Save the user event handlers created by Labview. Call this before using the event structure.
 * 
 * \param measureStartEvent Event handler for the event measure start.
 * \param measureDoneEvent Event handler for the event measure done.
 * \param blockStartEvent Event handler for the event block start.
 * \param blockDoneEvent Event handler for the event block done.
 * \return none
 */
DllAccess void DLLRegisterLVEvents( LVUserEventRef *measureStartEvent, LVUserEventRef *measureDoneEvent, LVUserEventRef *blockStartEvent, LVUserEventRef *blockDoneEvent )
{
	measureStartLVEvent = *measureStartEvent;
	measureDoneLVEvent = *measureDoneEvent;
	blockStartLVEvent = *blockStartEvent;
	blockDoneLVEvent = *blockDoneEvent;
	return;
}

DllAccess CStr DLLConvertErrorCodeToMsg(es_status_codes status)
{
	return ConvertErrorCodeToMsg(status);
}

/**
 * \copydoc SetupPCIE_DMA
 */
DllAccess es_status_codes DLLSetupPCIE_DMA(UINT32 drvno)
{
	return SetupPCIE_DMA(drvno);
}

/**
 * \copydoc SetTLPS
 */
DllAccess es_status_codes DLLSetTLPS(UINT32 drvno, UINT32 pixel)
{
	return SetTLPS(drvno, pixel);
}

/**
 * \copydoc SetSDAT
 */
DllAccess es_status_codes DLLSetSDAT(UINT32 drvno, UINT32 datin100ns)
{
	return SetSDAT(drvno, datin100ns);
}

/**
 * \copydoc SetBDAT
 */
DllAccess es_status_codes DLLSetBDAT(UINT32 drvno, UINT32 tin100ns)
{
	return SetBDAT(drvno, tin100ns);
}

/**
 * \brief Sets global variable useSWTrig.
 * 
 * The default of useSWTrig is FALSE and useSWTrig is set automatically in InitMeasurement depending on FFT mode.
 * Use this function to differ from the default behaviour.
 * \param on If TRUE BON starts immediately a scan. Must be TRUE for FFT in Area or ROI mode.
 * \return none
 */
DllAccess void setSWTrig(BOOL on)
{
	*useSWTrig = on;
	return;
}

/**
 * \copydoc InitMeasurement
 */
DllAccess es_status_codes DLLInitMeasurement(struct global_settings settings)
{
	InitMeasurement(settings);
}
