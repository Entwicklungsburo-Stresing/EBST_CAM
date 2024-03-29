//UI abstraction layer for Labview
#include "shared_src/UIAbstractionLayer.h"
#include "ESLSCDLL.h"
#ifdef COMPILE_FOR_LABVIEW
#include <sys/types.h>
#include <sys/timeb.h>

struct _timeb timebuffer_measureStart;
struct _timeb timebuffer_measureDone;
struct _timeb timebuffer_blockStart;
struct _timeb timebuffer_blockDone;
struct _timeb timebuffer_allBlocksDone;
const int64_t min_diff_in_ms = 50;
#endif

void notifyMeasureStart()
{
#ifdef COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_measureStart_new;
	ftime((struct timeb *const)&timebuffer_measureStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureStart_new.time - timebuffer_measureStart.time) + (timebuffer_measureStart_new.millitm - timebuffer_measureStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(measureStartLVEvent, NULL);
		ftime((struct timeb* const)&timebuffer_measureStart);
	}
#endif
	return;
}

void notifyMeasureDone()
{
#ifdef COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_measureDone_new;
	ftime((struct timeb* const)&timebuffer_measureDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureDone_new.time - timebuffer_measureDone.time) + (timebuffer_measureDone_new.millitm - timebuffer_measureDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(measureDoneLVEvent, NULL);
		ftime((struct timeb* const)&timebuffer_measureDone);
	}
#endif
	return;
}

void notifyBlockStart()
{
#ifdef COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_blockStart_new;
	ftime((struct timeb* const)&timebuffer_blockStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockStart_new.time - timebuffer_blockStart.time) + (timebuffer_blockStart_new.millitm - timebuffer_blockStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(blockStartLVEvent, NULL);
		ftime((struct timeb* const)&timebuffer_blockStart);
	}
#endif
	return;
}

void notifyBlockDone()
{
#ifdef COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_blockDone_new;
	ftime((struct timeb* const)&timebuffer_blockDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockDone_new.time - timebuffer_blockDone.time) + (timebuffer_blockDone_new.millitm - timebuffer_blockDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(blockDoneLVEvent, NULL);
		ftime((struct timeb* const)&timebuffer_blockDone);
	}
#endif
	return;
}

void notifyAllBlocksDone()
{
#ifdef COMPILE_FOR_LABVIEW
	struct _timeb timebuffer_allBlocksDone_new;
	ftime((struct timeb* const)&timebuffer_allBlocksDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_allBlocksDone_new.time - timebuffer_allBlocksDone.time) + (timebuffer_allBlocksDone_new.millitm - timebuffer_allBlocksDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		PostLVUserEvent(allBlocksDoneLVEvent, NULL);
		ftime((struct timeb* const)&timebuffer_allBlocksDone);
	}
#endif
	return;
}