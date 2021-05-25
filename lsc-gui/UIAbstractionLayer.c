//UI abstraction layer for CCDExample
#include "../shared_src/UIAbstractionLayer.h"
#include "UIAbstractionLayer_cpp.h"

void notifyMeasureStart( uint32_t drv )
{
	notifyMeasureStartCpp(drv);
	return;
}

void notifyMeasureDone( uint32_t drv )
{
	notifyMeasureDoneCpp(drv);
	return;
}

void notifyBlockStart( uint32_t drv )
{
	notifyBlockStartCpp(drv);
	return;
}

void notifyBlockDone( uint32_t drv )
{
	notifyBlockDoneCpp(drv);
	return;
}
