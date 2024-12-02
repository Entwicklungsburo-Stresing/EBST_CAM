/*****************************************************************//**
 * @file   hooks.h
 * @brief  Functions for setting up the callback functions of ESLSCDLL.
 * 
 * @author Florian Hahn
 * @date   31.10.2024
 *********************************************************************/

#pragma once
#include <stdint.h>

extern void emitMeasureStartSignal();
extern void emitMeasureDoneSignal();
extern void emitBlockStartSignal();
extern void emitBlockDoneSignal();
extern void emitAllBlocksDoneSignal();
