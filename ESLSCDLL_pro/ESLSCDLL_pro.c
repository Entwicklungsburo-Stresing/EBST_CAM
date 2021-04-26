/*
/*
This file is part of ESLSCDLL.

ESLSCDLL_pro is not free software :
all Code is under

Copyright 2021 ©Entwicklungsbuero G. Stresing (http://www.stresing.de/)

you can use it as is, but
for further sale ask Entwicklungsbuero G. Stresing for permission

ESLSCDLL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/

#include "shared_src/ESLSCDLL_pro.h"
#include "shared_src/Direct2dViewer_c.h"
// Make this data shared among all
// all applications that use this DLL.
//....................................
#pragma data_seg( ".GLOBALS" )
int nProcessCount = 0;
int nThreadCount = 0;

#include "shared_src/board.h"

void	*Direct2dViewer = NULL;

/**
\brief DllMain entry point

An optional entry point into a dynamic-link library (DLL). When the system starts or terminates a process or thread, it calls the entry-point function for each loaded DLL using the first thread of the process. The system also calls the entry-point function for a DLL when it is loaded or unloaded using the LoadLibrary and FreeLibrary functions. More information: https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain
\param[in] hinstDLL A handle to the DLL module. The value is the base address of the DLL. The HINSTANCE of a DLL is the same as the HMODULE of the DLL, so hinstDLL can be used in calls to functions that require a module handle.
\param[in] fdwReason	The reason code that indicates why the DLL entry-point function is being called. This parameter can be one of the following values:
	- DLL_PROCESS_ATTACH 1: The DLL is being loaded into the virtual address space of the current process as a result of the process starting up or as a result of a call to LoadLibrary. DLLs can use this opportunity to initialize any instance data or to use the TlsAlloc function to allocate a thread local storage (TLS) index. The lpReserved parameter indicates whether the DLL is being loaded statically or dynamically.
	- DLL_PROCESS_DETACH 0: The DLL is being unloaded from the virtual address space of the calling process because it was loaded unsuccessfully or the reference count has reached zero (the processes has either terminated or called FreeLibrary one time for each time it called LoadLibrary). The lpReserved parameter indicates whether the DLL is being unloaded as a result of a FreeLibrary call, a failure to load, or process termination. The DLL can use this opportunity to call the TlsFree function to free any TLS indices allocated by using TlsAlloc and to free any thread local data. Note that the thread that receives the DLL_PROCESS_DETACH notification is not necessarily the same thread that received the DLL_PROCESS_ATTACH notification.
	- DLL_THREAD_ATTACH 2: The current process is creating a new thread. When this occurs, the system calls the entry-point function of all DLLs currently attached to the process. The call is made in the context of the new thread. DLLs can use this opportunity to initialize a TLS slot for the thread. A thread calling the DLL entry-point function with DLL_PROCESS_ATTACH does not call the DLL entry-point function with DLL_THREAD_ATTACH. Note that a DLL's entry-point function is called with this value only by threads created after the DLL is loaded by the process. When a DLL is loaded using LoadLibrary, existing threads do not call the entry-point function of the newly loaded DLL.
	- DLL_THREAD_DETACH 3: A thread is exiting cleanly. If the DLL has stored a pointer to allocated memory in a TLS slot, it should use this opportunity to free the memory. The system calls the entry-point function of all currently loaded DLLs with this value. The call is made in the context of the exiting thread.
\param[in] lpvReserved If fdwReason is DLL_PROCESS_ATTACH, lpvReserved is NULL for dynamic loads and non-NULL for static loads. If fdwReason is DLL_PROCESS_DETACH, lpvReserved is NULL if FreeLibrary has been called or the DLL load failed and non-NULL if the process is terminating.
\return When the system calls the DllMain function with the DLL_PROCESS_ATTACH value, the function returns TRUE if it succeeds or FALSE if initialization fails. If the return value is FALSE when DllMain is called because the process uses the LoadLibrary function, LoadLibrary returns NULL. (The system immediately calls your entry-point function with DLL_PROCESS_DETACH and unloads the DLL.) If the return value is FALSE when DllMain is called during process initialization, the process terminates with an error. To get extended error information, call GetLastError. When the system calls the DllMain function with any value other than DLL_PROCESS_ATTACH, the return value is ignored.
*/
BOOL WINAPI DLLMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// DLL initialization code goes here. Formerly this 
		// would be in the LibMain function of a 16-bit DLL.
		//..................................................
		nProcessCount++;
		return(TRUE);
	case DLL_PROCESS_DETACH:
		// DLL cleanup code goes here. Formerly this would
		// be in the WEP function of a 16-bit DLL.
		//................................................
	 //	CCDDrvExit(DRV);
		nProcessCount--;
		break;
	case DLL_THREAD_ATTACH:
		// Special initialization code for new threads goes here.
		// This is so the DLL can "Thread Protect" itself.
		//.......................................................
		nThreadCount++;
		break;
	case DLL_THREAD_DETACH:
		// Special cleanup code for threads goes here.
		//............................................
		nThreadCount--;
		break;
	}
	return(FALSE);
}

/**
\brief Call before using pro DLL.
\param g global_vars struct, defined in Board.h.
\return none
*/
DllAccess void DLLInitGlobals( struct global_vars g )
{
	userBuffer = g.userBuffer;
	hDev = g.hDev;
	aPIXEL = g.aPIXEL;
	aCAMCNT = g.aCAMCNT;
	Nospb = g.Nospb;
	return;
}

