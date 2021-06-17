#pragma once
#include <stdint.h>

void notifyMeasureStart( uint32_t drv );
void notifyMeasureDone( uint32_t drv );
void notifyBlockStart( uint32_t drv );
void notifyBlockDone( uint32_t drv );
