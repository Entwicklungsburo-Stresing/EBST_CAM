#include "UIAbstractionLayer_cpp.h"
#include "lsc-gui.h"

void notifyMeasureStartCpp()
{
	emit mainWindow->lsc.measureStart();
	return;
}

void notifyMeasureDoneCpp()
{
	emit mainWindow->lsc.measureDone();
	return;
}

void notifyBlockStartCpp()
{
	emit mainWindow->lsc.blockStart();
	return;
}

void notifyBlockDoneCpp()
{
	emit mainWindow->lsc.blockDone();
	return;
}

void notifyAllBlocksDoneCpp()
{
	emit mainWindow->lsc.allBlocksDone();
	return;
}
