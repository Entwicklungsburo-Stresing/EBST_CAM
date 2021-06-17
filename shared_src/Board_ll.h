#ifndef BOARDLL_H
#define BOARDLL_H

#include <stdint.h>
#include "es_status_codes.h"
#include "globals.h"
#include "Board.h"

#ifdef WIN32
#define ES_LOG(...) WDC_Err(__VA_ARGS__);
#include "Jungo/wdc_lib.h"
extern WDC_DEVICE_HANDLE* hDev;
extern bool _SHOW_MSG;
#endif

#ifdef __linux__
#include <stdio.h>
#define ES_LOG(...) fprintf(stderr, __VA_ARGS__);
#endif

// Low level API
// platform specific implementation

es_status_codes checkDriverHandle(uint32_t drvno);
es_status_codes readRegister_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes readRegister_16( uint32_t drvno, uint16_t* data, uint16_t address );
es_status_codes readRegister_8( uint32_t drvno, uint8_t* data, uint16_t address );
es_status_codes writeRegister_32( uint32_t drvno, uint32_t data, uint16_t address );
es_status_codes writeRegister_16( uint32_t drvno, uint16_t data, uint16_t address );
es_status_codes writeRegister_8( uint32_t drvno, uint8_t data, uint16_t address );
es_status_codes readConfig_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes writeConfig_32( uint32_t drvno, uint32_t data, uint16_t address );
void FreeMemInfo( uint64_t *pmemory_all, uint64_t *pmemory_free );
es_status_codes SetupDma( uint32_t drvno );
es_status_codes enableInterrupt( uint32_t drvno );
uint64_t getDmaAddress( uint32_t drvno);
void ResetBufferWritePos(uint32_t drvno);
void copyRestData(uint32_t drvno, size_t rest_in_bytes);
es_status_codes _InitBoard(uint32_t drvno);
es_status_codes _InitDriver();
es_status_codes _ExitDriver(uint32_t drvno);
es_status_codes StartCopyDataToUserBufferThread(uint32_t drvno);
es_status_codes AboutDrv(uint32_t drvno);
es_status_codes AboutGPX(uint32_t drvno);
es_status_codes AboutS0(uint32_t drvno);
es_status_codes AboutTLPs(uint32_t drvno);
void ErrMsgBoxOn();
void ErrMsgBoxOff(); // switch to suppress error message boxes
void ErrorMsg(char ErrMsg[100]);
long long ticksTimestamp();
es_status_codes WaitTrigger(uint32_t drvno, bool ExtTrigFlag, bool *SpaceKey, bool *AbrKey);
uint32_t Tickstous(uint64_t tks);

#endif // BOARDLL_H
