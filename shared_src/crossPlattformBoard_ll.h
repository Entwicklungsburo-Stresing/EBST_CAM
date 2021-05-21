#ifndef CROSSPLATTFORMBOARDLL_H
#define CROSSPLATTFORMBOARDLL_H

#include <stdint.h>
#include "es_status_codes.h"

#ifdef WIN32
#define ES_LOG(...) WDC_Err(__VA_ARGS__);
#endif

#ifdef __linux__
#include <stdio.h>
#define ES_LOG(...) fprintf(stderr, __VA_ARGS__);
#endif

#define MAXPCIECARDS 5
#define DMA_BUFFER_SIZE_IN_SCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans
#define DMA_BUFFER_PARTS 2
#define DMA_DMASPERINTR DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb 
/**
 * @brief DMA_CONTIGBUF: DMA buffer type switch.
 * 
 * true: DMA buffer is set by driver (data must be copied afterwards to user space).
 * false: DMA buffer is set by application (pointer must be passed to SetupPCIE_DMA).
 */
#define DMA_CONTIGBUF true

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

#endif // CROSSPLATTFORMBOARDLL_H