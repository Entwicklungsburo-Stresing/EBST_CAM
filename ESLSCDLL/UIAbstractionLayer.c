/*****************************************************************//**
 * @file   UIAbstractionLayer.c
 * @copydoc UIAbstractionLayer.h
 *********************************************************************/

#include "UIAbstractionLayer.h"
#include "ESLSCDLL.h"
#include "Board.h"
#ifdef COMPILE_FOR_LABVIEW
#include <windows.h>
int64_t timebuffer_new = 0;
int64_t timebuffer_measureStart = 0;
int64_t timebuffer_measureDone = 0;
int64_t timebuffer_blockStart = 0;
int64_t timebuffer_blockDone = 0;
int64_t timebuffer_allBlocksDone = 0;
const int64_t min_diff_in_ms = 50;

/**
 * @brief Function pointer type for LabVIEW's PostLVUserEvent
 * @details Matches the declaration in LabVIEW 2015/cintools/extcode.h:
 * TH_REENTRANT EXTERNC MgErr _FUNCC PostLVUserEvent(LVUserEventRef ref, void* data);
 * with MgErr = int32and _FUNCC = __cdecl on Windows. The signature must match exactly, otherwise the call corrupts the stack.
 */
typedef int32_t(__cdecl* PostLVUserEvent_t)(LVUserEventRef ref, void* data);

/**
 * @brief Address of PostLVUserEvent in the calling LabVIEW process. NULL if not found (yet).
 */
static PostLVUserEvent_t pPostLVUserEvent = NULL;

/**
 * @brief 1 after the lookup of PostLVUserEvent has been done, so it is only done once.
 */
static int postLVUserEventResolved = 0;

/**
 * @brief Post a LabVIEW user event without linking against labview.lib
 * @details PostLVUserEvent is exported by the LabVIEW development environment (LabVIEW.exe, the main module of the process)
 * or by the LabVIEW run-time engine (lvrt.dll or lvffrt.dll) used by built LabVIEW applications.
 * Previously labview.lib from LabVIEW cintools linked a stub into this DLL that looked up the function
 * at runtime. This function does the same lookup itself, so building the LabVIEW configurations does not need cintools.
 * The lookup is done once the first call. GetModuleHandleA only returns modules that are already loaded, nothing new is loeaded.
 * If no LabVIEW module is found, e.g. because the DLL is not called from LabVIEW, the event is skipped
 * @param[in] ref LabVIEW user event refnum, registered with DLLRegisterLVEvents.
 */
static void postLVUserEvent(LVUserEventRef ref)
{
	if (!postLVUserEventResolved)
	{
		const char* modules[] = { NULL, "lvrt.dll", "lvffrt.dll"};
		for (size_t i = 0; i < sizeof(modules) / sizeof(modules[0]) && !pPostLVUserEvent; i++)
		{
			HMODULE module = GetModuleHandleA(modules[i]);
			if (module)
				pPostLVUserEvent = (PostLVUserEvent_t)GetProcAddress(module, "PostLVUserEvent");
		}
		postLVUserEventResolved = 1;
	}
	if (pPostLVUserEvent)
		pPostLVUserEvent(ref, NULL);
}
#endif

void notifyMeasureStart()
{
#ifdef COMPILE_FOR_LABVIEW
	timebuffer_new = GetTimestampInMilliseconds();
	int64_t diff_in_ms = timebuffer_new - timebuffer_measureStart;
	if (diff_in_ms > min_diff_in_ms)
	{
		postLVUserEvent(measureStartLVEvent);
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
		postLVUserEvent(measureDoneLVEvent);
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
		postLVUserEvent(blockStartLVEvent);
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
		postLVUserEvent(blockDoneLVEvent);
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
		postLVUserEvent(allBlocksDoneLVEvent);	
		timebuffer_allBlocksDone = timebuffer_new;
	}
#endif
	return;
}