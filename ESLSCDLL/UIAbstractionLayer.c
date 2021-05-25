//UI abstraction layer for Labview
#include "shared_src/UIAbstractionLayer.h"
#include "ESLSCDLL.h"

void notifyMeasureStart( uint32_t drv )
{
	PostLVUserEvent( measureStartLVEvent, NULL );
	return;
}

void notifyMeasureDone( uint32_t drv )
{
	PostLVUserEvent( measureDoneLVEvent, NULL );
	return;
}

void notifyBlockStart( uint32_t drv )
{
	PostLVUserEvent( blockStartLVEvent, NULL );
	return;
}

void notifyBlockDone( uint32_t drv )
{
	PostLVUserEvent( blockDoneLVEvent, NULL );
	return;
}