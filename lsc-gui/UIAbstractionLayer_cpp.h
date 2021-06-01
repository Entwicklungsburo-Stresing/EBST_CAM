#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void notifyMeasureStartCpp(uint32_t drv);
void notifyMeasureDoneCpp(uint32_t drv);
void notifyBlockStartCpp(uint32_t drv);
void notifyBlockDoneCpp(uint32_t drv);

#ifdef __cplusplus
}
#endif
