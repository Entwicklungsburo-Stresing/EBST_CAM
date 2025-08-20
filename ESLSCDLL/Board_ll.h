/*****************************************************************//**
 * @file		Board_ll.h
 * @brief		All the low level platform specific implementations for interacting with the Stresing PCIe board.
 * @author		Florian Hahn
 * @date		23.10.2020
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
*********************************************************************/

#pragma once

#include <stdint.h>
#include "globals.h"
#include "../shared_src/es_status_codes.h"

#ifdef WIN32
#include "lscpciej/lscpciej.h"
#ifdef _DEBUG
#define ES_LOG(...) lscpciej_log_error(__VA_ARGS__);
#define ES_TRACE(...) lscpciej_log_trace(__VA_ARGS__);
#else
#define ES_LOG(...)
#define ES_TRACE(...)
#endif
extern bool _SHOW_MSG;
#endif

#ifdef __linux__
#include <stdio.h>
#include <pthread.h>
#define ES_LOG(...) fprintf(stderr, __VA_ARGS__);
#define ES_TRACE(...) //fprintf(stderr, __VA_ARGS__);
extern pthread_mutex_t mutex[MAXPCIECARDS];
#endif

// Low level API
// platform specific implementation

es_status_codes checkDriverHandle(uint32_t drvno);
es_status_codes readRegister_32( uint32_t drvno, uint32_t* data, uint32_t address );
es_status_codes readRegister_16( uint32_t drvno, uint16_t* data, uint32_t address );
es_status_codes readRegister_8( uint32_t drvno, uint8_t* data, uint32_t address );
es_status_codes writeRegister_32( uint32_t drvno, uint32_t data, uint32_t address );
es_status_codes writeRegister_16( uint32_t drvno, uint16_t data, uint32_t address );
es_status_codes writeRegister_8( uint32_t drvno, uint8_t data, uint32_t address );
es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint32_t address );
es_status_codes writeConfig_32( uint32_t drvno, uint32_t data, uint32_t address );
void FreeMemInfo( uint64_t *pmemory_all, uint64_t *pmemory_free );
es_status_codes SetupDma( uint32_t drvno );
es_status_codes enableInterrupt( uint32_t drvno );
es_status_codes disableInterrupt( uint32_t drvno );
uint64_t getPhysicalDmaAddress( uint32_t drvno);
void ResetBufferWritePos(uint32_t drvno);
void copyRestData(uint32_t drvno, size_t rest_in_bytes);
es_status_codes _InitBoard(uint32_t drvno);
es_status_codes _InitDriver();
es_status_codes _ExitDriver();
es_status_codes CleanupDriver(uint32_t drvno);
es_status_codes StartCopyDataToUserBufferThread(uint32_t drvno);
uint16_t checkEscapeKeyState();
uint16_t checkSpaceKeyState();
es_status_codes InitMutex(uint32_t drvno);
es_status_codes SetPriority();
es_status_codes ResetPriority();
uint16_t* getVirtualDmaAddress(uint32_t drvno);
uint32_t getDmaBufferSizeInBytes(uint32_t drvno);
int64_t getCurrentInterruptCounter(uint32_t drvno);
uint8_t WaitforTelapsed(int64_t microseconds);
#ifndef MINIMAL_BUILD
es_status_codes SaveMeasurementDataToFileBIN(const char* filename);
es_status_codes CopyFromFileToUserBufferBIN(const char* filename);
void openFile(uint32_t drvno);
void closeFile(uint32_t drvno);
void setTimestamp();
void writeFileHeaderToFile(uint32_t drvno);
void writeToDisc(uint32_t* drvno_ptr);
void startWriteToDiscThead(uint32_t drvno);
void VerifyData(struct verify_data_parameter* vd);
void getFileHeaderFromFile(struct file_header* fh, const char* filename_full);
// direct 2d viewer
void Start2dViewer(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos);
void ShowNewBitmap(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos);
void Deinit2dViewer();
void SetGammaValue(uint16_t white, uint16_t black);
uint16_t GetGammaWhite();
uint16_t GetGammaBlack();
#endif
void WaitForAllInterruptsDone();
void LockHighLevelMutex(uint32_t drvno);
void UnlockHighLevelMutex(uint32_t drvno);
int64_t GetTimestampInMicroseconds();
int64_t GetTimestampInMilliseconds();
void initPerformanceCounter();

#ifdef WIN32
int64_t GetTimestampInTicks();
int64_t ConvertTicksToMicroseconds(int64_t ticks);
es_status_codes About(uint32_t board_sel);
es_status_codes AboutDrv(uint32_t drvno);
es_status_codes AboutGPX(uint32_t drvno);
es_status_codes AboutS0(uint32_t drvno);
es_status_codes AboutTLPs(uint32_t drvno);
es_status_codes AboutPCI(uint32_t drvno);
es_status_codes AboutMeasurementSettings();
es_status_codes AboutCameraSettings(uint32_t drvno);
void ErrMsgBoxOn();
void ErrMsgBoxOff(); // switch to suppress error message boxes
void ErrorMsg(char ErrMsg[100]);
void ValMsg(uint64_t val);
#endif
