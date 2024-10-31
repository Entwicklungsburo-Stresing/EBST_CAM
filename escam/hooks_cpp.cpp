#include "hooks_cpp.h"
#include "lsc-gui.h"

void emitMeasureStartSignalCpp()
{
	emit mainWindow->lsc.measureStart();
	return;
}

void emitMeasureDoneSignalCpp()
{
	emit mainWindow->lsc.measureDone();
	return;
}

void emitBlockStartSignalCpp()
{
	emit mainWindow->lsc.blockStart();
	return;
}

void emitBlockDoneSignalCpp()
{
	emit mainWindow->lsc.blockDone();
	return;
}

void emitAllBlocksDoneSignalCpp()
{
	emit mainWindow->lsc.allBlocksDone();
	return;
}
