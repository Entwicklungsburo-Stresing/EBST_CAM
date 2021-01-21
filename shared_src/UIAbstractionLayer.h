#pragma once
#include <Windows.h>

void notifyMeasureStart( UINT32 drv );
void notifyMeasureDone( UINT32 drv );
void notifyBlockStart( UINT32 drv );
void notifyBlockDone( UINT32 drv );