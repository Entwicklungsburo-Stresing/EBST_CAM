/*****************************************************************//**
 * @file   hooks.h
 * @brief  Functions for setting up the callback functions of ESLSCDLL.
 *
 * @author Florian Hahn
 * @date   31.10.2024
 *********************************************************************/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void emitMeasureStartSignalCpp();
void emitMeasureDoneSignalCpp();
void emitBlockStartSignalCpp();
void emitBlockDoneSignalCpp();
void emitAllBlocksDoneSignalCpp();

#ifdef __cplusplus
}
#endif
