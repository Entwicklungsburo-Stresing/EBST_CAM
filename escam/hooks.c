#include "hooks.h"
#include "hooks_cpp.h"
#include <sys/types.h>
#include <sys/timeb.h>

struct timeb timebuffer_measureStart;
struct timeb timebuffer_measureDone;
struct timeb timebuffer_blockStart;
struct timeb timebuffer_blockDone;
struct timeb timebuffer_allBlocksDone;
const int64_t min_diff_in_ms = 50;

void emitMeasureStartSignal()
{
	struct timeb timebuffer_measureStart_new;
	ftime(&timebuffer_measureStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureStart_new.time - timebuffer_measureStart.time) + (timebuffer_measureStart_new.millitm - timebuffer_measureStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		emitMeasureStartSignalCpp();
		ftime(&timebuffer_measureStart);
	}
	return;
}

void emitMeasureDoneSignal()
{
	struct timeb timebuffer_measureDone_new;
	ftime(&timebuffer_measureDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureDone_new.time - timebuffer_measureDone.time) + (timebuffer_measureDone_new.millitm - timebuffer_measureDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		emitMeasureDoneSignalCpp();
		ftime(&timebuffer_measureDone);
	}
	return;
}

void emitBlockStartSignal()
{
	struct timeb timebuffer_blockStart_new;
	ftime(&timebuffer_blockStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockStart_new.time - timebuffer_blockStart.time) + (timebuffer_blockStart_new.millitm - timebuffer_blockStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		emitBlockStartSignalCpp();
		ftime(&timebuffer_blockStart);
	}
	return;
}

void emitBlockDoneSignal()
{
	struct timeb timebuffer_blockDone_new;
	ftime(&timebuffer_blockDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockDone_new.time - timebuffer_blockDone.time) + (timebuffer_blockDone_new.millitm - timebuffer_blockDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		emitBlockDoneSignalCpp();
		ftime(&timebuffer_blockDone);
	}
	return;
}

void emitAllBlocksDoneSignal()
{
	struct timeb timebuffer_allBlocksDone_new;
	ftime(&timebuffer_allBlocksDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_allBlocksDone_new.time - timebuffer_allBlocksDone.time) + (timebuffer_allBlocksDone_new.millitm - timebuffer_allBlocksDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		emitAllBlocksDoneSignalCpp();
		ftime(&timebuffer_allBlocksDone);
	}
	return;
}
