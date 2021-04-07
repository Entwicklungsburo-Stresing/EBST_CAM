#include "UIAbstractionLayer_cpp.h"
#include "lsc-gui.h"

void notifyMeasureStartCpp(UINT32 drv)
{
	emit mainWindow->lsc.measureStart();
	return;
}

void notifyMeasureDoneCpp(UINT32 drv)
{
	emit mainWindow->lsc.measureDone();
	return;
}

void notifyBlockStartCpp(UINT32 drv)
{
	emit mainWindow->lsc.blockStart();
	return;
}

void notifyBlockDoneCpp(UINT32 drv)
{
	emit mainWindow->lsc.blockDone();
	return;
}