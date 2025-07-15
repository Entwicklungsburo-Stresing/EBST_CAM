/*****************************************************************//**
 * @file   hooks.cpp
 * @copydoc	hooks.h
 *********************************************************************/

#include "hooks.h"
#include <chrono>
#include "lsc-gui.h"

std::chrono::steady_clock::time_point timepoint_measureStart = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point timepoint_measureDone = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point timepoint_blockStart = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point timepoint_blockDone = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point timepoint_allBlocksDone = std::chrono::steady_clock::now();
const std::chrono::milliseconds min_diff_in_ms = std::chrono::milliseconds(100);

void emitMeasureStartSignal()
{
	auto timepoint_measureStart_new = std::chrono::steady_clock::now();
	std::chrono::milliseconds diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_measureStart_new - timepoint_measureStart);
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
	std::chrono::milliseconds diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_measureDone_new - timepoint_measureDone);
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.measureDone();
		timepoint_measureDone = timepoint_measureDone_new;
	}
	return;
}

void emitBlockStartSignal(uint32_t blockIndex)
{
	(void)blockIndex; // blockIndex is not used in this function
	auto timepoint_blockStart_new = std::chrono::steady_clock::now();
	std::chrono::milliseconds diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_blockStart_new - timepoint_blockStart);
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.blockStart();
		timepoint_blockStart = timepoint_blockStart_new;
	}
	return;
}

void emitBlockDoneSignal(uint32_t blockIndex)
{
	(void)blockIndex; // blockIndex is not used in this function
	auto timepoint_blockDone_new = std::chrono::steady_clock::now();
	std::chrono::milliseconds diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_blockDone_new - timepoint_blockDone);
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.blockDone();
		timepoint_blockDone = timepoint_blockDone_new;
	}
	return;
}

void emitAllBlocksDoneSignal(uint64_t measurementCnt)
{
	(void)measurementCnt; // measurementCnt is not used in this function
	auto timepoint_allBlocksDone_new = std::chrono::steady_clock::now();
	std::chrono::milliseconds diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timepoint_allBlocksDone_new - timepoint_allBlocksDone);
	if (diff_in_ms > min_diff_in_ms)
	{
		emit mainWindow->lsc.allBlocksDone();
		timepoint_allBlocksDone = timepoint_allBlocksDone_new;
	}
	return;
}