/**
\brief Start 2d viewer.
\param drvno board number
\param cur_nob current number of block
\param cam which camera to display (when camcnt is >1)
\param pixel count of pixel of one line
\param nos samples in one block
\return none
*/
DllAccess void DLLStart2dViewer( UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos )
{
	if (Direct2dViewer == NULL)
	{
		Direct2dViewer = Direct2dViewer_new();
		UINT16* address = NULL;
		GetAddressOfPixel(drvno, 0, 0, cur_nob, cam, &address);
		Direct2dViewer_start2dViewer(
			Direct2dViewer,
			GetActiveWindow(),
			address,
			pixel,
			nos );
	}
	return;
}

/**
\brief Update the displayed bitmap.
\param drvno board number
\param cur_nob current number of blocks
\param cam which camera to display (when camcnt is >1)
\param pixel count of pixel of one line
\param nos samples in one block
\return none
*/
DllAccess void DLLShowNewBitmap( UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos )
{
	if (Direct2dViewer != NULL)
	{
		UINT16* address = NULL;
		GetAddressOfPixel(drvno, 0, 0, cur_nob, cam, &address);
		Direct2dViewer_showNewBitmap(
			Direct2dViewer,
			address,
			pixel,
			nos );
	}
	return;
}

/**
\brief Call when closing 2d viewer or at least before opening a new 2d viewer.
\return none
*/
DllAccess void DLLDeinit2dViewer()
{
	if (Direct2dViewer != NULL)
	{
		SendMessage( Direct2dViewer_getWindowHandler( Direct2dViewer ), WM_CLOSE, NULL, NULL );
		Direct2dViewer_delete( Direct2dViewer );
		Direct2dViewer = NULL;
	}
	return;
}

/**
\copydoc Direct2dViewer_setGammaValue
*/
DllAccess void DLLSetGammaValue( UINT16 white, UINT16 black )
{
	if (Direct2dViewer != NULL)
	{
		Direct2dViewer_setGammaValue( Direct2dViewer, white, black );
		Direct2dViewer_repaintWindow( Direct2dViewer );
	}
	return;
}

/**
\copydoc Direct2dViewer_getGammaWhite
*/
DllAccess UINT16 DLLGetGammaWhite()
{
	if (Direct2dViewer != NULL)
	{
		return Direct2dViewer_getGammaWhite( Direct2dViewer );
	}
	return NULL;
}

/**
\copydoc Direct2dViewer_getGammaBlack
*/
DllAccess UINT16 DLLGetGammaBlack()
{
	if (Direct2dViewer != NULL)
	{
		return Direct2dViewer_getGammaBlack( Direct2dViewer );
	}
	return NULL;
}

/**
 * \brief Initializes region of interest.
 * 
 * \param drvno PCIe identifier
 * \param number_of_regions determines how many region of interests are initialized, choose 2 to 8
 * \param lines number of total lines in camera
 * \param keep_first kept regions are alternating, determine whether first is kept
 * \param region_size determines the size of each region. array of size number_of_regions.
 * 	When region_size[0]==0 the lines are equally distributed for all regions.
 * 	I don't know what happens when  region_size[0]!=0 and region_size[1]==0. Maybe don't do this.
 * 	The sum of all regions should equal lines.
 * \param vfreq VCLK frequency
 * \return es_status_codes
 * 		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
DllAccess es_status_codes DLLSetupROI( UINT32 drvno, UINT16 number_of_regions, UINT32 lines, UINT8 keep_first, UINT8* region_size, UINT8 vfreq )
{
	BOOL keep = keep_first;
	es_status_codes status = es_no_error;
	// calculate how many lines are in each region when equally distributed
	UINT32 lines_per_region = lines / number_of_regions;
	// calculate the rest of lines when equally distributed
	UINT32 lines_in_last_region = lines - lines_per_region * (number_of_regions - 1);
	WDC_Err( "Setup ROI: lines_per_region: %u , lines_in_last_region: %u\n", lines_per_region, lines_in_last_region );
	// go from region 1 to number_of_regions
	for (int i = 1; i <= number_of_regions; i++)
	{
		// check whether lines should be distributed equally or by custom region size
		if (*region_size == 0)
		{
			if (i == number_of_regions) status = SetupVPB( drvno, i, lines_in_last_region, keep );
			else status = SetupVPB( drvno, i, lines_per_region, keep );
		}
		else
		{
			status = SetupVPB( drvno, i, *(region_size + (i - 1)), keep );
		}
		if (status != es_no_error) return status;
		keep = !keep;
	}
	status = SetupVCLKReg( drvno, lines, vfreq );
	if (status != es_no_error) return status;
	status = SetPartialBinning( drvno, 0 ); //I don't know why there first is 0 written, I just copied it from Labview. - FH
	if (status != es_no_error) return status;
	status = SetPartialBinning( drvno, number_of_regions );
	if (status != es_no_error) return status;
	useSWTrig = TRUE;
	return SetSTI( drvno, sti_ASL);
}

/**
 * \brief For FFTs: Setup area mode.
 * 
 * \param drvno PCIe board identifier.
 * \param lines_binning Determines how many lines are binned (summed) when reading camera in area mode.
 * \param vfreq Frequency for vertical clock.
 * \return es_status_codes
 * 		- es_no_error
 * 		- es_register_read_failed
 * 		- es_register_write_failed
 */
DllAccess es_status_codes DLLSetupArea( UINT32 drvno, UINT32 lines_binning, UINT8 vfreq )
{
	es_status_codes status = SetupVCLKReg( drvno, lines_binning, vfreq );
	if (status != es_no_error) return status;
	status = SetSTI(drvno, sti_ASL);
	if (status != es_no_error) return status;
	useSWTrig = TRUE; //software starts 1st scan
	ErrorMsg("setup area");
	return ResetPartialBinning( drvno );
}