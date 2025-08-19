/*****************************************************************//**
 * @file		lscpciej.h
 * @brief		Library lscpciej for driver communication with PCIe card driver.
 * @details		The library lscpciej communicatates with the DLL wdapi1400.dll, which is the PCIe card driver.
 * @author		Florian Hahn
 * @date		18.08.2025
 * @copyright	Copyright Entwicklungsbüro Stresing.
 *********************************************************************/

#pragma once

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include "../shared_src/es_status_codes.h"

#define DllAccess __declspec( dllexport )
#ifdef __cplusplus
extern "C" {
#endif

typedef void(__stdcall* LSCPCIEJ_INT_HANDLER_STUB)(void);

DllAccess es_status_codes lscpciej_CleanupDma(uint32_t drvno);
DllAccess es_status_codes lscpciej_readRegister_32(uint32_t drvno, uint32_t* data, uint32_t address);
DllAccess es_status_codes lscpciej_readRegister_16(uint32_t drvno, uint16_t* data, uint32_t address);
DllAccess es_status_codes lscpciej_readRegister_8(uint32_t drvno, uint8_t* data, uint32_t address);
DllAccess es_status_codes lscpciej_writeRegister_32(uint32_t drvno, uint32_t data, uint32_t address);
DllAccess es_status_codes lscpciej_writeRegister_16(uint32_t drvno, uint16_t data, uint32_t address);
DllAccess es_status_codes lscpciej_writeRegister_8(uint32_t drvno, uint8_t data, uint32_t address);
DllAccess es_status_codes lscpciej_checkDriverHandle(uint32_t drvno);
DllAccess uint64_t lscpciej_getPhysicalDmaAddress(uint32_t drvno);
DllAccess es_status_codes lscpciej_SetupDma(uint32_t drvno, uint16_t** dmaBufferPtr, DWORD dmaBufferSizeInBytes);
DllAccess es_status_codes lscpciej_enableInterrupt(uint32_t drvno, LSCPCIEJ_INT_HANDLER_STUB interrupt_handler);
DllAccess es_status_codes lscpciej_disableInterrupt(uint32_t drvno);
DllAccess es_status_codes lscpciej_InitBoard(uint32_t drvno);
DllAccess es_status_codes lscpciej_InitDriver(uint8_t* number_of_boards);
DllAccess es_status_codes lscpciej_CleanupDriver(uint32_t drvno);
DllAccess es_status_codes lscpciej_ExitDriver();
DllAccess es_status_codes lscpciej_readConfig_32(uint32_t drvno, uint32_t* data, uint32_t address);
DllAccess es_status_codes lscpciej_writeConfig_32(uint32_t drvno, uint32_t data, uint32_t address);
DllAccess void lscpciej_log_error(const char* msg, ...);
DllAccess void lscpciej_log_trace(const char* msg, ...);


#ifdef __cplusplus
}
#endif
