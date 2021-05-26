#ifndef CROSSPLATTFORMBOARDLL_H
#define CROSSPLATTFORMBOARDLL_H

#include <stdint.h>
#include "es_status_codes.h"
#include "crossPlattformBoard.h"
#include "globals.h"

#ifdef WIN32
#define ES_LOG(...) WDC_Err(__VA_ARGS__);
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
void copyRestData(size_t rest_in_bytes);
es_status_codes _InitBoard(uint32_t drvno);
es_status_codes _InitDriver();
es_status_codes _ExitDriver(uint32_t drvno);
es_status_codes GetAddressOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t CAM, uint16_t** address );
#endif // CROSSPLATTFORMBOARDLL_H
