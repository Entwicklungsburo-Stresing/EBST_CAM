//UI abstraction layer for CCDExample
#include "../shared_src/UIAbstractionLayer.h"
#include "UIAbstractionLayer_cpp.h"

void notifyMeasureStart()
{
	notifyMeasureStartCpp();
	return;
}

void notifyMeasureDone()
{
	notifyMeasureDoneCpp();
	return;
}

void notifyBlockStart()
{
	notifyBlockStartCpp();
	return;
}

void notifyBlockDone()
{
	notifyBlockDoneCpp();
	return;
}
