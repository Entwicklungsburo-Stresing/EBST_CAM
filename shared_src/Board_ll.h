#ifndef BOARDLL_H
#define BOARDLL_H

#include <stdint.h>
#include "es_status_codes.h"
#include "globals.h"

#ifdef WIN32
#define ES_LOG(...) WDC_Err(__VA_ARGS__);
#define ES_TRACE(...) WDC_Trace(__VA_ARGS__);
#include "Jungo/wdc_lib.h"
extern WDC_DEVICE_HANDLE* hDev;
extern bool _SHOW_MSG;
#endif

#ifdef __linux__
#include <stdio.h>
#include <pthread.h>
#define ES_LOG(...) fprintf(stderr, __VA_ARGS__);
#define ES_TRACE(...) fprintf(stderr, __VA_ARGS__);
extern pthread_mutex_t mutex[MAXPCIECARDS];
#endif

// Low level API
// platform specific implementation

es_status_codes checkDriverHandle(uint32_t drvno);
es_status_codes readRegister_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes readRegister_16( uint32_t drvno, uint16_t* data, uint16_t address );
es_status_codes readRegister_8( uint32_t drvno, uint8_t* data, uint16_t address );
es_status_codes writeRegister_32( uint32_t drvno, uint32_t data, uint16_t address );
es_status_codes writeRegister_32twoBoards(uint32_t data1, uint32_t data2, uint16_t address);
es_status_codes writeRegister_16( uint32_t drvno, uint16_t data, uint16_t address );
es_status_codes writeRegister_8( uint32_t drvno, uint8_t data, uint16_t address );
es_status_codes writeRegister_8twoBoards(uint8_t data1, uint8_t data2, uint16_t address);
es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes writeConfig_32( uint32_t drvno, uint32_t data, uint16_t address );
void FreeMemInfo( uint64_t *pmemory_all, uint64_t *pmemory_free );
es_status_codes SetupDma( uint32_t drvno );
es_status_codes enableInterrupt( uint32_t drvno );
uint64_t getPhysicalDmaAddress( uint32_t drvno);
void ResetBufferWritePos(uint32_t drvno);
void copyRestData(uint32_t drvno, size_t rest_in_bytes);
es_status_codes _InitBoard(uint32_t drvno);
es_status_codes _InitDriver();
es_status_codes _ExitDriver(uint32_t drvno);
es_status_codes StartCopyDataToUserBufferThread(uint32_t drvno);
uint16_t checkEscapeKeyState();
uint16_t checkSpaceKeyState();
es_status_codes InitMutex(uint32_t drvno);
es_status_codes SetPriority(uint32_t threadp);
es_status_codes ResetPriority();
uint16_t* getVirtualDmaAddress(uint32_t drvno);
uint32_t getDmaBufferSizeInBytes(uint32_t drvno);
int64_t getCurrentInterruptCounter();

#ifdef WIN32
es_status_codes About(uint32_t drvno);
es_status_codes AboutDrv(uint32_t drvno);
es_status_codes AboutGPX(uint32_t drvno);
es_status_codes AboutS0(uint32_t drvno);
es_status_codes AboutTLPs(uint32_t drvno);
es_status_codes AboutPCI(uint32_t drvno);
es_status_codes AboutSettings();
void ErrMsgBoxOn();
void ErrMsgBoxOff(); // switch to suppress error message boxes
void ErrorMsg(char ErrMsg[100]);
void ValMsg(uint64_t val);
long long ticksTimestamp();
es_status_codes WaitTrigger(uint32_t drvno, bool ExtTrigFlag, bool *SpaceKey, bool *AbrKey);
uint32_t Tickstous(uint64_t tks);
uint8_t WaitforTelapsed(long long musec);
void InitProDLL();
#endif

#endif // BOARDLL_H
