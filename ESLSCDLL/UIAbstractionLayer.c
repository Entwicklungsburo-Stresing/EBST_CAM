//UI abstraction layer for Labview
#include "shared_src/UIAbstractionLayer.h"
#include "ESLSCDLL.h"

void notifyMeasureStart()
{
	PostLVUserEvent( measureStartLVEvent, NULL );
	return;
}

void notifyMeasureDone()
{
	PostLVUserEvent( measureDoneLVEvent, NULL );
	return;
}

void notifyBlockStart()
{
	PostLVUserEvent( blockStartLVEvent, NULL );
	return;
}

void notifyBlockDone()
{
	PostLVUserEvent( blockDoneLVEvent, NULL );
	return;
}