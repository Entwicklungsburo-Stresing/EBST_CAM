//UI abstraction layer for CCDExample
#include "shared_src/UIAbstractionLayer.h"
#include "UIAbstractionLayer_cpp.h"

void notifyMeasureStart( UINT32 drv )
{
	notifyMeasureStartCpp(drv);
	return;
}

void notifyMeasureDone( UINT32 drv )
{
	notifyMeasureDoneCpp(drv);
	return;
}

void notifyBlockStart( UINT32 drv )
{
	notifyBlockStartCpp(drv);
	return;
}

void notifyBlockDone( UINT32 drv )
{
	notifyBlockDoneCpp(drv);
	return;
}
