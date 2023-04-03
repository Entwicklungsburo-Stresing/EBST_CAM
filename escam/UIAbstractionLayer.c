//UI abstraction layer for CCDExample
#include "../shared_src/UIAbstractionLayer.h"
#include "UIAbstractionLayer_cpp.h"
#include <sys/types.h>
#include <sys/timeb.h>

struct timeb timebuffer_measureStart;
struct timeb timebuffer_measureDone;
struct timeb timebuffer_blockStart;
struct timeb timebuffer_blockDone;
struct timeb timebuffer_allBlocksDone;
const int64_t min_diff_in_ms = 50;

void notifyMeasureStart()
{
	struct timeb timebuffer_measureStart_new;
	ftime(&timebuffer_measureStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureStart_new.time - timebuffer_measureStart.time) + (timebuffer_measureStart_new.millitm - timebuffer_measureStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		notifyMeasureStartCpp();
		ftime(&timebuffer_measureStart);
	}
	return;
}

void notifyMeasureDone()
{
	struct timeb timebuffer_measureDone_new;
	ftime(&timebuffer_measureDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_measureDone_new.time - timebuffer_measureDone.time) + (timebuffer_measureDone_new.millitm - timebuffer_measureDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		notifyMeasureDoneCpp();
		ftime(&timebuffer_measureDone);
	}
	return;
}

void notifyBlockStart()
{
	struct timeb timebuffer_blockStart_new;
	ftime(&timebuffer_blockStart_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockStart_new.time - timebuffer_blockStart.time) + (timebuffer_blockStart_new.millitm - timebuffer_blockStart.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		notifyBlockStartCpp();
		ftime(&timebuffer_blockStart);
	}
	return;
}

void notifyBlockDone()
{
	struct timeb timebuffer_blockDone_new;
	ftime(&timebuffer_blockDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_blockDone_new.time - timebuffer_blockDone.time) + (timebuffer_blockDone_new.millitm - timebuffer_blockDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		notifyBlockDoneCpp();
		ftime(&timebuffer_blockDone);
	}
	return;
}

void notifyAllBlocksDone()
{
	struct timeb timebuffer_allBlocksDone_new;
	ftime(&timebuffer_allBlocksDone_new);
	int64_t diff_in_ms = (int64_t)(1000.0 * (timebuffer_allBlocksDone_new.time - timebuffer_allBlocksDone.time) + (timebuffer_allBlocksDone_new.millitm - timebuffer_allBlocksDone.millitm));
	if (diff_in_ms > min_diff_in_ms)
	{
		notifyAllBlocksDoneCpp();
		ftime(&timebuffer_allBlocksDone);
	}
	return;
}
