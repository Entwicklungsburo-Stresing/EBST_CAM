#include "UIAbstractionLayer_cpp.h"
#include "lsc-gui.h"

void notifyMeasureStartCpp(uint32_t drv)
{
	emit mainWindow->lsc.measureStart();
	return;
}

void notifyMeasureDoneCpp(uint32_t drv)
{
	emit mainWindow->lsc.measureDone();
	return;
}

void notifyBlockStartCpp(uint32_t drv)
{
	emit mainWindow->lsc.blockStart();
	return;
}

void notifyBlockDoneCpp(uint32_t drv)
{
	emit mainWindow->lsc.blockDone();
	return;
}