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
#include "shared_src/Direct2dViewer_c.h"

#ifdef COMPILE_FOR_LABVIEW
LVUserEventRef measureStartLVEvent;
LVUserEventRef measureDoneLVEvent;
LVUserEventRef blockStartLVEvent;
LVUserEventRef blockDoneLVEvent;
LVUserEventRef allBlocksDoneLVEvent;
#endif

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
 * \copydoc InitDriver
 * \param _number_of_boards Pointer for returning recognized number of PCIe boards.
 */
DllAccess es_status_codes DLLInitDriver( uint8_t* _number_of_boards )
{
	es_status_codes status = InitDriver();
	if (status == es_no_error)
		*_number_of_boards = number_of_boards;
	return status;
}

/**
 * \copydoc ExitDriver
 */
DllAccess es_status_codes DLLExitDriver()
{
	return ExitDriver();
}

/**
 * \copydoc InitBoard
 */
DllAccess es_status_codes DLLInitBoard()
{
	return InitBoard();
}

/**
\copydoc readRegisterS0_8
*/
DllAccess es_status_codes DLLreadRegisterS0_8( uint32_t board_sel, uint8_t *data, uint32_t address)
{
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
			// this function only returns data for the first found board
			return readRegisterS0_8(drvno, data, address);
	return es_parameter_out_of_range;
}

/**
\copydoc writeRegisterS0_8
*/
DllAccess es_status_codes DLLwriteRegisterS0_8( uint32_t board_sel, uint8_t data, uint32_t address)
{
	return writeRegisterS0_8_allBoards(board_sel, data, address);
}

/**
\copydoc readRegisterS0_32
*/
DllAccess es_status_codes DLLreadRegisterS0_32( uint32_t board_sel, uint32_t* data, uint32_t address)
{
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
			// this function only returns data for the first found board
			return readRegisterS0_32(drvno, data, address);;
	return es_parameter_out_of_range;
}

/**
\copydoc writeRegisterS0_32
*/
DllAccess es_status_codes DLLwriteRegisterS0_32( uint32_t board_sel, uint32_t data, uint32_t address)
{
	return writeRegisterS0_32_allBoards( board_sel, data, address);
}

/**
 * \copydoc About
 */
DllAccess es_status_codes DLLAbout( uint32_t board_sel )
{
	return About(board_sel);
}

/**
 * \copydoc CalcRamUsageInMB
 */
DllAccess double DLLCalcRamUsageInMB( uint32_t nos, uint32_t nob )
{
	return CalcRamUsageInMB( nos, nob );
}

/**
 * \copydoc CalcMeasureTimeInSeconds
 */
DllAccess double DLLCalcMeasureTimeInSeconds( uint32_t nos, uint32_t nob, double exposure_time_in_ms )
{
	return CalcMeasureTimeInSeconds( nos, nob, exposure_time_in_ms );
}

/**
 * \copydoc OutTrigHigh
 */
