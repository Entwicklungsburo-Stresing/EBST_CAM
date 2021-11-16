//UI abstraction layer for Labview
#include "shared_src/UIAbstractionLayer.h"
#include "ESLSCDLL.h"
#include <sys/types.h>
#include <sys/timeb.h>

struct _timeb timebuffer_measureStart;
struct _timeb timebuffer_measureDone;
struct _timeb timebuffer_blockStart;
struct _timeb timebuffer_blockDone;
const int64_t min_diff_in_ms = 50;

void notifyMeasureStart()
{
#if COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_measureStart_new;
	ftime(&timebuffer_measureStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureStart_new.time - timebuffer_measureStart.time) + (timebuffer_measureStart_new.millitm - timebuffer_measureStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(measureStartLVEvent, NULL);
		ftime(&timebuffer_measureStart);
	}
#endif
	return;
}

void notifyMeasureDone()
{
#if COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_measureDone_new;
	ftime(&timebuffer_measureDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureDone_new.time - timebuffer_measureDone.time) + (timebuffer_measureDone_new.millitm - timebuffer_measureDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(measureDoneLVEvent, NULL);
		ftime(&timebuffer_measureDone);
	}
#endif
	return;
}

void notifyBlockStart()
{
#if COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_blockStart_new;
	ftime(&timebuffer_blockStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockStart_new.time - timebuffer_blockStart.time) + (timebuffer_blockStart_new.millitm - timebuffer_blockStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(blockStartLVEvent, NULL);
		ftime(&timebuffer_blockStart);
	}
#endif
	return;
}

void notifyBlockDone()
{
#if COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_blockDone_new;
	ftime(&timebuffer_blockDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockDone_new.time - timebuffer_blockDone.time) + (timebuffer_blockDone_new.millitm - timebuffer_blockDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(blockDoneLVEvent, NULL);
		ftime(&timebuffer_blockDone);
	}
#endif
	return;
}