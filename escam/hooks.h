/*****************************************************************//**
 * @file   hooks.h
 * @brief  C-wrapper of hooks_cpp.h for setting up the callback functions of ESLSCDLL.
 * 
 * @author Florian Hahn
 * @date   31.10.2024
 *********************************************************************/

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void emitMeasureStartSignal();
extern void emitMeasureDoneSignal();
extern void emitBlockStartSignal();
extern void emitBlockDoneSignal();
extern void emitAllBlocksDoneSignal();

#ifdef __cplusplus
}
#endif