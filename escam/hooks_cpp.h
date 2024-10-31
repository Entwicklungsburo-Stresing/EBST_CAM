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
