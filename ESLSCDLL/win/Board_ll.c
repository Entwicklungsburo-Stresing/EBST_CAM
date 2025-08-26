/*****************************************************************//**
 * @file   Board_ll.c
 * @copydoc Board_ll.h
 *********************************************************************/

#include "../Board_ll.h"
#include <stdint.h>
#include "../Board.h"
#include <process.h>
#include <io.h>
#include "../../version.h"
#include "../Direct2dViewer_c.h"

volatile DWORD dmaBufferSizeInBytes[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
uint16_t* dmaBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
volatile uint64_t IsrCounter[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
volatile UINT8 dmaBufferPartReadPos[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
bool _SHOW_MSG = TRUE;
HANDLE ghMutex[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
HANDLE registerReadWriteMutex[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
HANDLE registerReadWriteMutexHighLevel[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
HANDLE mutexUserBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
FILE* file_stream[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
void* Direct2dViewer = NULL;
uint16_t* greyscale_data = NULL;
LARGE_INTEGER freq;

/**
 * @brief
 *
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes CleanupDma(uint32_t drvno)
{
	dmaBuffer[drvno] = NULL;
	return lscpciej_CleanupDma(drvno);
}

/**
 * @brief This interrupt routine copies data from the DMA buffer to the user buffer. If write to disk is true, the data is also written to disc.
 * 
 * This routine is called every DMASPERINTR=500 scans. The size of a drivers contiguous memory is limited, so we must copy the data from a small buffer to a bigger buffer. The copy process is split in lower/upper half blocks.
 * 
 * @param drvno PCIe board identifier.
 */
void isr( uint32_t drvno )
{
	ES_TRACE( "ISR Counter drvno %"PRIu32": %"PRIu64"\n", drvno, IsrCounter[drvno]);
	// Set INTRSR flag for TRIGO
	es_status_codes status = setBitS0_32(drvno, IRQREG_bitindex_INTRSR, S0Addr_IRQREG );
	if (status != es_no_error) return;
	// Be sure not to stop run before last ISR is ready or last part is truncated.
	// Usually dmaBufferSizeInBytes = 1000scans 
	// Sometimes (all 10 minutes) one INTR more occurs -> just do not serve it and return
	// Error when too much ISRs -> memcpy out of range
	if (IsrCounter[drvno] >= numberOfInterrupts[drvno])
	{
		ES_LOG( "numberOfInterrupts: %"PRIu32" \n", numberOfInterrupts[drvno]);
		ES_LOG( "ISR Counter overflow: %"PRIu64" \n", IsrCounter[drvno]);
		status = resetBitS0_32( drvno, IRQREG_bitindex_INTRSR, S0Addr_IRQREG );//reset INTRSR flag for TRIGO
		return;
	}
	//ES_TRACE("dmaBufferSizeInBytes: 0x%x \n", dmaBufferSizeInBytes);
	// dmaBufferPartSizeInBytes = 1000 * pixel *2 -> /2 = 500 scans = 1088000 bytes
	// that means one 500 scan copy block has 1088000 bytes
	size_t dmaBufferPartSizeInBytes = dmaBufferSizeInBytes[drvno] / DMA_BUFFER_PARTS;
	uint16_t* dmaBufferReadPos = dmaBuffer[drvno] + dmaBufferPartReadPos[drvno] * dmaBufferPartSizeInBytes / sizeof(uint16_t);
	// The copy process is done here
	ES_TRACE("copy from DMA 0x%p to userBufferWritePos 0x%p \n", dmaBufferReadPos, userBufferWritePos[drvno]);
	memcpy( userBufferWritePos[drvno], dmaBufferReadPos, dmaBufferPartSizeInBytes );
	manipulateData(drvno, userBufferWritePos[drvno], (uint32_t)(dmaBufferPartSizeInBytes / (sizeof(uint16_t) * settings_struct.camera_settings[drvno].pixel) ));
	dmaBufferPartReadPos[drvno]++;
	// number of ISR per dmaBuf - 1
	if (dmaBufferPartReadPos[drvno] >= DMA_BUFFER_PARTS)
		// dmaBufferPartReadPos is 0 or 1 for buffer divided in 2 parts
		dmaBufferPartReadPos[drvno] = 0;
	userBufferWritePos[drvno] += dmaBufferPartSizeInBytes / sizeof( uint16_t );
	data_available[drvno] += dmaBufferPartSizeInBytes / sizeof(uint16_t);
	ES_TRACE("increase userBufferWritePos to 0x%p \n", userBufferWritePos[drvno]);
	ES_TRACE("increase data_available to %zu \n", data_available[drvno]);
	// Reset INTRSR flag for TRIGO
	status = resetBitS0_32(drvno, IRQREG_bitindex_INTRSR, S0Addr_IRQREG );
	IsrCounter[drvno]++;
	if (IsrCounter[drvno] >= numberOfInterrupts[drvno])
	{
		ES_TRACE("set allInterruptsDone to true\n");
		allInterruptsDone[drvno] = true;
	}
	return;
}

void __stdcall interrupt_handler0(void) { isr( 0 ); }
void __stdcall interrupt_handler1(void) { isr( 1 ); }
void __stdcall interrupt_handler2(void) { isr( 2 ); }
void __stdcall interrupt_handler3(void) { isr( 3 ); }
void __stdcall interrupt_handler4(void) { isr( 4 ); }
LSCPCIEJ_INT_HANDLER_STUB interrupt_handler_array[MAXPCIECARDS] = { &interrupt_handler0, &interrupt_handler1, &interrupt_handler2, &interrupt_handler3, &interrupt_handler4 };

/**
 * @brief Reads 4 bytes on DMA area.
 *
 * @param drvno PCIe board identifier
 * @param data buffer for data
 * @param address Offset from BaseAdress - in Bytes ! 0..3= Regs of Board.
 * @return @ref es_status_codes
 */
es_status_codes readRegister_32(uint32_t drvno, uint32_t* data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	WaitForSingleObject(registerReadWriteMutex[drvno], INFINITE);
	status = lscpciej_readRegister_32(drvno, data, address);
	ReleaseMutex(registerReadWriteMutex[drvno]);
	return status;
};

/**
 * @brief Reads long on DMA area.
 *
 * @param drvno PCIe board identifier
 * @param data buffer for data
 * @param address Offset from BaseAdress - in Bytes ! 0..3= Regs of Board.
 * @return @ref es_status_codes
 */
es_status_codes readRegister_16(uint32_t drvno, uint16_t* data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	WaitForSingleObject(registerReadWriteMutex[drvno], INFINITE);
	status = lscpciej_readRegister_16(drvno, data, address);
	ReleaseMutex(registerReadWriteMutex[drvno]);
	return status;
};

/**
 * @brief Read byte (8 bit) from register of PCIe board, except r10-r1f.
 *
 * @param drvno board number (=0 if one PCI board)
 * @param data pointer to where data is stored
 * @param address offset of register from base address (count in bytes)
 * @return @ref es_status_codes
 */
es_status_codes readRegister_8(uint32_t drvno, uint8_t* data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	WaitForSingleObject(registerReadWriteMutex[drvno], INFINITE);
	status = lscpciej_readRegister_8(drvno, data, address);
	ReleaseMutex(registerReadWriteMutex[drvno]);
	return status;
};

/**
 * @brief Writes 32 bits (4 bytes) to register.
 *
 * @param drvno PCIe board identifier.
 * @param data data to write
 * @param address Register offset from BaseAdress - in bytes
 * @return @ref es_status_codes
 */
es_status_codes writeRegister_32(uint32_t drvno, uint32_t data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	WaitForSingleObject(registerReadWriteMutex[drvno], INFINITE);
	status = lscpciej_writeRegister_32(drvno, data, address);
	ReleaseMutex(registerReadWriteMutex[drvno]);
	return status;
};

/**
 * @brief Writes 16 bits (2 bytes) to register.
 *
 * @param drvno PCIe board identifier.
 * @param data data to write
 * @param address Register offset from BaseAdress - in bytes
 * @return @ref es_status_codes
 */
es_status_codes writeRegister_16(uint32_t drvno, uint16_t data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	WaitForSingleObject(registerReadWriteMutex[drvno], INFINITE);
	status = lscpciej_writeRegister_16(drvno, data, address);
	ReleaseMutex(registerReadWriteMutex[drvno]);
	return status;
};

/**
 * @brief Write byte (8 bit) to register in space0 of PCIe board, except r10-r1f.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param data byte value to write
 * @param address Offset from BaseAdress of register (count in bytes)
 * @return @ref es_status_codes
 */
es_status_codes writeRegister_8(uint32_t drvno, uint8_t data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	WaitForSingleObject(registerReadWriteMutex[drvno], INFINITE);
	status = lscpciej_writeRegister_8(drvno, data, address);
	ReleaseMutex(registerReadWriteMutex[drvno]);
	return status;
};

/**
 * Check drvno for being legit
 * 
 * @param drvno driver number
 * @return @ref es_status_codes
 */
es_status_codes checkDriverHandle(uint32_t drvno)
{
	if (drvno >= number_of_boards)
		return es_invalid_driver_number;
	return lscpciej_checkDriverHandle(drvno);
}

uint64_t getPhysicalDmaAddress( uint32_t drvno)
{
	return lscpciej_getPhysicalDmaAddress(drvno);
}

/**
 * @brief Allocate DMA buffer - should only be called once.
 * 
 * Gets address of DMASubBuf from driver and copy it later to our pDMABigBuf.
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes SetupDma( uint32_t drvno )
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	ES_LOG( "Setup DMA\n" );
	//If DMA is already set up, clean it before
	if (dmaBuffer[drvno])
	{
		status = CleanupDma(drvno);
		if (status != es_no_error) return status;
	}
	dmaBufferSizeInBytes[drvno] = settings_struct.camera_settings[drvno].dma_buffer_size_in_scans * settings_struct.camera_settings[drvno].pixel * sizeof(uint16_t);
	return lscpciej_SetupDma(drvno, &dmaBuffer[drvno], dmaBufferSizeInBytes[drvno]);
}

es_status_codes enableInterrupt( uint32_t drvno )
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	ES_LOG("Enable interrupt\n");
	return lscpciej_enableInterrupt(drvno, interrupt_handler_array[drvno]);
}

/**
 * @brief Disable interrupt.
 * 
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes disableInterrupt(uint32_t drvno)
{
	ES_LOG("Disable interrupt\n");
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	return lscpciej_disableInterrupt(drvno);
}

/**
 * @brief Reset the buffer write pointers and software ISR counter.
 * 
 * @param drvno PCIe board identifier.
 */
void ResetBufferWritePos(uint32_t drvno)
{
	dmaBufferPartReadPos[drvno] = 0;
	// reset buffer index to base we got from InitDMA
	userBufferWritePos[drvno] = userBuffer[drvno];
	ES_TRACE( "reset userBufferWritePos to 0x%p\n", userBufferWritePos[drvno] );
	IsrCounter[drvno] = 0;
	return;
}

/**
 * @brief DMA copies in blocks of dmaBufferSizeInWords/DMA_BUFFER_PARTS - the rest is copied here
 *
 * @param drvno PCIe board identifier.
 * @param rest_in_bytes bytes which were not copied by INTR.
 */
void copyRestData(uint32_t drvno, size_t rest_in_bytes)
{
	ES_TRACE( "Copy rest data\n" );
	uint16_t* dmaBufferReadPos = dmaBuffer[drvno];
	// dmaBufferPartReadPos is 0 or 1 when DMA_BUFFER_PARTS=2 -> hi/lo half
	dmaBufferReadPos += dmaBufferPartReadPos[drvno] * dmaBufferSizeInBytes[drvno] /2 / DMA_BUFFER_PARTS;
	//					0 or 1 for lo/hi half		*  DMA buffer in shorts		  /      2	
	// rest_in_bytes = 2 x pixel x rest in scans
	ES_TRACE("copyRestData: dmaBufferReadPos: 0x%p \n", dmaBufferReadPos);
	ES_TRACE("copyRestData: userBufferWritePos: 0x%p \n", userBufferWritePos[drvno]);
	memcpy( userBufferWritePos[drvno], dmaBufferReadPos, rest_in_bytes);
	manipulateData(drvno, userBufferWritePos[drvno], (uint32_t)(rest_in_bytes / (sizeof(uint16_t) * settings_struct.camera_settings[drvno].pixel)));
	data_available[drvno] += rest_in_bytes / sizeof(uint16_t);
	ES_TRACE("copyRestData: increased available data to : %zu \n", data_available[drvno]);
	return;
}

/**
 * @brief Initializes PCIe board on a platform specific way.
 * 
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes _InitBoard(uint32_t drvno)
{
	ES_LOG("Initialize board %"PRIu32"\n", drvno);
	return lscpciej_InitBoard(drvno);
}

/**
 * @brief Windows specific function for initializing driver.
 * 
 * @return @ref es_status_codes
 */
es_status_codes _InitDriver(uint8_t* _number_of_boards)
{
	es_status_codes status = lscpciej_InitDriver(&number_of_boards);
	*_number_of_boards = number_of_boards;
	if (status == es_device_not_found)
	{
		ES_LOG("No PCIe card found. Set test mode to true.\n");
		testModeOn = true;
	}
	return status;
}

/**
 * @brief Cleanup driver. Call this before Exit driver.
 *
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes CleanupDriver(uint32_t drvno)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	ES_LOG("Cleanup driver drv: %"PRIu32"\n", drvno);
	status = disableInterrupt(drvno);
	if (status != es_no_error) return status;
	if (dmaBuffer[drvno])
	{
		status = CleanupDma(drvno);
		if (status != es_no_error) return status;
	}
	return lscpciej_CleanupDriver(drvno);
}

/**
 * @brief Exit driver. Call this after Cleanup driver.
 * 
 * @return @ref es_status_codes
 */
es_status_codes _ExitDriver()
{
	ES_LOG( "Driver exit\n");
	es_status_codes status = lscpciej_ExitDriver();
	ES_LOG( "Driver closed\n" );
	return status;
}

/**
 * @brief Read long (32 bit) from runtime register of PCIe board.
 *  
 * This function reads the memory mapped data , not the I/O Data. Reads data from PCIe config space.
 * 
 * @param drvno board number (=1 if one PCI board)
 * @param data pointer to where data is stored
 * @param address offset of register (count in bytes)
 * @return @ref es_status_codes
 */
es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint32_t address )
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	return lscpciej_readConfig_32(drvno, data, address);
}

/**
 * @brief Write long (32 bit) to register in space0 of PCIe board.
 *
 * @param drvno board number (=1 if one PCI board)
 * @param data long value to write
 * @param address offset from base address of register (count in bytes)
 * @return @ref es_status_codes
 */
es_status_codes writeConfig_32(uint32_t drvno, uint32_t data, uint32_t address)
{
	es_status_codes status = checkDriverHandle(drvno);
	if (status != es_no_error) return status;
	return lscpciej_writeConfig_32(drvno, data, address);
};

/**
 *
 * @brief Get the free and installed memory info.
 * 
 * @param[out] pmemory_all how much is installed
 * @param[out] pmemory_free how much is free
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
	if (settings_struct.camera_settings[drvno].use_software_polling)
	{
		ES_LOG("Start copy data to user buffer thread.\n");
		uint32_t* param = (uint32_t*)malloc(sizeof(uint32_t));
		*param = drvno;
		_beginthread(&PollDmaBufferToUserBuffer, 0, param);
	}
	return es_no_error;
}

es_status_codes InitMutex(uint32_t drvno)
{
	if (registerReadWriteMutex[drvno])
		CloseHandle(registerReadWriteMutex[drvno]);
	registerReadWriteMutex[drvno] = CreateMutex(NULL, FALSE, NULL);
	if (registerReadWriteMutexHighLevel[drvno])
		CloseHandle(registerReadWriteMutexHighLevel[drvno]);
	registerReadWriteMutexHighLevel[drvno] = CreateMutex(NULL, FALSE, NULL);
	return es_no_error;
}

void initPerformanceCounter()
{
	QueryPerformanceFrequency(&freq);
}

/**
* @brief Reads system timer.
*
* Read 2x ticks and calculate the difference between the calls in microseconds with DLLTickstous, init timer by calling DLLInitSysTimer before use.
* @return act ticks
*/
int64_t GetTimestampInTicks()
{
	LARGE_INTEGER PERFORMANCECOUNTERVAL = { 0, 0 };
	QueryPerformanceCounter(&PERFORMANCECOUNTERVAL);
	return PERFORMANCECOUNTERVAL.QuadPart;

}

int64_t GetTimestampInMicroseconds()
{
	int64_t timestampInTicks = GetTimestampInTicks();
	return ConvertTicksToMicroseconds(timestampInTicks);
}

int64_t GetTimestampInMilliseconds()
{
	return GetTimestampInMicroseconds() / 1000;
}

/**
 * @brief Translate ticks to microseconds.
 * @param ticks ticks of system timer
 * @return microseconds of ticks
*/
int64_t ConvertTicksToMicroseconds(int64_t ticks)
{
	int64_t ticksPerSecond = 0;
	ticksPerSecond = freq.QuadPart;
	int64_t microseconds = 0;
	if(ticksPerSecond)
		microseconds = ticks * 1000000 / ticksPerSecond;
	return microseconds;
}

/**
 * @brief This functions returns after a time given in microseconds. The time is measured in CPU ticks. The function is escapable by pressing ESC.
 *
 * @param microseconds Time to wait in microseconds.
 * @return 1 when success, 0 when aborted by ESC or failure
 */
uint8_t WaitforTelapsed(int64_t microseconds)
{
	if (microseconds)
	{
		//ES_TRACE("Wait for %u microseconds\n", microseconds);
		int64_t _start_timestamp = GetTimestampInMicroseconds();
		int64_t destination_timestamp = _start_timestamp + microseconds;
		//ES_LOG("start time: %"PRId64"\n", _start_timestamp);
		// detect overflow
		if (destination_timestamp < _start_timestamp) return 0;
		// wait until time elapsed
		while (destination_timestamp > GetTimestampInMicroseconds())
		{
			if (abortMeasurementFlag || checkEscapeKeyState())
			{
				abortMeasurementFlag = true;
				return 0;
			}
		}
		//ES_LOG("end time:  %"PRId64"\n", GetTimestampInMicroseconds());
	}
	return 1;
}

uint16_t checkEscapeKeyState()
{
#ifndef MINIMAL_BUILD
	return GetAsyncKeyState(VK_ESCAPE);
#else
	return 0;
#endif
}

uint16_t checkSpaceKeyState()
{
#ifndef MINIMAL_BUILD
	return GetAsyncKeyState(VK_SPACE);
#else
	return 0;
#endif
}

/**
 * @brief Set thread to high priority level.
 * 
 * @return @ref es_status_codes
 */
es_status_codes SetPriority()
{
	ES_TRACE("Increase priority\n");
	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
		return es_setting_thread_priority_failed;
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
		return es_setting_thread_priority_failed;
	return es_no_error;
}

/**
 * @brief Reset thread priority to normal.
 * 
 * @return @ref es_status_codes
 */
es_status_codes ResetPriority()
{
	ES_TRACE("Reset priority to normal\n");
	if (!SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS))
		return es_setting_thread_priority_failed;
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL))
		return es_setting_thread_priority_failed;
	return es_no_error;
}

uint16_t* getVirtualDmaAddress(uint32_t drvno)
{
	return dmaBuffer[drvno];
}

uint32_t getDmaBufferSizeInBytes(uint32_t drvno)
{
	return dmaBufferSizeInBytes[drvno];
}

int64_t getCurrentInterruptCounter(uint32_t drvno)
{
	// When measurement_cnt is greater than 1, that means measurement is running in a loop, return number of interrupts, because the last interrupt was the maximum number of interrupts, when the IsrCounter is 0.
	if (measurement_cnt > 1 && IsrCounter[drvno] == 0)
		return numberOfInterrupts[drvno];
	else
		return IsrCounter[drvno];
}

#ifndef MINIMAL_BUILD

/**
 * \brief Export the measurement data to a binary file.
 *
 * \param filename Filename with complete absolute path.
 * \return @ref es_status_codes
 */
es_status_codes SaveMeasurementDataToFileBIN(const char* filename)
{
	ES_LOG("Export measurement to bin file\n");
	FILE* file = NULL;
	fopen_s(&file, filename, "wb");
	if (!file)
	{
		ES_LOG("File could not be opened\n");
		return es_create_file_failed;
	}
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		ES_LOG("Writing header drvno %"PRIu32"\n", drvno);
		// Assemble the file_header
		struct file_header fh;
		fh.software_version_major = VERSION_MAJOR_ESCAM;
		fh.software_version_pcie = VERSION_PCIE_BOARD_VERSION;
		fh.software_version_minor = VERSION_MINOR_ESCAM;
		fh.number_of_boards = number_of_boards;
		fh.board_sel = settings_struct.board_sel;
		fh.drvno = drvno;
		fh.pixel = settings_struct.camera_settings[drvno].pixel;
		fh.nos = settings_struct.nos;
		fh.nob = settings_struct.nob;
		fh.camcnt = virtualCamcnt[drvno];
		fh.measurement_cnt = measurement_cnt;
		memset(fh.timestamp, 0, file_timestamp_size);
		strcpy_s(fh.timestamp, file_timestamp_size, start_timestamp);
		// Write struct file_header to the file.
		fwrite(&fh, 1, sizeof(struct file_header), file);
		// Write data to the file.
		ES_LOG("Writing measurement data drvno %"PRIu32"\n", drvno);
		fwrite(userBuffer[drvno], sizeof(uint16_t), (uint64_t)settings_struct.nos * (uint64_t)settings_struct.nob * (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.camera_settings[drvno].pixel, file);
	}
	ES_LOG("Close file\n");
	fclose(file);
	return es_no_error;
}

es_status_codes CopyFromFileToUserBufferBIN(const char* filename)
{
	ES_LOG("Copy from file to user buffer\n");
	FILE* file = NULL;
	fopen_s(&file, filename, "rb");
	if (!file)
	{
		ES_LOG("File could not be opened\n");
		return es_open_file_failed;
	}
	// Read struct file_header from the file.
	struct file_header fh;
	fread(&fh, 1, sizeof(struct file_header), file);
	uint32_t drvno = fh.drvno;
	// Read data from the file.
	ES_LOG("Reading measurement data drvno %"PRIu32"\n", drvno);
	fread(userBuffer[drvno], sizeof(uint16_t), (uint64_t)settings_struct.nos * (uint64_t)settings_struct.nob * (uint64_t)virtualCamcnt[drvno] * (uint64_t)settings_struct.camera_settings[drvno].pixel, file);
	ES_LOG("Close file\n");
	fclose(file);
	return es_no_error;
}

void openFile(uint32_t drvno)
{
	size_t path_length = strlen(settings_struct.camera_settings[drvno].file_path);
	// Check if the path is terminated with /
	char last_char = settings_struct.camera_settings[drvno].file_path[path_length - 1];
	if (last_char != '/' && last_char != '\\')
	{
		// Append / to the path
		settings_struct.camera_settings[drvno].file_path[path_length] = '/';
		// Terminate the string with 0
		settings_struct.camera_settings[drvno].file_path[path_length + 1] = 0;
	}
	char filename_full[file_filename_full_size];
	memset(filename_full, 0, file_filename_full_size);
	// Create filenames
	sprintf_s(filename_full, file_filename_full_size, "%s%s_board-%"PRIu32".bin", settings_struct.camera_settings[drvno].file_path, start_timestamp, drvno);
	// Check if the file exists
	if (_access_s(filename_full, 0) != 0)
	{
		ES_LOG("File doesn't exist. Creating file.\n");
		// Create file and write the file header to it.
		fopen_s(&file_stream[drvno], filename_full, "abc");
		writeFileHeaderToFile(drvno);
		if (ghMutex[drvno])
			CloseHandle(ghMutex[drvno]);
		ghMutex[drvno] = CreateMutex(NULL, FALSE, NULL);
	}
	else
	{
		ES_LOG("File already exist. Open file.\n");
		fopen_s(&file_stream[drvno], filename_full, "abc");
	}
	return;
}

void closeFile(uint32_t drvno)
{
	ES_LOG("Close file\n");
	fclose(file_stream[drvno]);
	return;
}

void setTimestamp()
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	sprintf_s(start_timestamp, file_timestamp_size, "%04d-%02d-%02d-%02d-%02d-%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	return;
}

/**
 * @brief Creates a file at filename_full and writes struct file_header to it.
 * 
 * @param drvno PCIe board identifier.
 */
void writeFileHeaderToFile(uint32_t drvno)
{
	ES_LOG("Writing file header\n");
	// Assemble the file_header
	struct file_header fh;
	fh.software_version_major = VERSION_MAJOR_ESCAM;
	fh.software_version_pcie = VERSION_PCIE_BOARD_VERSION;
	fh.software_version_minor = VERSION_MINOR_ESCAM;
	fh.number_of_boards = number_of_boards;
	fh.board_sel = settings_struct.board_sel;
	fh.drvno = drvno;
	fh.pixel = settings_struct.camera_settings[drvno].pixel;
	fh.nos = settings_struct.nos;
	fh.nob = settings_struct.nob;
	fh.camcnt = virtualCamcnt[drvno];
	fh.measurement_cnt = measurement_cnt;
	memset(fh.timestamp, 0, file_timestamp_size);
	strcpy_s(fh.timestamp, file_timestamp_size, start_timestamp);
	if (file_stream[drvno])
	{
		// Write struct file_header to the file.
		fwrite(&fh, 1, sizeof(struct file_header), file_stream[drvno]);
	}
	return;
}

void writeToDisc(uint32_t* drvno_ptr)
{
	uint32_t drvno = *drvno_ptr;
	free(drvno_ptr);
	ES_LOG("Start write to disc, drvno %"PRIu32"\n", drvno);
	openFile(drvno);
	size_t data_count_to_write = 0;
	size_t data_written_all = 0;
	size_t data_written = 0;
	int errnumber = 0;
	// wait until there is data available
	while (!data_available[drvno]);
	data_count_to_write = data_available[drvno];
	while (isRunning || data_count_to_write && !abortMeasurementFlag && !errnumber)
	{
		// check if there is new available data
		data_count_to_write = data_available[drvno] - data_written_all;
		if (data_count_to_write)
		{
			// check if data_count_to_write is in the boundaries of userBuffer
			if ((uint16_t*)(userBufferWritePos_last[drvno] + data_count_to_write) > userBufferEndPtr[drvno])
			{
				data_count_to_write = userBufferEndPtr[drvno] - userBufferWritePos_last[drvno];
				ES_TRACE("data_count_to_write is exceeding the user buffer. Write the last part of the user buffer to disc.\n");
			}
			ES_TRACE("Write %zu bytes to disk, drvno %"PRIu32", data_available %zu, data_written_all %zu\n", data_count_to_write, drvno, data_available[drvno], data_written_all);
			ES_TRACE("write data to disc from userBufferWritePos_last: 0x%p \n", userBufferWritePos_last[drvno]);
			// write data to disc
			data_written = fwrite(userBufferWritePos_last[drvno], sizeof(uint16_t), data_count_to_write, file_stream[drvno]);
			// when there were no data written, probably an error occurred
			if (!data_written)
			{
				_get_errno(&errnumber);
				ES_TRACE("Error: %d\n", errnumber);
			}
			_flushall();
			ES_TRACE("data_written: %zu \n", data_written);
			// advance user buffer pointer
			userBufferWritePos_last[drvno] += data_written;
			// check if userBufferWritePos_last reached the end of userBuffer
			if (userBufferWritePos_last[drvno] >= userBufferEndPtr[drvno])
			{
				userBufferWritePos_last[drvno] = userBuffer[drvno];
				ES_TRACE("end of user buffer reached, set userBufferWritePos_last back to user buffer start \n");
			}
			ES_TRACE("advanced userBufferWritePos_last to: 0x%p \n", userBufferWritePos_last[drvno]);
			// increase the counter for overall written data
			data_written_all += data_written;
			ES_TRACE("data_written_all: %zu \n", data_written_all);
		}
	}
	closeFile(drvno);
	ES_LOG("Write to disc done, drvno: %"PRIu32"\n", drvno);
	return;
}

void startWriteToDiscThead(uint32_t drvno)
{
	uint32_t* drvno_tmp = malloc(sizeof(uint32_t));
	*drvno_tmp = drvno;
	_beginthread(&writeToDisc, 0, drvno_tmp);
	return;
}

/**
 * @brief Check a file for its data consistency.
 * 
 * @param vd see struct verify_data_parameter in globals.h for details
 */
void VerifyData(struct verify_data_parameter* vd)
{
	FILE* stream;
	// Read file in binary mode
	fopen_s(&stream, vd->filename_full, "rb");
	if (stream)
	{
		uint16_t data_buffer[4000];
		// Read file header and write it to fh
		fread_s(&vd->fh, sizeof(struct file_header), 1, sizeof(struct file_header), stream);
		// Set all counter to initial values
		uint32_t cur_sample_cnt = 1;
		uint32_t cur_block_cnt = 1;
		vd->sample_cnt = 1;
		vd->block_cnt = 1;
		vd->measurement_cnt = 0;
		vd->error_cnt = 0;
		vd->last_sample_before_error = 1;
		vd->last_block_before_error = 1;
		vd->last_measurement_before_error = 0;
		// while is ended by break
		while (true)
		{
			// Read data from file frame wise
			fread_s(data_buffer, sizeof(data_buffer), 1, vd->fh.pixel * sizeof(uint16_t), stream);
			// When the end of the file is reached, break the while loop.
			if (feof(stream)) break;
			// Assemble the 32 bit counters from upper and lower 16 bit counter half
			vd->last_sample = ((uint32_t)data_buffer[pixel_scan_index_high]) << 16 | ((uint32_t)data_buffer[pixel_scan_index_low]);
			uint16_t block_index_high = data_buffer[pixel_block_index_high_s1_s2] & pixel_block_index_high_s1_s2_bits_block_index;
			vd->last_block = ((uint32_t)block_index_high << 16) | ((uint32_t)data_buffer[pixel_block_index_low]);
			// Check if the theoretical counter value is identical with the actual counter value from the file
			if (vd->last_sample != cur_sample_cnt || vd->last_block != cur_block_cnt)
			{
				// Save the counter of the last valid sample, block and measurement on the first error
				if (vd->error_cnt == 0)
				{
					vd->last_sample_before_error = vd->last_sample - 1;
					vd->last_block_before_error = vd->last_block - 1;
					vd->last_measurement_before_error = vd->measurement_cnt;
				}
				vd->error_cnt++;
			}
			// Set sample_cnt to the maximum found cur_sample_cnt
			if (cur_sample_cnt > vd->sample_cnt) vd->sample_cnt = cur_sample_cnt;
			// Set block_cnt to the maximum found cur_block_cnt
			if (cur_block_cnt > vd->block_cnt) vd->block_cnt= cur_block_cnt;
			// Increment counters for the next loop
			cur_sample_cnt++;
			if (cur_sample_cnt > vd->fh.nos)
			{
				cur_sample_cnt = 1;
				cur_block_cnt++;
			}
			// block counter is only 30 bits wide, so maximum is 0x3FFFFFFF
			if (cur_block_cnt > vd->fh.nob || cur_block_cnt > 0x3FFFFFFF)
			{
				cur_block_cnt = 1;
				vd->measurement_cnt++;
			}
		}
		fclose(stream);
	}
	return;
}

/**
 * @brief Open the file at filename_full and write the header to fh.
 * 
 * @param fh struct file_header*
 * @param filename_full Path and file name to the file.
 */
void getFileHeaderFromFile(struct file_header* fh, const char* filename_full)
{
	FILE* stream;
	// Open file in read binary mode
	fopen_s(&stream, filename_full, "rb");
	if (stream)
	{
		fread_s(fh, sizeof(struct file_header), 1, sizeof(struct file_header), stream);
		fclose(stream);
	}
	return;
}

#endif

void WaitForAllInterruptsDone()
{
	ES_TRACE("Wait for all interrupts\n")
	int64_t start_time = GetTimestampInMicroseconds();
	int64_t timeoutInMicroseconds = 10000;
	while (!(allInterruptsDone[0] && allInterruptsDone[1] && allInterruptsDone[2] && allInterruptsDone[3] && allInterruptsDone[4]))
	{
		if (abortMeasurementFlag || checkEscapeKeyState())
		{
			abortMeasurementFlag = true;
			return;
		}
		if (GetTimestampInMicroseconds() - start_time > timeoutInMicroseconds)
		{
			ES_LOG("WaitForAllInterruptsDone() timeout, start time %lli, current time %lli, diff %lli\n", start_time, GetTimestampInMicroseconds(), GetTimestampInMicroseconds()- start_time);
			return;
		}
	}
	ES_TRACE("All interrupts done\n")
	return;
}

/**
 * @brief Display information about registers and settings in pop up windows.
 * 
 * @return @ref es_status_codes
 */
es_status_codes About(uint32_t board_sel)
{
	es_status_codes status = es_no_error;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			status = AboutDrv(drvno);
			if (status != es_no_error) return status;
			status = AboutGPX(drvno);
			//if (status != es_no_error) return status;
			status = AboutS0(drvno);
			if (status != es_no_error) return status;
			status = AboutTLPs(drvno);
			if (status != es_no_error) return status;
			status = AboutPCI(drvno);
			if (status != es_no_error) return status;
			status = AboutCameraSettings(drvno);
			if (status != es_no_error) return status;
		}
	status = AboutMeasurementSettings();
	if (status != es_no_error) return status;
	return status;
}

/**
* @brief Shows window with infos about the PCIe board.
*
* - version of driver
* - ID = 53xx
* - length of space0 BAR =0x3f
* - vendor ID = EBST
* - PCI board version (same as label on PCI board)
* @param drvno board number (=1 if one PCI board)
* @return @ref es_status_codes
*/
es_status_codes AboutDrv(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = _AboutDrv(drvno, &cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"About driver", MB_OK);
	return status;
};

/**
 * @brief Reads registers 0 to 12 of TDC-GPX chip. Time delay counter option.
 *
 * @param drvno PCIe board identifier
 * @return @ref es_status_codes
 */
es_status_codes AboutGPX(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = _AboutGPX(drvno, &cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"GPX regs", MB_OK);
	return status;
}

/**
 * @brief Read registers of space0. Space0 are the control registers of the PCIe board.
 *
 * @param drvno PCIe board identifier
 * @return @ref es_status_codes
 */
es_status_codes AboutS0(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpS0Registers(drvno, &cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"S0 regs", MB_OK);
	return status;
}//AboutS0

/**
 * @brief
 *
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes AboutTLPs(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpTlpRegisters(drvno, &cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"DMA transfer payloads", MB_OK | MB_DEFBUTTON2);
	return status;
}//AboutTLPs

/**
 * @brief
 *
 * @param drvno PCIe board identifier.
 * @return @ref es_status_codes
 */
es_status_codes AboutPCI(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpPciRegisters(drvno, &cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"PCI regs", MB_OK);
	return status;
}//AboutPCI

es_status_codes AboutMeasurementSettings()
{
	char* cstring;
	es_status_codes status = dumpMeasurementSettings(&cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"Measurement settings", MB_OK);
	return status;
}

es_status_codes AboutCameraSettings(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpCameraSettings(drvno, &cstring);
	MessageBox(GetActiveWindow(), (LPCTSTR)cstring, (LPCTSTR)L"Camera settings", MB_OK);
	return status;
}


/**
 * @brief Switch on error message boxes of our software. Default is On.
 */
void ErrMsgBoxOn()
{
	_SHOW_MSG = TRUE;
}

/**
 * @brief Disable error message boxes, if not needed.
 */
void ErrMsgBoxOff()
{
	_SHOW_MSG = FALSE;
}

/**
 * @brief Display error message. If ErrMsgBoxOn is set.
 *
 * @param[in] ErrMsg Message. Buffer size: 100.
 */
void ErrorMsg(char ErrMsg[100])
{
	if (_SHOW_MSG)
	{
		if (MessageBoxA(GetActiveWindow(), (LPCSTR)ErrMsg, (LPCSTR)L"ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

/**
 * @brief Simple display of unsigned integer as error message for test purpose.
 *
 * @param val unsigned integer 64 bit
 */
void ValMsg(uint64_t val)
{
	char AString[60];
	if (_SHOW_MSG)
	{
		sprintf_s(AString, 60, "%s%"PRIu64" 0x%I64x", "val= ", val, val);
		if (MessageBoxA(GetActiveWindow(), (LPCSTR)AString, (LPCSTR)L"ERROR", MB_OK | MB_ICONEXCLAMATION) == IDOK) {};
	}
};

#ifndef MINIMAL_BUILD

/**
@brief Start 2d viewer.
@param[in] drvno board number
@param[in] block current number of block
@param[in] camera which camera to display (when camcnt is >1)
@param[in] pixel count of pixel of one line
@param[in] nos samples in one block
*/
void Start2dViewer(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos)
{
	ES_TRACE("Start 2d viewer, drvno %"PRIu32", block %"PRIu32", camera %"PRIu16", pixel %"PRIu16", nos %"PRIu32"\n", drvno, block, camera, pixel, nos);
	if (Direct2dViewer != NULL)
	{
		Deinit2dViewer();
	}
	Direct2dViewer = Direct2dViewer_new();
	uint16_t* address = NULL;
	if (virtualCamcnt[drvno] <= 1)
		GetOneBlockPointer(drvno, block, &address, NULL);
	else
	{
		// check if greyscale_data was allocated before, if so free it
		if (greyscale_data)
			free(greyscale_data);
		// allocate memory for one block of one camera
		greyscale_data = (uint16_t*)malloc(settings_struct.camera_settings[drvno].pixel * settings_struct.nos * sizeof(uint16_t));
		CopyOneBlockOfOneCamera(drvno, block, camera, greyscale_data);
		address = greyscale_data;
	}
	Direct2dViewer_start2dViewer(
		Direct2dViewer,
		GetActiveWindow(),
		address,
		pixel,
		nos);
	return;
}

/**
@brief Update the displayed bitmap.
@param[in] drvno board number
@param[in] block current number of blocks
@param[in] camera which camera to display (when camcnt is >1)
@param[in] pixel count of pixel of one line
@param[in] nos samples in one block
*/
void ShowNewBitmap(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos)
{
	ES_TRACE("Show new bitmap, drvno %"PRIu32", block %"PRIu32", camera %"PRIu16", pixel %"PRIu16", nos %"PRIu32"\n", drvno, block, camera, pixel, nos);
	if (Direct2dViewer != NULL)
	{
		uint16_t* address = NULL;
		if (virtualCamcnt[drvno] <= 1)
			GetOneBlockPointer(drvno, block, &address, NULL);
		else
		{
			// check if greyscale_data was allocated before, if so free it
			if(greyscale_data)
				free(greyscale_data);
			// allocate memory for one block of one camera
			greyscale_data = (uint16_t*)malloc(settings_struct.camera_settings[drvno].pixel * settings_struct.nos * sizeof(uint16_t));
			CopyOneBlockOfOneCamera(drvno, block, camera, greyscale_data);
			address = greyscale_data;
		}
		Direct2dViewer_showNewBitmap(
			Direct2dViewer,
			address,
			pixel,
			nos);
	}
	return;
}

/**
@brief Call when closing 2d viewer or at least before opening a new 2d viewer.
*/
void Deinit2dViewer()
{
	ES_TRACE("Deinit 2d viewer\n");
	if (Direct2dViewer != NULL)
	{
		SendMessage(Direct2dViewer_getWindowHandler(Direct2dViewer), WM_CLOSE, 0, 0);
		Direct2dViewer_delete(Direct2dViewer);
		Direct2dViewer = NULL;
	}
	return;
}

/**
 * @copydoc Direct2dViewer::SetGammaValue
 */
void SetGammaValue(uint16_t white, uint16_t black)
{
	ES_TRACE("Set gamma value, white %"PRIu16", black %"PRIu16"\n", white, black);
	if (Direct2dViewer != NULL)
	{
		Direct2dViewer_setGammaValue(Direct2dViewer, white, black);
		Direct2dViewer_repaintWindow(Direct2dViewer);
	}
	return;
}

/**
 * @copydoc Direct2dViewer::GetGammaWhite
 */
uint16_t GetGammaWhite()
{
	ES_TRACE("Get gamma white\n");
	if (Direct2dViewer != NULL)
	{
		return Direct2dViewer_getGammaWhite(Direct2dViewer);
	}
	return 0;
}

/**
 * @copydoc Direct2dViewer::GetGammaBlack
 */
uint16_t GetGammaBlack()
{
	ES_TRACE("Get gamma black\n");
	if (Direct2dViewer != NULL)
	{
		return Direct2dViewer_getGammaBlack(Direct2dViewer);
	}
	return 0;
}

#endif

void LockHighLevelMutex(uint32_t drvno)
{
	WaitForSingleObject(registerReadWriteMutexHighLevel[drvno], INFINITE);
	return;
}

void UnlockHighLevelMutex(uint32_t drvno)
{
	ReleaseMutex(registerReadWriteMutexHighLevel[drvno]);
	return;
}
