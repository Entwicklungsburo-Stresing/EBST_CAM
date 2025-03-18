/*****************************************************************//**
 * @file   UIAbstractionLayer.c
 * @copydoc UIAbstractionLayer.h
 *********************************************************************/

#include "UIAbstractionLayer.h"
#include "ESLSCDLL.h"
#include "Board.h"
#ifdef COMPILE_FOR_LABVIEW
int64_t timebuffer_new = 0;
int64_t timebuffer_measureStart = 0;
int64_t timebuffer_measureDone = 0;
int64_t timebuffer_blockStart = 0;
int64_t timebuffer_blockDone = 0;
int64_t timebuffer_allBlocksDone = 0;
const int64_t min_diff_in_ms = 50;
#endif

void notifyMeasureStart()
{
#ifdef COMPILE_FOR_LABVIEW
	timebuffer_new = GetTimestampInMilliseconds();
	int64_t diff_in_ms = timebuffer_new - timebuffer_measureStart;
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(measureStartLVEvent, NULL);
		timebuffer_measureStart = timebuffer_new;
	}
#endif
	return;
}

void notifyMeasureDone()
{
#ifdef COMPILE_FOR_LABVIEW
	timebuffer_new = GetTimestampInMilliseconds();
	int64_t diff_in_ms = timebuffer_new - timebuffer_measureDone;
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(measureDoneLVEvent, NULL);
		timebuffer_measureDone = timebuffer_new;
	}
#endif
	return;
}

void notifyBlockStart()
{
#ifdef COMPILE_FOR_LABVIEW
	timebuffer_new = GetTimestampInMilliseconds();
	int64_t diff_in_ms = timebuffer_new - timebuffer_blockStart;
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(blockStartLVEvent, NULL);
		timebuffer_blockStart = timebuffer_new;
	}
#endif
	return;
}

void notifyBlockDone()
{
#ifdef COMPILE_FOR_LABVIEW
	timebuffer_new = GetTimestampInMilliseconds();
	int64_t diff_in_ms = timebuffer_new - timebuffer_blockDone;
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(blockDoneLVEvent, NULL);
		timebuffer_blockDone = timebuffer_new;
	}
#endif
	return;
}

void notifyAllBlocksDone()
{
#ifdef COMPILE_FOR_LABVIEW
	timebuffer_new = GetTimestampInMilliseconds();
	int64_t diff_in_ms = timebuffer_new - timebuffer_allBlocksDone;
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(allBlocksDoneLVEvent, NULL);
		timebuffer_allBlocksDone = timebuffer_new;
	}
#endif
	return;
}