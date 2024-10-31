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