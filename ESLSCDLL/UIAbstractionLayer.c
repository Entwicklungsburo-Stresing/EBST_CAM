//UI abstraction Layer for Labview
#include "shared_src/UIAbstractionLayer.h"
#include "ESLSCDLL.h"

void notifyMeasureStart( UINT32 drv )
{
	PostLVUserEvent( measureStartLVEvent, NULL );
	return;
}

void notifyMeasureDone( UINT32 drv )
{
	PostLVUserEvent( measureDoneLVEvent, NULL );
	return;
}

void notifyBlockStart( UINT32 drv )
{
	PostLVUserEvent( blockStartLVEvent, NULL );
	return;
}

void notifyBlockDone( UINT32 drv )
{
	PostLVUserEvent( blockDoneLVEvent, NULL );
	return;
}