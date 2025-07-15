/*****************************************************************//**
 * @file		hooks.h
 * @brief		Functions for setting up the callback functions of ESLSCDLL.
 * @author		Florian Hahn
 * @date		31.10.2024
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <stdint.h>

extern void emitMeasureStartSignal();
extern void emitMeasureDoneSignal();
extern void emitBlockStartSignal(uint32_t blockIndex);
extern void emitBlockDoneSignal(uint32_t blockIndex);
extern void emitAllBlocksDoneSignal(uint64_t measurementCnt);
