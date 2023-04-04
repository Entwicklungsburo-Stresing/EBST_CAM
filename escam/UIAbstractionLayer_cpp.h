#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void notifyMeasureStartCpp();
void notifyMeasureDoneCpp();
void notifyBlockStartCpp();
void notifyBlockDoneCpp();
void notifyAllBlocksDoneCpp();

#ifdef __cplusplus
}
#endif
