#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

void notifyMeasureStartCpp(UINT32 drv);
void notifyMeasureDoneCpp(UINT32 drv);
void notifyBlockStartCpp(UINT32 drv);
void notifyBlockDoneCpp(UINT32 drv);

#ifdef __cplusplus
}
#endif
