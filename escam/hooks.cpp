#include "hooks.h"
#include <chrono>
#include "lsc-gui.h"

std::chrono::steady_clock::time_point timepoint_measureStart;
std::chrono::steady_clock::time_point timepoint_measureDone;
std::chrono::steady_clock::time_point timepoint_blockStart;
std::chrono::steady_clock::time_point timepoint_blockDone;
std::chrono::steady_clock::time_point timepoint_allBlocksDone;
const int64_t min_diff_in_ms = 50;

void emitMeasureStartSignal()
{
	auto timepoint_measureStart_new = std::chrono::steady_clock::now();
	int64_t diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_measureStart_new - timepoint_measureStart).count();
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.measureStart();
		timepoint_measureStart = timepoint_measureStart_new;
	}
	return;
}

void emitMeasureDoneSignal()
{
	auto timepoint_measureDone_new = std::chrono::steady_clock::now();
	int64_t diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_measureDone_new - timepoint_measureDone).count();
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.measureDone();
		timepoint_measureDone = timepoint_measureDone_new;
	}
	return;
}

void emitBlockStartSignal()
{
	auto timepoint_blockStart_new = std::chrono::steady_clock::now();
	int64_t diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_blockStart_new - timepoint_blockStart).count();
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.blockStart();
		timepoint_blockStart = timepoint_blockStart_new;
	}
	return;
}

void emitBlockDoneSignal()
{
	auto timepoint_blockDone_new = std::chrono::steady_clock::now();
	int64_t diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_blockDone_new - timepoint_blockDone).count();
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.blockDone();
		timepoint_blockDone = timepoint_blockDone_new;
	}
	return;
}

void emitAllBlocksDoneSignal()
{
	auto timepoint_allBlocksDone_new = std::chrono::steady_clock::now();
	int64_t diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_allBlocksDone_new - timepoint_allBlocksDone).count();
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.allBlocksDone();
		timepoint_allBlocksDone = timepoint_allBlocksDone_new;
	}
	return;
}
