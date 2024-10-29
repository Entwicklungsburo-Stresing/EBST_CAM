#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void notifyMeasureStart();
extern void notifyMeasureDone();
extern void notifyBlockStart();
extern void notifyBlockDone();
extern void notifyAllBlocksDone();

#ifdef __cplusplus
}
#endif