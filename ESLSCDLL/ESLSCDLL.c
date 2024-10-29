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
#ifdef WIN32
#include "Direct2dViewer_c.h"
#include <process.h>
#endif

#ifdef COMPILE_FOR_LABVIEW
LVUserEventRef measureStartLVEvent;
LVUserEventRef measureDoneLVEvent;
LVUserEventRef blockStartLVEvent;
LVUserEventRef blockDoneLVEvent;
LVUserEventRef allBlocksDoneLVEvent;
#endif

int nProcessCount = 0;
int nThreadCount = 0;

#ifdef WIN32

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
BOOL WINAPI DLLMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
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

#endif

/**
 * \copydoc InitDriver
 * \param _number_of_boards Pointer for returning recognized number of PCIe boards.
 */
DllAccess es_status_codes DLLInitDriver(uint8_t* _number_of_boards)
{
	es_status_codes status = InitDriver();
	if (status == es_no_error)
		*_number_of_boards = number_of_boards;
	return status;
}


/**
 * \copydoc InitBoard
 */
DllAccess es_status_codes DLLInitBoard()
{
	return InitBoard();
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
 * \brief Set settings with Matlab compatible structs.
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
	settings.continuous_measurement = measurement_s.continuous_measurement;
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
 * \copydoc StartMeasurement
 */
DllAccess es_status_codes DLLStartMeasurement_blocking()
{
	return StartMeasurement();
}

#ifdef WIN32

unsigned __stdcall StartMeasurementThread(void* param)
{
	return StartMeasurement();
}

#endif

/**
 * \brief This function is starting the measurement and returns immediately.
 *
 * StartMeasurement is run a new thread. When there are multiple boards, all boards are starting the measurement. You can check the status of the measurement with DllisMeasureOn and DllisBlockOn or create a blocking call with DLLwaitForMeasureReady and DLLwaitForBlockReady.
 */
DllAccess void DLLStartMeasurement_nonblocking()
{
#ifdef WIN32
	_beginthread(&StartMeasurementThread, 0, NULL);
#endif
	return;
}

/**
 * \copydoc SetAbortMeasurementFlag
 */
DllAccess es_status_codes DLLAbortMeasurement()
{
	return SetAbortMeasurementFlag();
}

/**
 * \copydoc CopyOneSample
 */
DllAccess es_status_codes DLLCopyOneSample(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest)
{
	return CopyOneSample(drvno, sample, block, camera, pdest);
}

/**
 * \brief Get data of a single measurement for all boards selected by settings parameter board_sel.
 *
 * \param sample sample number ( 0...(nos - 1) )
 * \param block block number ( 0...(nob - 1) )
 * \param camera camera number ( 0...(CAMCNT - 1) )
 * \param pdest0 Pointer where frame data for board0 will be written. Make sure that the size is >= sizeof(uint16_t) * pixel
 * \param pdest1 Pointer where frame data for board1 will be written. Make sure that the size is >= sizeof(uint16_t) * pixel
 * \param pdest2 Pointer where frame data for board2 will be written. Make sure that the size is >= sizeof(uint16_t) * pixel
 * \param pdest3 Pointer where frame data for board3 will be written. Make sure that the size is >= sizeof(uint16_t) * pixel
 * \param pdest4 Pointer where frame data for board4 will be written. Make sure that the size is >= sizeof(uint16_t) * pixel
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLCopyOneSample_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest0, uint16_t* pdest1, uint16_t* pdest2, uint16_t* pdest3, uint16_t* pdest4)
{
	uint16_t* pdest[MAXPCIECARDS] = { pdest0, pdest1, pdest2, pdest3, pdest4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = CopyOneSample(drvno, sample, block, camera, pdest[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

/**
 * \copydoc CopyAllData
 */
DllAccess es_status_codes DLLCopyAllData(uint32_t drvno, uint16_t* pdest)
{
	return CopyAllData(drvno, pdest);
}

/**
 * \brief Copies all pixel data to pdest for all used boards set in settings parameter board_sel.
 *
 * \param pdest0 Address where data is written for board 0, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( uint16_t )
 * \param pdest1 Address where data is written for board 1, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( uint16_t )
 * \param pdest2 Address where data is written for board 2, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( uint16_t )
 * \param pdest3 Address where data is written for board 3, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( uint16_t )
 * \param pdest4 Address where data is written for board 4, should be a buffer with size: nos * nob * camcnt * pixel * sizeof( uint16_t )
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_memory_not_initialized
 */
DllAccess es_status_codes DLLCopyAllData_multipleBoards(uint16_t* pdest0, uint16_t* pdest1, uint16_t* pdest2, uint16_t* pdest3, uint16_t* pdest4)
{
	uint16_t* pdest[MAXPCIECARDS] = { pdest0, pdest1, pdest2, pdest3, pdest4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = CopyAllData(drvno, pdest[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

/**
 * \copydoc CopyOneBlock
 */
DllAccess es_status_codes DLLCopyOneBlock(uint32_t drvno, uint16_t block, uint16_t* pdest)
{
	return CopyOneBlock(drvno, block, pdest);
}

/**
 * \copydoc CopyDataArbitrary
 */
DllAccess es_status_codes DLLCopyDataArbitrary(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t pixel, uint32_t length_in_pixel, uint16_t* pdest)
{
	return CopyDataArbitrary(drvno, sample, block, camera, pixel, length_in_pixel, pdest);
}

/**
 * \copydoc GetOneSamplePointer
 */
DllAccess es_status_codes DLLGetOneSamplePointer(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	return GetOneSamplePointer(drvno, sample, block, camera, pdest, bytes_to_end_of_buffer);
}

/**
 * \copydoc GetOneBlockPointer
 */
DllAccess es_status_codes DLLGetOneBlockPointer(uint32_t drvno, uint32_t block, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	return GetOneBlockPointer(drvno, block, pdest, bytes_to_end_of_buffer);
}

/**
 * \copydoc GetPixelPointer
 */
DllAccess es_status_codes DLLGetPixelPointer(uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer)

{
	return GetPixelPointer(drvno, pixel, sample, block, camera, pdest, bytes_to_end_of_buffer);
}

/**
 * \brief Copies one block of pixel data of all used boards selected by settings parameter board_sel to pdest.
 *
 * \param block Selects which block to copy.
 * \param pdest0 address where data is written for board 0, should be a buffer with size: nos * camcnt * pixel * sizeof( uint16_t )
 * \param pdest1 address where data is written for board 1, should be a buffer with size: nos * camcnt * pixel * sizeof( uint16_t )
 * \param pdest2 address where data is written for board 2, should be a buffer with size: nos * camcnt * pixel * sizeof( uint16_t )
 * \param pdest3 address where data is written for board 3, should be a buffer with size: nos * camcnt * pixel * sizeof( uint16_t )
 * \param pdest4 address where data is written for board 4, should be a buffer with size: nos * camcnt * pixel * sizeof( uint16_t )
 * \return es_status_codes:
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_memory_not_initialized
 */
DllAccess es_status_codes DLLCopyOneBlock_multipleBoards(uint16_t block, uint16_t* pdest0, uint16_t* pdest1, uint16_t* pdest2, uint16_t* pdest3, uint16_t* pdest4)
{
	uint16_t* pdest[MAXPCIECARDS] = { pdest0, pdest1, pdest2, pdest3, pdest4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = CopyOneBlock(drvno, block, pdest[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

/**
 * \copydoc GetAllDataPointer
 */
DllAccess es_status_codes DLLGetAllDataPointer(uint32_t drvno, uint16_t** pdest, size_t* bytes_to_end_of_buffer)
{
	return GetAllDataPointer(drvno, pdest, bytes_to_end_of_buffer);
}

/**
 * \copydoc ExitDriver
 */
DllAccess es_status_codes DLLExitDriver()
{
	return ExitDriver();
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
\copydoc readRegisterS0_8
*/
DllAccess es_status_codes DLLreadRegisterS0_8(uint32_t drvno, uint8_t* data, uint32_t address)
{
	return readRegisterS0_8(drvno, data, address);
}

/**
 * \brief Read 1 byte of a register in S0 space of all boards selected by settings parameter board_sel.
 *
 * \param data0 Read buffer of board 0.
 * \param data1 Read buffer of board 1.
 * \param data2 Read buffer of board 2.
 * \param data3 Read buffer of board 3.
 * \param data4 Read buffer of board 4.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLreadRegisterS0_8_multipleBoards(uint8_t* data0, uint8_t* data1, uint8_t* data2, uint8_t* data3, uint8_t* data4, uint32_t address)
{
	es_status_codes status = es_no_error;
	uint8_t* data[MAXPCIECARDS] = { data0, data1, data2, data3, data4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = readRegisterS0_8(drvno, data[usedBoards], address);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Write the same 1 byte to a register in S0 space of all boards selected by settings parameter board_sel.
 *
 * \param data Data to write.
 * \param address Address of the register to write.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLwriteRegisterS0_8(uint8_t data, uint32_t address)
{
	return writeRegisterS0_8_allBoards(data, address);
}

/**
\copydoc readRegisterS0_32
*/
DllAccess es_status_codes DLLreadRegisterS0_32(uint32_t drvno, uint32_t* data, uint32_t address)
{
	return readRegisterS0_32(drvno, data, address);
}

/**
 * \brief Read 4 bytes of a register in S0 space of all boards selected by settings parameter board_sel.
 *
 * \param data0 Read buffer for board 0.
 * \param data1 Read buffer for board 1.
 * \param data2 Read buffer for board 2.
 * \param data3 Read buffer for board 3.
 * \param data4 Read buffer for board 4.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLreadRegisterS0_32_multipleBoards(uint32_t* data0, uint32_t* data1, uint32_t* data2, uint32_t* data3, uint32_t* data4, uint32_t address)
{
	es_status_codes status = es_no_error;
	uint32_t* data[MAXPCIECARDS] = { data0, data1, data2, data3, data4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = readRegisterS0_32(drvno, data[usedBoards], address);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

/**
 * \brief Write 4 bytes of a register in S0 space for all boards selected by settings parameter board_sel.
 *
 * \param data Data to write.
 * \param address Address of the register to read.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLwriteRegisterS0_32(uint32_t data, uint32_t address)
{
	return writeRegisterS0_32_allBoards(data, address);
}

/**
 * \copydoc ReadScanFrequencyBit
 */
DllAccess es_status_codes DLLReadScanFrequencyBit(uint32_t drvno, uint8_t* scanFrequencyTooHigh)
{
	return ReadScanFrequencyBit(drvno, scanFrequencyTooHigh);
}

/**
 * \brief Reads the ScanFrequency bit and checks if its high or low for all boards selected by settings parameter board_sel.
 * 
 * \param scanFrequencyTooHigh0 True when scan frequency too high bit is set for board 0
 * \param scanFrequencyTooHigh1 True when scan frequency too high bit is set for board 1
 * \param scanFrequencyTooHigh2 True when scan frequency too high bit is set for board 2
 * \param scanFrequencyTooHigh3 True when scan frequency too high bit is set for board 3
 * \param scanFrequencyTooHigh4 True when scan frequency too high bit is set for board 4
 * \return 
 */
DllAccess es_status_codes DLLReadScanFrequencyBit_multipleBoards(uint8_t* scanFrequencyTooHigh0, uint8_t* scanFrequencyTooHigh1, uint8_t* scanFrequencyTooHigh2, uint8_t* scanFrequencyTooHigh3, uint8_t* scanFrequencyTooHigh4)
{
	es_status_codes status = es_no_error;
	uint8_t* scanFrequencyTooHigh[MAXPCIECARDS] = { scanFrequencyTooHigh0, scanFrequencyTooHigh1, scanFrequencyTooHigh2, scanFrequencyTooHigh3, scanFrequencyTooHigh4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ReadScanFrequencyBit(drvno, scanFrequencyTooHigh[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

/**
 * \brief Resets the ScanFrequency bit.
 *
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
DllAccess es_status_codes DLLResetScanFrequencyBit()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ResetScanFrequencyBit(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc ReadBlockFrequencyBit
 */
DllAccess es_status_codes DLLReadBlockFrequencyBit(uint32_t drvno, uint8_t* blockFrequencyTooHigh)
{
	return ReadBlockFrequencyBit(drvno, blockFrequencyTooHigh);
}

/**
 * \brief Reads the ScanFrequency bit and checks if its high or low for all boards selected by settings parameter board_sel.
 *
 * \param blockFrequencyTooHigh0 True when block frequency too high bit is set for board 0
 * \param blockFrequencyTooHigh1 True when block frequency too high bit is set for board 1
 * \param blockFrequencyTooHigh2 True when block frequency too high bit is set for board 2
 * \param blockFrequencyTooHigh3 True when block frequency too high bit is set for board 3
 * \param blockFrequencyTooHigh4 True when block frequency too high bit is set for board 4
 * \return
 */
DllAccess es_status_codes DLLReadBlockFrequencyBit_multipleBoards(uint8_t* blockFrequencyTooHigh0, uint8_t* blockFrequencyTooHigh1, uint8_t* blockFrequencyTooHigh2, uint8_t* blockFrequencyTooHigh3, uint8_t* blockFrequencyTooHigh4)
{
	es_status_codes status = es_no_error;
	uint8_t* blockFrequencyTooHigh[MAXPCIECARDS] = { blockFrequencyTooHigh0, blockFrequencyTooHigh1, blockFrequencyTooHigh2, blockFrequencyTooHigh3, blockFrequencyTooHigh4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ReadBlockFrequencyBit(drvno, blockFrequencyTooHigh[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

/**
 * \brief Resets the BlockFrequency bit.
 *
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 * 		- es_register_write_failed
 */
DllAccess es_status_codes DLLResetBlockFrequencyBit()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ResetBlockFrequencyBit(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \copydoc GetCameraStatusOverTemp
 */
DllAccess es_status_codes DLLGetCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* overTemp)
{
	return GetCameraStatusOverTemp(drvno, sample, block, camera_pos, overTemp);
}

DllAccess es_status_codes DLLGetBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex)
{
	return GetBlockIndex(drvno, sample, block, camera_pos, blockIndex);
}

DllAccess es_status_codes DLLGetScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex)
{
	return GetScanIndex(drvno, sample, block, camera_pos, scanIndex);
}

DllAccess es_status_codes DLLGetS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return GetS1State(drvno, sample, block, camera_pos, state);
}

DllAccess es_status_codes DLLGetS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return GetS2State(drvno, sample, block, camera_pos, state);
}

DllAccess es_status_codes DLLGetImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return GetImpactSignal1(drvno, sample, block, camera_pos, impactSignal);
}

DllAccess es_status_codes DLLGetImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return GetImpactSignal2(drvno, sample, block, camera_pos, impactSignal);
}

DllAccess uint32_t DLLGetVirtualCamcnt(uint32_t drvno)
{
	return virtualCamcnt[drvno];
}

DllAccess bool DLLGetTestModeOn()
{
	return testModeOn;
}

/**
 * \brief This function returns the bit overTemp of a specific scan.
 *
 * The information over temperature is given in the special pixel camera status (pixel_camera_status) in bit pixel_camera_status_bit_over_temp.
 *
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param overTemp1 board 1: Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * \param overTemp2 board 2: Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * \param overTemp3 board 3: Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * \param overTemp4 board 4: Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * \param overTemp5 board 5: Pointer to a bool, where the information overTemp will be written. true - over temperature detected, false - no over temperature detected
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLGetCameraStatusOverTemp_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* overTemp1, uint8_t* overTemp2, uint8_t* overTemp3, uint8_t* overTemp4, uint8_t* overTemp5)
{
	es_status_codes status = es_no_error;
	uint8_t* tempArr[MAXPCIECARDS] = { overTemp1, overTemp2, overTemp3, overTemp4, overTemp5 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetCameraStatusOverTemp(drvno, sample, block, camera_pos, tempArr[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}


/**
 * \copydoc GetCameraStatusTempGood
 */
DllAccess es_status_codes DLLGetCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* tempGood)
{
	return GetCameraStatusTempGood(drvno, sample, block, camera_pos, tempGood);
}

/**
 * \brief This function returns the bit tempGood of a specific scan.
 *
 * The information temperature good is given in the special pixel camera status (pixel_camera_status) in bit pixel_camera_status_bit_temp_good. This bit is used only in cooled cameras.
 *
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param tempGood1 board 1: Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * \param tempGood2 board 2: Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * \param tempGood3 board 3: Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * \param tempGood4 board 4: Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * \param tempGood5 board 5: Pointer to a bool, where the information tempGood will be written. true - target temperature reached, false - target temperature not reached
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLGetCameraStatusTempGood_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* tempGood1, uint8_t* tempGood2, uint8_t* tempGood3, uint8_t* tempGood4, uint8_t* tempGood5)
{
	es_status_codes status = es_no_error;
	uint8_t* tempArr[MAXPCIECARDS] = { tempGood1, tempGood2, tempGood3, tempGood4, tempGood5 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetCameraStatusTempGood(drvno, sample, block, camera_pos, tempArr[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \copydoc FindCam
 */
DllAccess es_status_codes DLLFindCam(uint32_t drvno)
{
	return FindCam(drvno);
}

/**
 * \brief Test if SFP module is there and fiber is linked up.
 *
 * \param cameraFound0 true when camera is found on board 0
 * \param cameraFound1 true when camera is found on board 1
 * \param cameraFound2 true when camera is found on board 2
 * \param cameraFound3 true when camera is found on board 3
 * \param cameraFound4 true when camera is found on board 4
 * \return 
 */
DllAccess es_status_codes DLLFindCam_multipleBoards(uint8_t* cameraFound0, uint8_t* cameraFound1, uint8_t* cameraFound2, uint8_t* cameraFound3, uint8_t* cameraFound4 )
{
	es_status_codes status_return = es_no_error;
	uint8_t* cameraFound[MAXPCIECARDS] = { cameraFound0, cameraFound1, cameraFound2, cameraFound3, cameraFound4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			es_status_codes status = FindCam(drvno);
			if (status == es_no_error)
				*cameraFound[usedBoards] = true;
			else
			{
				*cameraFound[usedBoards] = false;
				status_return = status;
			}
			usedBoards++;
		}
	return status_return;
}

/**
 * \copydoc GetBlockOn
 */
DllAccess es_status_codes DLLGetBlockOn(uint32_t drvno, uint8_t* blockOn)
{
	return GetBlockOn(drvno, blockOn);
}

/**
 * \brief Get the block on bit from the PCIe flags register.
 *
 * Since the block on bit position was change in 222.14 this function looks at a different bit depending on the firmware version.
 * \param blockOn0 Pointer to a bool, where the block on bit of board 0 will be written.
 * \param blockOn1 Pointer to a bool, where the block on bit of board 1 will be written.
 * \param blockOn2 Pointer to a bool, where the block on bit of board 2 will be written.
 * \param blockOn3 Pointer to a bool, where the block on bit of board 3 will be written.
 * \param blockOn4 Pointer to a bool, where the block on bit of board 4 will be written.
 * \return es_status_codes
 */
DllAccess es_status_codes DLLGetBlockOn_multipleBoards(uint8_t* blockOn0, uint8_t* blockOn1, uint8_t* blockOn2, uint8_t* blockOn3, uint8_t* blockOn4)
{
	es_status_codes status = es_no_error;
	uint8_t* blockOn[MAXPCIECARDS] = { blockOn0, blockOn1, blockOn2, blockOn3, blockOn4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetBlockOn(drvno, blockOn[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	return status;
}

DllAccess es_status_codes DLLDumpS0Registers(uint32_t drvno, char** stringPtr)
{
	return dumpS0Registers(drvno, stringPtr);
}

DllAccess es_status_codes DLLDumpHumanReadableS0Registers(uint32_t drvno, char** stringPtr)
{
	return dumpHumanReadableS0Registers(drvno, stringPtr);
}

DllAccess es_status_codes DLLDumpDmaRegisters(uint32_t drvno, char** stringPtr)
{
	return dumpDmaRegisters(drvno, stringPtr);
}

DllAccess es_status_codes DLLDumpTlpRegisters(uint32_t drvno, char** stringPtr)
{
	return dumpTlpRegisters(drvno, stringPtr);
}

DllAccess es_status_codes DLLDumpMeasurementSettings(char** stringPtr)
{
	return dumpMeasurementSettings(stringPtr);
}

DllAccess es_status_codes DLLDumpCameraSettings(uint32_t drvno, char** stringPtr)
{
	return dumpCameraSettings(drvno, stringPtr);
}

DllAccess es_status_codes DLLDumpPciRegisters(uint32_t drvno, char** stringPtr)
{
	return dumpPciRegisters(drvno, stringPtr);
}

DllAccess es_status_codes DLLAboutDrv(uint32_t drvno, char** stringPtr)
{
#ifdef WIN32
	return _AboutDrv(drvno, stringPtr);
#else
	return es_no_error;
#endif
}

DllAccess es_status_codes DLLAboutGPX(uint32_t drvno, char** stringPtr)
{
#ifdef WIN32
	return _AboutGPX(drvno, stringPtr);
#else
	return es_no_error;
#endif
}

DllAccess void DLLGetVerifiedDataDialog(struct verify_data_parameter* vd, char** resultString)
{
	GetVerifiedDataDialog(vd, resultString);
	return;
}

DllAccess uint8_t DLLGetIsRunning()
{
	return (uint8_t)isRunning;
}

/**
 * \copydoc CalcRamUsageInMB
 */
DllAccess double DLLCalcRamUsageInMB(uint32_t nos, uint32_t nob)
{
	return CalcRamUsageInMB(nos, nob);
}

/**
 * \copydoc CalcMeasureTimeInSeconds
 */
DllAccess double DLLCalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms)
{
	return CalcMeasureTimeInSeconds(nos, nob, exposure_time_in_ms);
}

/**
 * \brief Set trigger out(Reg CtrlA:D3) for all boards selected by settings parameter board_sel. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLOutTrigHigh()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = OutTrigHigh(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \brief Reset trigger out(Reg CtrlA:D3) for all boards selected by settings parameter board_sel. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLOutTrigLow()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = OutTrigLow(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \brief Pulses trigger out(Reg CtrlA:D3) for all boards selected by setings parameter board_sel. Can be used to control timing issues in software.
 *
 * The Reg TOR:D31 must have been set to 1 and D30:D27 to zero to see the signal -> see manual
 * \param pulseWidthInMicroseconds duration of pulse in us
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLOutTrigPulse(int64_t pulseWidthInMicroseconds)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = OutTrigPulse(drvno, pulseWidthInMicroseconds);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \brief Open shutter for sensors with EC (exposure control) / sets IFC signal = high for all boards selected by settings parameter board_sel.
 *
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLOpenShutter()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = OpenShutter(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * \brief Sets the IFC bit of interface for sensors with shutter function for all boards set by settings parameter board_sel. IFC=low.
 *
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLCloseShutter()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = CloseShutter(drvno);
			if (status != es_no_error) return status;
		}
	return status;
}

/**
 * @brief Set bit to 1 in S0 register at memory address for all boards selected by settings parameter board_sel.
 *
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLsetBitS0_32(uint32_t bitnumber, uint16_t address)
{
	return setBitS0_32_allBoards(bitnumber, address);
}

/**
 * @brief Set bit to 0 in register at memory address for all boards selected by settings parameter board_sel.
 *
 * @param bitnumber 0...31, 0 is LSB, 31 MSB
 * @param address register address. Only 4 byte steps are valid.
 * @return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLresetBitS0_32(uint32_t bitnumber, uint16_t address)
{
	return resetBitS0_32_allBoards(bitnumber, address);
}

/**
 * \copydoc ReadBitS0_32
 */
DllAccess es_status_codes DLLReadBitS0_32(uint32_t drvno, uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh)
{
	return ReadBitS0_32(drvno, address, bitnumber, isBitHigh);
}

/**
 * \brief Read 1 bit of a 4 bytes s0 register for all boards.
 *
 * \param address Address of the register to read.
 * \param bitnumber Address of the bit to read.
 * \param isBitHigh0 board 0: Tells if bit is 1 or 0.
 * \param isBitHigh1 board 1: Tells if bit is 1 or 0.
 * \param isBitHigh2 board 2: Tells if bit is 1 or 0.
 * \param isBitHigh3 board 3: Tells if bit is 1 or 0.
 * \param isBitHigh4 board 4: Tells if bit is 1 or 0.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLReadBitS0_32_multipleBoards(uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh0, uint8_t* isBitHigh1, uint8_t* isBitHigh2, uint8_t* isBitHigh3, uint8_t* isBitHigh4)
{
	es_status_codes status = es_no_error;
	uint8_t* isBitHigh[MAXPCIECARDS] = { isBitHigh0, isBitHigh1, isBitHigh2, isBitHigh3, isBitHigh4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ReadBitS0_32(drvno, address, bitnumber, isBitHigh[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \copydoc ReadBitS0_8
 */
DllAccess es_status_codes DLLReadBitS0_8(uint32_t drvno, uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh)
{
	return ReadBitS0_8(drvno, address, bitnumber, isBitHigh);
}

/**
 * \brief Read 1 bit of 1 byte of a s0 register for all boards.
 *
 * \param address Address of the register to read.
 * \param bitnumber Address of the bit to read.
 * \param isBitHigh0 Tells if bit is high or low.
 * \param isBitHigh1 Tells if bit is high or low.
 * \param isBitHigh2 Tells if bit is high or low.
 * \param isBitHigh3 Tells if bit is high or low.
 * \param isBitHigh4 Tells if bit is high or low.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLReadBitS0_8_multipleBoards(uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh0, uint8_t* isBitHigh1, uint8_t* isBitHigh2, uint8_t* isBitHigh3, uint8_t* isBitHigh4)
{
	es_status_codes status = es_no_error;
	uint8_t* isBitHigh[MAXPCIECARDS] = { isBitHigh0, isBitHigh1, isBitHigh2, isBitHigh3, isBitHigh4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ReadBitS0_8(drvno, address, bitnumber, isBitHigh[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Set temperature level for cooled cameras for all boards selected by settings parameter board_sel.
 *
 * \param level level 0..7 / 0=off, 7=min -> see cooling manual
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
DllAccess es_status_codes DLLSetTemp(uint8_t level)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = SetTemp(drvno, level);
			if (status != es_no_error) return status;
		}
	return status;
}

DllAccess es_status_codes DLLSetTORReg(uint32_t drvno, uint8_t tor)
{
	return SetTORReg(drvno, tor);
}

/**
 * \brief Set signal of output port of PCIe card for all boards selected by settings parameter board_sel.
 *
 * \param tor select output signal. See [enum tor_out_t](@ref tor_out_t) in enum_settings.h for options.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 */
DllAccess es_status_codes DLLSetTORReg_multipleBoards(uint8_t tor)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = SetTORReg(drvno, tor);
			if (status != es_no_error) return status;
		}
	return status;
}

DllAccess es_status_codes DLLDAC8568_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, uint8_t reorder_channel)
{
	return DAC8568_setAllOutputs(drvno, location, cameraPosition, output, reorder_channel);
}

/**
 * \brief Sets all outputs of the DAC8568 in camera 3030 or on PCIe board for all PCIe boards.
 *
 * Use this function to set the outputs, because it is resorting the channel numeration correctly.
 * \param location Switch for the different locations of DAC85689. See [enum DAC8568_location_t](@ref DAC8568_location_t) in enum_settings.h for details.
 * \param cameraPosition This is describing the camera position when there are mumltiple cameras in line. Possible values: 0....8. This parameter is only used when location == DAC8568_camera.
 * \param output0 all output values as array for board 0 that will be converted to analog voltage (0 ... 0xFFFF)
 * \param output1 all output values as array for board 1 that will be converted to analog voltage (0 ... 0xFFFF)
 * \param output2 all output values as array for board 2 that will be converted to analog voltage (0 ... 0xFFFF)
 * \param output3 all output values as array for board 3 that will be converted to analog voltage (0 ... 0xFFFF)
 * \param output4 all output values as array for board 4 that will be converted to analog voltage (0 ... 0xFFFF)
 * \param reorder_channel used to reorder DAC channels for high speed camera
 * \return es_status_codes
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLDAC8568_setAllOutputs_multipleBoards(uint8_t location, uint8_t cameraPosition, uint32_t* output0, uint32_t* output1, uint32_t* output2, uint32_t* output3, uint32_t* output4, uint8_t reorder_channel)
{
	es_status_codes status = es_no_error;
	uint32_t* output[MAXPCIECARDS] = { output0, output1, output2, output3, output4 };
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
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
DllAccess void DLLFreeMemInfo(uint64_t* pmemory_all, uint64_t* pmemory_free)
{
	FreeMemInfo(pmemory_all, pmemory_free);
	return;
}


/**
 * \copydoc isMeasureOn
 */
DllAccess es_status_codes DLLisMeasureOn(uint32_t drvno, uint8_t* measureOn)
{
	return isMeasureOn(drvno, measureOn);
}

/**
 * \brief Check if measure on bit is set for all boards selected by settings parameter board_sel.
 *
 * \param measureOn0 True when measureon bit is set in board 0.
 * \param measureOn1 True when measureon bit is set in board 1.
 * \param measureOn2 True when measureon bit is set in board 2.
 * \param measureOn3 True when measureon bit is set in board 3.
 * \param measureOn4 True when measureon bit is set in board 4.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLisMeasureOn_multipleBoards(uint8_t* measureOn0, uint8_t* measureOn1, uint8_t* measureOn2, uint8_t* measureOn3, uint8_t* measureOn4)
{
	es_status_codes status = es_no_error;
	uint8_t* measureOn[MAXPCIECARDS] = { measureOn0, measureOn1, measureOn2, measureOn3, measureOn4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = isMeasureOn(drvno, measureOn[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \copydoc isBlockOn
 */
DllAccess es_status_codes DLLisBlockOn(uint32_t drvno, uint8_t* blockOn)
{
	return isBlockOn(drvno, blockOn);
}

/**
 * \brief Check if blockon bit is set for all boards selected by settings parameter board_sel.
 *
 * \param blockOn0 True when blockon bit is set in board 0.
 * \param blockOn1 True when blockon bit is set in board 1.
 * \param blockOn2 True when blockon bit is set in board 2.
 * \param blockOn3 True when blockon bit is set in board 3.
 * \param blockOn4 True when blockon bit is set in board 4.
 *  \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLisBlockOn_multipleBoards(uint8_t* blockOn0, uint8_t* blockOn1, uint8_t* blockOn2, uint8_t* blockOn3, uint8_t* blockOn4)
{
	es_status_codes status = es_no_error;
	uint8_t* blockOn[MAXPCIECARDS] = { blockOn0, blockOn1, blockOn2, blockOn3, blockOn4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = isBlockOn(drvno, blockOn[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Returns when measure on bit is 0 in all boards selected by settings parameter board_sel.
 *
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLwaitForMeasureReady()
{
	return WaitForMeasureReady(settings_struct.board_sel);
}

/**
 * \brief Returns when block on bit is 0 in all boards selected by settings parameter board_sel.
 *
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLwaitForBlockReady()
{
	return waitForBlockReady(settings_struct.board_sel);
}

/**
 * \copydoc SetSTimer
 */
DllAccess es_status_codes DLLSetSTimer(uint32_t drvno, uint32_t stime_in_microseconds)
{
	return SetSTimer(drvno, stime_in_microseconds);
}

/**
 * \copydoc SetBTimer
 */
DllAccess es_status_codes DLLSetBTimer(uint32_t drvno, uint32_t btime_in_microseconds)
{
	return SetBTimer(drvno, btime_in_microseconds);
}

/**
 * \copydoc GetXckLength
 */
DllAccess es_status_codes DLLGetXckLength(uint32_t drvno, uint32_t* xckLengthIn10ns)
{
	return GetXckLength(drvno, xckLengthIn10ns);
}

/**
 * \copydoc GetXckPeriod
 */
DllAccess es_status_codes DLLGetXckPeriod(uint32_t drvno, uint32_t* xckPeriodIn10ns)
{
	return GetXckPeriod(drvno, xckPeriodIn10ns);
}

/**
 * \copydoc GetBonLength
 */
DllAccess es_status_codes DLLGetBonLength(uint32_t drvno, uint32_t* bonLengthIn10ns)
{
	return GetBonLength(drvno, bonLengthIn10ns);
}

/**
 * \copydoc GetBonPeriod
 */
DllAccess es_status_codes DLLGetBonPeriod(uint32_t drvno, uint32_t* bonPeriodIn10ns)
{
	return GetBonPeriod(drvno, bonPeriodIn10ns);
}

/**
 * \brief Get the high time duration of XCK from the S0 register XCKLEN.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the first completed XCK.
 * The value range is:
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * \param xckLengthIn10ns0 pointer to uint32 where the XCK length of board0 is returned
 * \param xckLengthIn10ns1 pointer to uint32 where the XCK length of board1 is returned
 * \param xckLengthIn10ns2 pointer to uint32 where the XCK length of board2 is returned
 * \param xckLengthIn10ns3 pointer to uint32 where the XCK length of board3 is returned
 * \param xckLengthIn10ns4 pointer to uint32 where the XCK length of board4 is returned

 * \return es_status_codes
 */
DllAccess es_status_codes DLLGetXckLength_multipleBoards(uint32_t* xckLengthIn10ns0, uint32_t* xckLengthIn10ns1, uint32_t* xckLengthIn10ns2, uint32_t* xckLengthIn10ns3, uint32_t* xckLengthIn10ns4)
{
	es_status_codes status = es_no_error;
	uint32_t* xckLengthIn10ns[MAXPCIECARDS] = { xckLengthIn10ns0, xckLengthIn10ns1, xckLengthIn10ns2, xckLengthIn10ns3, xckLengthIn10ns4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetXckLength(drvno, xckLengthIn10ns[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Get pos edge to pos egde time of XCK time from the S0 register XCKPERIOD.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the start of the second XCK.
 * The value range is:
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * \param xckPeriodIn10ns0 pointer to uint32 where the XCK period of board0 is returned
 * \param xckPeriodIn10ns1 pointer to uint32 where the XCK period of board1 is returned
 * \param xckPeriodIn10ns2 pointer to uint32 where the XCK period of board2 is returned
 * \param xckPeriodIn10ns3 pointer to uint32 where the XCK period of board3 is returned
 * \param xckPeriodIn10ns4 pointer to uint32 where the XCK period of board4 is returned

 * \return es_status_codes
 */
DllAccess es_status_codes DLLGetXckPeriod_multipleBoards(uint32_t* xckPeriodIn10ns0, uint32_t* xckPeriodIn10ns1, uint32_t* xckPeriodIn10ns2, uint32_t* xckPeriodIn10ns3, uint32_t* xckPeriodIn10ns4)
{
	es_status_codes status = es_no_error;
	uint32_t* xckPeriodIn10ns[MAXPCIECARDS] = { xckPeriodIn10ns0, xckPeriodIn10ns1, xckPeriodIn10ns2, xckPeriodIn10ns3, xckPeriodIn10ns4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetXckPeriod(drvno, xckPeriodIn10ns[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Get the high time duration of BON from the S0 register BONLEN.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the first completed BON.
 * The value range is:
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * \param bonLengthIn10ns0 pointer to uint32 where the BON length of board0 is returned
 * \param bonLengthIn10ns1 pointer to uint32 where the BON length of board1 is returned
 * \param bonLengthIn10ns2 pointer to uint32 where the BON length of board2 is returned
 * \param bonLengthIn10ns3 pointer to uint32 where the BON length of board3 is returned
 * \param bonLengthIn10ns4 pointer to uint32 where the BON length of board4 is returned

 * \return es_status_codes
 */
DllAccess es_status_codes DLLGetBonLength_multipleBoards(uint32_t* bonLengthIn10ns0, uint32_t* bonLengthIn10ns1, uint32_t* bonLengthIn10ns2, uint32_t* bonLengthIn10ns3, uint32_t* bonLengthIn10ns4)
{
	es_status_codes status = es_no_error;
	uint32_t* bonLengthIn10ns[MAXPCIECARDS] = { bonLengthIn10ns0, bonLengthIn10ns1, bonLengthIn10ns2, bonLengthIn10ns3, bonLengthIn10ns4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetBonLength(drvno, bonLengthIn10ns[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Get the pos edge to pos edge time of BON from the S0 register BONPERIOD.
 *
 * The signal is measured once per measurement. The fist valid value can be read after the start of the second BON.
 * The value range is:
 *		* min: 0
 *		* step: 1 => 10 ns
 *		* max: 0xFFFFFFFF = 4,294,967,295 => 42,949,672,950 ns
 * \param bonPeriodIn10ns0 pointer to uint32 where the BON period of board0 is returned
 * \param bonPeriodIn10ns1 pointer to uint32 where the BON period of board1 is returned
 * \param bonPeriodIn10ns2 pointer to uint32 where the BON period of board2 is returned
 * \param bonPeriodIn10ns3 pointer to uint32 where the BON period of board3 is returned
 * \param bonPeriodIn10ns4 pointer to uint32 where the BON period of board4 is returned

 * \return es_status_codes
 */
DllAccess es_status_codes DLLGetBonPeriod_multipleBoards(uint32_t* bonPeriodIn10ns0, uint32_t* bonPeriodIn10ns1, uint32_t* bonPeriodIn10ns2, uint32_t* bonPeriodIn10ns3, uint32_t* bonPeriodIn10ns4)
{
	es_status_codes status = es_no_error;
	uint32_t* bonPeriodIn10ns[MAXPCIECARDS] = { bonPeriodIn10ns0, bonPeriodIn10ns1, bonPeriodIn10ns2, bonPeriodIn10ns3, bonPeriodIn10ns4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetBonPeriod(drvno, bonPeriodIn10ns[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
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
DllAccess void DLLRegisterLVEvents(LVUserEventRef* measureStartEvent, LVUserEventRef* measureDoneEvent, LVUserEventRef* blockStartEvent, LVUserEventRef* blockDoneEvent, LVUserEventRef* allBlocksDoneEvent)
{
	measureStartLVEvent = *measureStartEvent;
	measureDoneLVEvent = *measureDoneEvent;
	blockStartLVEvent = *blockStartEvent;
	blockDoneLVEvent = *blockDoneEvent;
	allBlocksDoneLVEvent = *allBlocksDoneEvent;
	return;
}
#endif

/**
 * \copydoc ConvertErrorCodeToMsg
 */
DllAccess char* DLLConvertErrorCodeToMsg(es_status_codes status)
{
	return ConvertErrorCodeToMsg(status);
}

/**
 * \copydoc IOCtrl_setOutput
 */
DllAccess es_status_codes DLLIOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	return IOCtrl_setOutput(drvno, number, width_in_5ns, delay_in_5ns);
}

DllAccess es_status_codes DLLIOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns)
{
	return IOCtrl_setT0(drvno, period_in_10ns);
}

/**
 * \brief Set period of IOCtrl pulse outputs base frequency T0 for all boards selected by settings parameter board_sel.
 *
 * \param period_in_10ns Period of T0 in 10ns steps.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_camera_not_found
 */
DllAccess es_status_codes DLLIOCtrl_setT0_multipleBoards(uint32_t period_in_10ns)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = IOCtrl_setT0(drvno, period_in_10ns);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \brief Set parameters of all pulses outputs of IOCTRL for all boards selected by settings parameter board_sel.
 *
 * \param width_in_5ns Set width of pulse in 5ns steps. Array with 7 entries.
 * \param delay_in_5ns Set delay of pulse in 5ns steps. Array with 7 entries.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLIOCtrl_setAllOutputs(uint32_t* width_in_5ns, uint32_t* delay_in_5ns)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = IOCtrl_setAllOutputs(drvno, width_in_5ns, delay_in_5ns);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc GetCurrentScanNumber
 */
DllAccess void DLLGetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block)
{
	GetCurrentScanNumber(drvno, sample, block);
	return;
}

/**
 * \brief Gives scan and block number of the last scan written to userBuffer for all boards selected by settings paramter board_sel.
 *
 * When settings parameter USE_SOFTWARE_POLLING is true this function converts scanCounterTotal to scan and block.
 * This is necessary, because scanCounterTotal is just counting each scan not regarding camcnt and blocks.
 * When USE_SOFTWARE_POLLING is false the scan and block number of the last interrupt is given.
 *
 * \param sample0 Scan number of the last scan in userBuffer of board 0. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * \param sample1 Scan number of the last scan in userBuffer of board 1. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * \param sample2 Scan number of the last scan in userBuffer of board 2. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * \param sample3 Scan number of the last scan in userBuffer of board 3. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * \param sample4 Scan number of the last scan in userBuffer of board 4. -1 when no scan has been written yet, otherwise 0...(nos-1)
 * \param block0 Block number of the last scan in userBuffer of board 0. -1 when no scans has been written yet, otherwise 0...(nob-1)
 * \param block1 Block number of the last scan in userBuffer of board 1. -1 when no scans has been written yet, otherwise 0...(nob-1)
 * \param block2 Block number of the last scan in userBuffer of board 2. -1 when no scans has been written yet, otherwise 0...(nob-1)
 * \param block3 Block number of the last scan in userBuffer of board 3. -1 when no scans has been written yet, otherwise 0...(nob-1)
 * \param block4 Block number of the last scan in userBuffer of board 4. -1 when no scans has been written yet, otherwise 0...(nob-1)
 */
DllAccess void DLLGetCurrentScanNumber_multipleBoards(int64_t* sample0, int64_t* block0, int64_t* sample1, int64_t* block1, int64_t* sample2, int64_t* block2, int64_t* sample3, int64_t* block3, int64_t* sample4, int64_t* block4)
{
	uint64_t* sample[MAXPCIECARDS] = { sample0, sample1, sample2, sample3, sample4 };
	uint64_t* block[MAXPCIECARDS] = { block0, block1, block2, block3, block4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			GetCurrentScanNumber(drvno, sample[usedBoards], block[usedBoards]);
			usedBoards++;
		}
	return;
}

/**
 * \copydoc GetIsTdc
 */
DllAccess es_status_codes DLLGetIsTdc(uint32_t drvno, uint8_t* isTdc)
{
	return GetIsTdc(drvno, isTdc);
}

/**
 * \brief Read TDC flag in PCIEFLAGS register of all boards selected by settings parameter board_sel.
 *
 * \param isTdc0 TDC flag of board 0. 1: TDC board detected, 0: no TDC board detected
 * \param isTdc1 TDC flag of board 1. 1: TDC board detected, 0: no TDC board detected
 * \param isTdc2 TDC flag of board 2. 1: TDC board detected, 0: no TDC board detected
 * \param isTdc3 TDC flag of board 3. 1: TDC board detected, 0: no TDC board detected
 * \param isTdc4 TDC flag of board 4. 1: TDC board detected, 0: no TDC board detected
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLGetIsTdc_multipleBoards(uint8_t* isTdc0, uint8_t* isTdc1, uint8_t* isTdc2, uint8_t* isTdc3, uint8_t* isTdc4)
{
	uint8_t* isTdc[MAXPCIECARDS] = { isTdc0, isTdc1, isTdc2, isTdc3, isTdc4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetIsTdc(drvno, isTdc[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \copydoc GetIsDsc
 */
DllAccess es_status_codes DLLGetIsDsc(uint32_t drvno, uint8_t* isDsc)
{
	return GetIsDsc(drvno, isDsc);
}

/**
 * \brief Read DSC flag in PCIEFLAGS register for all boards selected by settings parameter board sel.
 *
 * \param isDsc0 DSC flag of board 0. 1: DSC board detected, 0: no DSC board detected
 * \param isDsc1 DSC flag of board 1. 1: DSC board detected, 0: no DSC board detected
 * \param isDsc2 DSC flag of board 2. 1: DSC board detected, 0: no DSC board detected
 * \param isDsc3 DSC flag of board 3. 1: DSC board detected, 0: no DSC board detected
 * \param isDsc4 DSC flag of board 4. 1: DSC board detected, 0: no DSC board detected
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 */
DllAccess es_status_codes DLLGetIsDsc_multipleBoards(uint8_t* isDsc0, uint8_t* isDsc1, uint8_t* isDsc2, uint8_t* isDsc3, uint8_t* isDsc4)
{
	uint8_t* isDsc[MAXPCIECARDS] = { isDsc0, isDsc1, isDsc2, isDsc3, isDsc4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetIsDsc(drvno, isDsc[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

DllAccess es_status_codes DLLResetDSC(uint32_t drvno, uint8_t DSCNumber)
{
	return ResetDSC(drvno, DSCNumber);
}

/**
 * @brief reset Delay Stage Counter for all boards selected by settings parameter board_sel.
 *
 * @param DSCNumber 1: DSC 1; 2: DSC 2
 * @return es_status_codes
 */
DllAccess es_status_codes DLLResetDSC_multipleBoards(uint8_t DSCNumber)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ResetDSC(drvno, DSCNumber);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

DllAccess es_status_codes DLLSetDIRDSC(uint32_t drvno, uint8_t DSCNumber, uint8_t dir)
{
	return SetDIRDSC(drvno, DSCNumber, dir);
}

/**
 * @brief set direction of Delay Stage Counter for all boards selected by settings parameter board_sel.
 *
 * @param DSCNumber 1: DSC 1; 2: DSC 2
 * @param dir true: up; false: down
 * @return es_status_codes
 */
DllAccess es_status_codes DLLSetDIRDSC_multipleBoards(uint8_t DSCNumber, uint8_t dir)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
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
DllAccess es_status_codes DLLGetDSC(uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC)
{
	return GetDSC(drvno, DSCNumber, ADSC, LDSC);
}

/**
 * \brief return all values of Delay Stage Counter for all boards selected by settings parameter board_sel
 *
 * \param DSCNumber 1: DSC 1; 2: DSC 2
 * \param ADSC0 current DSC of board 0
 * \param ADSC1 current DSC of board 1
 * \param ADSC2 current DSC of board 2
 * \param ADSC3 current DSC of board 3
 * \param ADSC4 current DSC of board 4
 * \param LDSC0 last DSC of board 0
 * \param LDSC1 last DSC of board 1
 * \param LDSC2 last DSC of board 2
 * \param LDSC3 last DSC of board 3
 * \param LDSC4 last DSC of board 4
 * \return es_status_codes
 */
DllAccess es_status_codes DLLGetDSC_multipleBoards(uint8_t DSCNumber, uint32_t* ADSC0, uint32_t* LDSC0, uint32_t* ADSC1, uint32_t* LDSC1, uint32_t* ADSC2, uint32_t* LDSC2, uint32_t* ADSC3, uint32_t* LDSC3, uint32_t* ADSC4, uint32_t* LDSC4)
{
	es_status_codes status = es_no_error;
	uint32_t* ADSC[MAXPCIECARDS] = { ADSC0, ADSC1, ADSC2, ADSC3, ADSC4 };
	uint32_t* LDSC[MAXPCIECARDS] = { LDSC0, LDSC1, LDSC2, LDSC3, LDSC4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetDSC(drvno, DSCNumber, ADSC[usedBoards], LDSC[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief Initialize the TDC-GPX chip for all boards selected by settings parameter board_sel. TDC: time delay counter option.
 *
 * \param delay GPX offset is used to increase accuracy. A counter value can be added, usually 1000.
 * \return es_status_codes:
 *		- es_no_error
 *		- es_register_read_failed
 *		- es_register_write_failed
 * register dump with _AboutGPX() but with error status
 */
DllAccess es_status_codes DLLInitGPX(uint32_t delay)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = InitGPX(drvno, delay);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

/**
 * \copydoc SetContinuousMeasurement
 */
DllAccess void DLLSetContinuousMeasurement(uint8_t on)
{
	SetContinuousMeasurement(on);
	return;
}

/**
 * \copydoc GetAllSpecialPixelInformation
 */
DllAccess es_status_codes DLLGetAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	return GetAllSpecialPixelInformation(drvno, sample, block, camera_pos, sp);
}

/**
 * \brief This function returns the all special pixel information of a specific scan.
 *
 * The information impact signal 2 is given in the special pixels pixel_impact_signal_2_low and pixel_impact_signal_2_high. Impact signal 2 is either TDC 2 or DSC 2, depending on the PCIe daughter board.
 *
 * \param sample sample number (0 ... (nos-1))
 * \param block block number (0 ... (nob-1))
 * \param camera_pos camera position (0 ... (CAMCNT-1))
 * \param sp0 struct special_pixels for board 0. Pointer to struct special_pixel, where all special pixel information will be written.
 * \param sp1 struct special_pixels for board 1. Pointer to struct special_pixel, where all special pixel information will be written.
 * \param sp2 struct special_pixels for board 2. Pointer to struct special_pixel, where all special pixel information will be written.
 * \param sp3 struct special_pixels for board 3. Pointer to struct special_pixel, where all special pixel information will be written.
 * \param sp4 struct special_pixels for board 4. Pointer to struct special_pixel, where all special pixel information will be written.
 * \return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_parameter_out_of_range
 */
DllAccess es_status_codes DLLGetAllSpecialPixelInformation_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp0, struct special_pixels* sp1, struct special_pixels* sp2, struct special_pixels* sp3, struct special_pixels* sp4)
{
	struct special_pixels* sp[MAXPCIECARDS] = { sp0, sp1, sp2, sp3, sp4 };
	es_status_codes status = es_no_error;
	uint32_t usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetAllSpecialPixelInformation(drvno, sample, block, camera_pos, sp[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
 * \brief This function inserts data to user buffer for developing purpose.
 */
DllAccess void DLLFillUserBufferWithDummyData()
{
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
			FillUserBufferWithDummyData(drvno);
	}
	return;
}

#ifndef MINIMAL_BUILD

/**
 * \copydoc CalcTrms
 */
DllAccess es_status_codes DLLCalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double* mwf, double* trms)
{
	return CalcTrms(drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf, trms);
}

/**
 * \brief Calculate TRMS noise value of one pixel for all used boards.
 *
 * Calculates RMS of TRMS_pixel in the range of samples from firstSample to lastSample. Only calculates RMS from one block. Boards set by settings parameter board_sel are used.
 * \param firstSample start sample to calculate RMS. 0...(nos-2). Typical value: 10, to skip overexposed first samples
 * \param lastSample last sample to calculate RMS. firstSample+1...(nos-1).
 * \param TRMS_pixel pixel for calculating noise (0...(PIXEL-1))
 * \param CAMpos index for camcount (0...(CAMCNT-1))
 * \param mwf0 pointer for mean value for board 0
 * \param trms0 pointer for noise for board 0
 * \param mwf1 pointer for mean value for board 1
 * \param trms1 pointer for noise for board 1
 * \param mwf2 pointer for mean value for board 2
 * \param trms2 pointer for noise for board 2
 * \param mwf3 pointer for mean value for board 3
 * \param trms3 pointer for noise for board 3
 * \param mwf4 pointer for mean value for board 4
 * \param trms4 pointer for noise for board 4
 * \return es_status_codes
 *		- es_no_error
 *		- es_parameter_out_of_range
 *		- es_allocating_memory_failed
 *		- es_memory_not_initialized
 */
DllAccess es_status_codes DLLCalcTrms_multipleBoards(uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double* mwf0, double* trms0, double* mwf1, double* trms1, double* mwf2, double* trms2, double* mwf3, double* trms3, double* mwf4, double* trms4)
{
	es_status_codes status = es_no_error;
	double* mwf[MAXPCIECARDS] = { mwf0, mwf1, mwf2, mwf3, mwf4 };
	double* trms[MAXPCIECARDS] = { trms0, trms1, trms2, trms3, trms4 };
	int usedBoards = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = CalcTrms(drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf[usedBoards], trms[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

/**
\copydoc ErrMsgBoxOn
*/
DllAccess void DLLErrMsgBoxOn()
{
#ifdef WIN32
	ErrMsgBoxOn();
#endif
	return;
}

/**
\copydoc ErrMsgBoxOff
*/
DllAccess void DLLErrMsgBoxOff()
{
#ifdef WIN32
	ErrMsgBoxOff();
#endif
	return;
}


/**
 * \copydoc ErrorMsg
 */
DllAccess void DLLErrorMsg(char ErrMsg[20])
{
#ifdef WIN32
	ErrorMsg(ErrMsg);
#endif
	return;
}

/**
 * \copydoc About
 */
DllAccess es_status_codes DLLAbout()
{
#ifdef WIN32
	return About(settings_struct.board_sel);
#else
	return es_no_error;
#endif
}

/**
* \copydoc Start2dViewer
*/
DllAccess void DLLStart2dViewer(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos)
{
#ifdef WIN32
	Start2dViewer(drvno, block, camera, pixel, nos);
#endif
	return;
}

/**
* \copydoc ShowNewBitmap
*/
DllAccess void DLLShowNewBitmap(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos)
{
#ifdef WIN32
	ShowNewBitmap(drvno, block, camera, pixel, nos);
#endif
	return;
}

/**
* \copydoc Deinit2dViewer
*/
DllAccess void DLLDeinit2dViewer()
{
#ifdef WIN32
	Deinit2dViewer();
#endif
	return;
}

/**
* \copydoc SetGammaValue
*/
DllAccess void DLLSetGammaValue(uint16_t white, uint16_t black)
{
#ifdef WIN32
	SetGammaValue(white, black);
#endif
}

/**
* \copydoc GetGammaWhite
*/
DllAccess uint16_t DLLGetGammaWhite()
{
#ifdef WIN32
	return GetGammaWhite();
#else
	return 0;
#endif
}

/**
* \copydoc GetGammaBlack
*/
DllAccess uint16_t DLLGetGammaBlack()
{
#ifdef WIN32
	return GetGammaBlack();
#else
	return 0;
#endif
}

#endif

/**
* \copydoc GetScanTriggerDetected
*/
DllAccess es_status_codes DLLGetScanTriggerDetected(uint32_t drvno, uint8_t* detected)
{
	return GetScanTriggerDetected(drvno, detected);
}

/**
* \copydoc GetBlockTriggerDetected
*/
DllAccess es_status_codes DLLGetBlockTriggerDetected(uint32_t drvno, uint8_t* detected)
{
	return GetBlockTriggerDetected(drvno, detected);
}

/**
* \copydoc ResetScanTriggerDetected
*/
DllAccess es_status_codes DLLResetScanTriggerDetected(uint32_t drvno)
{
	return ResetScanTriggerDetected(drvno);
}

/**
* \copydoc ResetBlockTriggerDetected
*/
DllAccess es_status_codes DLLResetBlockTriggerDetected(uint32_t drvno)
{
	return ResetBlockTriggerDetected(drvno);
}

DllAccess es_status_codes DLLGetScanTriggerDetected_multipleBoards(uint8_t* detected0, uint8_t* detected1, uint8_t* detected2, uint8_t* detected3, uint8_t* detected4)
{
	uint8_t* detected[MAXPCIECARDS] = { detected0, detected1, detected2, detected3, detected4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetScanTriggerDetected(drvno, detected[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

DllAccess es_status_codes DLLGetBlockTriggerDetected_multipleBoards(uint8_t* detected0, uint8_t* detected1, uint8_t* detected2, uint8_t* detected3, uint8_t* detected4)
{
	uint8_t* detected[MAXPCIECARDS] = { detected0, detected1, detected2, detected3, detected4 };
	int usedBoards = 0;
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = GetBlockTriggerDetected(drvno, detected[usedBoards]);
			if (status != es_no_error) return status;
			usedBoards++;
		}
	}
	return status;
}

DllAccess es_status_codes DLLResetScanTriggerDetected_multipleBoards()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ResetScanTriggerDetected(drvno);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

DllAccess es_status_codes DLLResetBlockTriggerDetected_multipleBoards()
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((settings_struct.board_sel >> drvno) & 1)
		{
			status = ResetBlockTriggerDetected(drvno);
			if (status != es_no_error) return status;
		}
	}
	return status;
}

DllAccess es_status_codes DLLDAC8568_setOutput(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output)
{
	return DAC8568_setOutput(drvno, location, cameraPosition, channel, output);
}

DllAccess es_status_codes DLLCheckFifoValid(uint32_t drvno, bool* valid)
{
	return CheckFifoValid(drvno, valid);
}

DllAccess es_status_codes DLLCheckFifoOverflow(uint32_t drvno, bool* overflow)
{
	return CheckFifoOverflow(drvno, overflow);
}

DllAccess es_status_codes DLLCheckFifoEmpty(uint32_t drvno, bool* empty)
{
	return CheckFifoEmpty(drvno, empty);
}

DllAccess es_status_codes DLLCheckFifoFull(uint32_t drvno, bool* full)
{
	return CheckFifoFull(drvno, full);
}

DllAccess es_status_codes DLLExportMeasurementHDF5(const char* path, char* filename)
{
	return ExportMeasurementHDF5(path, filename);
}

DllAccess void DLLSetMeasureStartHook(void(*hook)())
{
	measureStartHook = hook;
}

DllAccess void DLLSetMeasureDoneHook(void(*hook)())
{
	measureDoneHook = hook;
}

DllAccess void DLLSetBlockStartHook(void(*hook)())
{
	blockStartHook = hook;
}

DllAccess void DLLSetBlockDoneHook(void(*hook)())
{
	blockDoneHook = hook;
}

DllAccess void DLLSetAllBlocksDoneHook(void(*hook)())
{
	allBlocksDoneHook = hook;
}