DllAccess es_status_codes DLLOutTrigHigh( uint32_t board_sel )
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = OutTrigHigh(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc OutTrigLow
 */
DllAccess es_status_codes DLLOutTrigLow( uint32_t board_sel )
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = OutTrigLow(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc OutTrigPulse
 */
DllAccess es_status_codes DLLOutTrigPulse( uint32_t board_sel, uint32_t PulseWidth )
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = OutTrigPulse(drvno, PulseWidth);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc OpenShutter
 */
DllAccess es_status_codes DLLOpenShutter( uint32_t board_sel )
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = OpenShutter(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc CloseShutter
 */
DllAccess es_status_codes DLLCloseShutter( uint32_t board_sel )
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = CloseShutter(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \brief For test purposes only: output of 2 strings.
 *
 * \param testMsg1 string1
 * \param testMsg2 string2
 */
void TestMsg( char testMsg1[20], char testMsg2[20] )
{
	if (MessageBox( GetActiveWindow(), testMsg1, testMsg2, MB_OK | MB_ICONEXCLAMATION ) == IDOK) {};
	return;
}

/**
 * \copydoc setBitS0_32
 */
DllAccess es_status_codes DLLsetBitS0_32( uint32_t board_sel, uint32_t bitnumber, uint16_t address )
{
	return setBitS0_32_allBoards( board_sel, bitnumber, address );
}

/**
 * \copydoc resetBitS0_32
 */
DllAccess es_status_codes DLLresetBitS0_32( uint32_t board_sel, uint32_t bitnumber, uint16_t address )
{
	return resetBitS0_32_allBoards(board_sel, bitnumber, address );
}

/**
 * \copydoc ReturnFrame
 */
DllAccess es_status_codes DLLReturnFrame(uint32_t board_sel, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest0, uint16_t* pdest1, uint32_t pixel)
{
	uint16_t* pdest[2] = { pdest0, pdest1 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = ReturnFrame(drvno, sample, block, camera, pdest[usedBoards], pixel);
			if (status != es_no_error) return status;
			usedBoards++;
			// this function only returns data for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	return status;
}

/**
 * \brief Copies all pixel data to pdest
 * 
 * \param drv indentifier of PCIe card
 * \param pdest address where data is written, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( uint16_t )
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLCopyAllData( uint32_t board_sel, uint16_t *pdest0, uint16_t *pdest1 )
{
	uint16_t* pdest[2] = { pdest0, pdest1 };
	int usedBoards = 0;
	uint16_t* pframe = NULL;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = GetAddressOfPixel(drvno, 0, 0, 0, 0, &pframe);
			if (status != es_no_error) return status;
			memcpy(pdest[usedBoards], pframe, (uint64_t)(*Nospb) * (uint64_t)(*Nob) * (uint64_t)aCAMCNT[drvno] * (uint64_t)aPIXEL[drvno] * sizeof(uint16_t));  // length in bytes
			usedBoards++;
			// this function only returns data for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	return status;
}

/**
 * \brief Copies one block of pixel data to pdest
 * 
 * \param drv indentifier of PCIe card
 * \param block Selects which block to copy.
 * \param pdest address where data is written, should be a buffer with size: nos * camcnt * pixel * sizeof( uint16_t )
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLCopyOneBlock( uint32_t board_sel, uint16_t block, uint16_t *pdest0, uint16_t *pdest1 )
{
	uint16_t* pdest[2] = { pdest0, pdest1 };
	int usedBoards = 0;
	uint16_t* pframe = NULL;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = GetAddressOfPixel(drvno, 0, 0, block, 0, &pframe);
			if (status != es_no_error) return status;
			memcpy(pdest[usedBoards], pframe, (uint64_t)(*Nospb) * (uint64_t)aCAMCNT[drvno] * (uint64_t)aPIXEL[drvno] * sizeof(uint16_t)); // length in bytes
			usedBoards++;
			// this function only returns data for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	return status;
}

/**
 * \brief This function is starting the measurement and returns immediately.
 * 
 * StartMeasurement is run a new thread. When there are multiple boards, all boards are starting the measurement. You can check the status of the measurement with DllisMeasureOn and DllisBlockOn or create a blocking call with DLLwaitForMeasureReady and DLLwaitForBlockReady.
 */
DllAccess void DLLStartMeasurement_nonblocking()
{
	//thread wit prio 15
	_beginthread( &StartMeasurement, 0, NULL );
	return;
}

/**
 * \copydoc StartMeasurement
 */
DllAccess es_status_codes DLLStartMeasurement_blocking()
{
	return StartMeasurement();
}

/**
 * \copydoc SetTemp
 */
DllAccess es_status_codes DLLSetTemp( uint32_t board_sel, uint8_t level )
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = SetTemp(drvno, level);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc SetTORReg
 */
DllAccess es_status_codes DLLSetTORReg( uint32_t board_sel, uint8_t tor)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = SetTORReg(drvno, tor);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc AboutS0
 */
DllAccess es_status_codes DLLAboutS0( uint32_t drvno )
{
	return AboutS0( drvno );
}

/**
 * \copydoc DAC8568_setAllOutputs
 */
DllAccess es_status_codes DLLDAC8568_setAllOutputs(uint32_t board_sel, uint8_t location, uint8_t cameraPosition, uint32_t* output0, uint32_t* output1, uint32_t* output2, uint32_t* output3, uint32_t* output4, uint8_t reorder_channel)
{
	es_status_codes status = es_no_error;
	uint32_t* output[MAXPCIECARDS] = { output0, output1, output2, output3, output4 };
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = DAC8568_setAllOutputs(drvno, location, cameraPosition, output[drvno], reorder_channel);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc FreeMemInfo
 */
DllAccess void DLLFreeMemInfo( uint64_t * pmemory_all, uint64_t * pmemory_free )
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
DllAccess es_status_codes DLLCalcTrms( uint32_t board_sel, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf0, double *trms0, double *mwf1, double *trms1 )
{
	es_status_codes status = es_no_error;
	double* mwf[2] = { mwf0, mwf1 };
	double* trms[2] = { trms0, trms1 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = CalcTrms(drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf[usedBoards], trms[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
			// this function only returns the values for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	}
	return status;
}

/**
 * \copydoc isMeasureOn
 */
DllAccess es_status_codes DLLisMeasureOn( uint32_t board_sel, uint8_t* measureOn0, uint8_t* measureOn1 )
{
	es_status_codes status = es_no_error;
	uint8_t* measureOn[2] = { measureOn0, measureOn1 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = isMeasureOn(drvno, measureOn[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
			// this function only returns the values for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	}
	return status;
}

/**
 * \copydoc isBlockOn
 */
DllAccess es_status_codes DLLisBlockOn( uint32_t board_sel, uint8_t* blockOn0, uint8_t* blockOn1 )
{
	es_status_codes status = es_no_error;
	uint8_t* blockOn[2] = { blockOn0, blockOn1 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = isBlockOn(drvno, blockOn[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
			// this function only returns the values for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	}
	return status;
}

/**
 * \copydoc waitForMeasureReady
 */
DllAccess es_status_codes DLLwaitForMeasureReady( uint32_t board_sel )
{
	return waitForMeasureReady( board_sel );
}

/**
 * \copydoc waitForBlockReady
 */
DllAccess es_status_codes DLLwaitForBlockReady( uint32_t board_sel )
{
	return waitForBlockReady(board_sel);
}

/**
 * \copydoc SetSTimer
 */
DllAccess es_status_codes DLLSetSTimer( uint32_t drvno, uint32_t stime_in_microseconds )
{
	return SetSTimer(drvno, stime_in_microseconds);
}

/**
 * \copydoc SetBTimer
 */
DllAccess es_status_codes DLLSetBTimer( uint32_t drvno, uint32_t btime_in_microseconds )
{
	return SetBTimer( drvno, btime_in_microseconds );
}

#ifdef COMPILE_FOR_LABVIEW
/**
 * \brief Save the user event handlers created by Labview. Call this before using the event structure.
 *
 * \param measureStartEvent Event handler for the event measure start.
 * \param measureDoneEvent Event handler for the event measure done.
 * \param blockStartEvent Event handler for the event block start.
 * \param blockDoneEvent Event handler for the event block done.
 * \return none
 */
DllAccess void DLLRegisterLVEvents( LVUserEventRef *measureStartEvent, LVUserEventRef *measureDoneEvent, LVUserEventRef *blockStartEvent, LVUserEventRef *blockDoneEvent, LVUserEventRef* allBlocksDoneEvent)
{
	measureStartLVEvent = *measureStartEvent;
	measureDoneLVEvent = *measureDoneEvent;
	blockStartLVEvent = *blockStartEvent;
	blockDoneLVEvent = *blockDoneEvent;
	allBlocksDoneLVEvent = *allBlocksDoneEvent;
	return;
}
#endif

DllAccess char* DLLConvertErrorCodeToMsg( es_status_codes status )
{
	return ConvertErrorCodeToMsg( status );
}

/**
 * \copydoc SetGlobalSettings
 */
DllAccess es_status_codes DLLSetGlobalSettings(struct measurement_settings settings)
{
	SetGlobalSettings(settings);
	return es_no_error;
}

/**
 * \bief Set settings with Matlab compatible structs.
 * 
 * \param measurement_s Measurement settings struct without embedded camera settings struct.
 * \param camera_s0 Camera settings for PCIe board 0
 * \param camera_s1 Camera settings for PCIe board 1
 * \param camera_s2 Camera settings for PCIe board 2
 * \param camera_s3 Camera settings for PCIe board 3
 * \param camera_s4 Camera settings for PCIe board 4
 * \return 
 */
DllAccess es_status_codes DLLSetGlobalSettings_matlab(struct measurement_settings_matlab measurement_s, struct camera_settings camera_s0, struct camera_settings camera_s1, struct camera_settings camera_s2, struct camera_settings camera_s3, struct camera_settings camera_s4)
{
	struct measurement_settings settings;
	settings.board_sel = measurement_s.board_sel;
	settings.contiuous_measurement = measurement_s.contiuous_measurement;
	settings.cont_pause_in_microseconds = measurement_s.cont_pause_in_microseconds;
	settings.nob = measurement_s.nob;
	settings.nos = measurement_s.nos;
	struct camera_settings camera_s[MAXPCIECARDS] = { camera_s0 , camera_s1, camera_s2, camera_s3, camera_s4 };
	for (int i = 0; i < MAXPCIECARDS; i++)
		settings.camera_settings[i] = camera_s[i];
	SetGlobalSettings(settings);
	return es_no_error;
}

/**
 * \copydoc InitMeasurement
 */
DllAccess es_status_codes DLLInitMeasurement()
{
	return InitMeasurement();
}

/**
 * \copydoc AbortMeasurement
 */
DllAccess es_status_codes DLLAbortMeasurement()
{
	return AbortMeasurement();
}

/**
 * \copydoc IOCtrl_setOutput
 */
DllAccess es_status_codes DLLIOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	return IOCtrl_setOutput(drvno, number, width_in_5ns, delay_in_5ns);
}

/**
 * \copydoc IOCtrl_setT0
 */
DllAccess es_status_codes DLLIOCtrl_setT0(uint32_t board_sel, uint32_t period_in_10ns)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = IOCtrl_setT0(drvno, period_in_10ns);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc IOCtrl_setAllOutputs
 */
DllAccess es_status_codes DLLIOCtrl_setAllOutputs(uint32_t board_sel, uint32_t* width_in_5ns, uint32_t* delay_in_5ns)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = IOCtrl_setAllOutputs(board_sel, width_in_5ns, delay_in_5ns);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc GetCurrentScanNumber
 */
DllAccess void DLLGetCurrentScanNumber(uint32_t board_sel, int64_t* sample, int64_t* block)
{
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			GetCurrentScanNumber(drvno, sample, block);
			// this function only returns the scan number for the first found board
			return;
		}
	return;
}

/**
 * \copydoc GetIsTdc
 */
DllAccess es_status_codes DLLGetIsTdc(uint32_t board_sel, uint8_t* isTdc0, uint8_t* isTdc1, uint8_t* isTdc2, uint8_t* isTdc3, uint8_t* isTdc4)
{
	uint8_t* isTdc[MAXPCIECARDS] = { isTdc0, isTdc1, isTdc2, isTdc3, isTdc4 };
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = GetIsTdc(drvno, isTdc[drvno]);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc GetIsDsc
 */
DllAccess es_status_codes DLLGetIsDsc(uint32_t board_sel, uint8_t* isDsc0, uint8_t* isDsc1, uint8_t* isDsc2, uint8_t* isDsc3, uint8_t* isDsc4)
{
	uint8_t* isDsc[MAXPCIECARDS] = { isDsc0, isDsc1, isDsc2, isDsc3, isDsc4 };
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = GetIsDsc(drvno, isDsc[drvno]);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc ResetDSC
 */
DllAccess es_status_codes DLLResetDSC(uint32_t board_sel, uint8_t DSCNumber)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = ResetDSC(drvno, DSCNumber);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc SetDIRDSC
 */
DllAccess es_status_codes DLLSetDIRDSC(uint32_t board_sel, uint8_t DSCNumber, uint8_t dir)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = SetDIRDSC(drvno, DSCNumber, dir);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc GetDSC
 */
DllAccess es_status_codes DLLGetDSC(uint32_t board_sel, uint8_t DSCNumber, uint32_t* ADSC0, uint32_t* LDSC0, uint32_t* ADSC1, uint32_t* LDSC1)
{
	es_status_codes status = es_no_error;
	uint32_t* ADSC[2] = { ADSC0, ADSC1 };
	uint32_t* LDSC[2] = { LDSC0, LDSC1 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = GetDSC(drvno, DSCNumber, ADSC[usedBoards], LDSC[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
			// this function only returns the values for the first two found boards
			if (usedBoards >= 2)
				return status;
		}
	}
	return status;
}

DllAccess es_status_codes DLLInitGPX(uint32_t board_sel, uint32_t delay)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = InitGPX(drvno, delay);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

DllAccess void DLLSetContinuousMeasurement(uint8_t on)
{
	SetContinuousMeasurement(on);
	return;
}

DllAccess es_status_codes DLLGetAllSpecialPixelInformation(uint32_t board_sel, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	es_status_codes status = es_no_error;
	uint32_t usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = GetAllSpecialPixelInformation(drvno, sample, block, camera_pos, sp);
			if (status != es_no_error) return status;
			usedBoards++;
			// this function only returns the values for the first found board
			if (usedBoards >= 1)
				return status;
		}
	}
	return status;
}

DllAccess void DLLFillUserBufferWithDummyData(uint32_t board_sel)
{
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
			FillUserBufferWithDummyData(drvno);
	}
	return;
}

/**
* \copydoc Start2dViewer
*/
DllAccess void DLLStart2dViewer(UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos)
{
	Start2dViewer(drvno, cur_nob, cam, pixel, nos);
	return;
}

/**
* \copydoc ShowNewBitmap
*/
DllAccess void DLLShowNewBitmap(UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos)
{
	ShowNewBitmap(drvno, cur_nob, cam, pixel, nos);
	return;
}

/**
* \copydoc Deinit2dViewer
*/
DllAccess void DLLDeinit2dViewer()
{
	Deinit2dViewer();
	return;
}

/**
* \copydoc SetGammaValue
*/
DllAccess void DLLSetGammaValue(UINT16 white, UINT16 black)
{
	SetGammaValue(white, black);
}

/**
* \copydoc GetGammaWhite
*/
DllAccess UINT16 DLLGetGammaWhite()
{
	return GetGammaWhite();
}

/**
* \copydoc GetGammaBlack
*/
DllAccess UINT16 DLLGetGammaBlack()
{
	return GetGammaBlack();
}

/**
* \copydoc SetupROI
*/
es_status_codes DLLSetupROI(UINT32 drvno, UINT16 number_of_regions, UINT32 lines, UINT8 keep, UINT8* region_size, UINT8 vfreq)
{
	return SetupROI(drvno, number_of_regions, lines, keep, region_size, vfreq);
}

/**
* \copydoc SetupArea
*/
DllAccess es_status_codes DLLSetupArea(UINT32 drvno, UINT32 lines_binning, UINT8 vfreq)
{
	return SetupArea(drvno, lines_binning, vfreq);
}
