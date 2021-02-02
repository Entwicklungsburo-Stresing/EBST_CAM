#pragma once
#include <windows.h>

// Microsoft C/C++ specific import/export specifier.
// These take the place of the EXPORTS and IMPORTS
// statements in the application and DLL .DEF files.
//..................................................
//
#ifdef _DLL
#define DllAccess __declspec( dllexport )
#else
#define DllAccess __declspec( dllimport )
#endif

BOOL WINAPI DLLMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );
DllAccess void DLLInitGlobals( struct global_vars g );
//************  2d greyscale viewer
DllAccess void DLLStart2dViewer( UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos );
DllAccess void DLLShowNewBitmap( UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos );
DllAccess void DLLDeinit2dViewer();
DllAccess void DLLSetGammaValue( UINT16 white, UINT16 black );
DllAccess UINT16 DLLGetGammaWhite();
DllAccess UINT16 DLLGetGammaBlack();
//************  Area and Region of Interest
DllAccess UINT8 DLLSetupROI( UINT32 drvno, UINT16 number_of_regions, UINT32 lines, UINT8 keep_first, UINT8* region_size, UINT8 vfreq );
DllAccess UINT8 DLLSetupArea( UINT32 drvno, UINT32 lines_binning, UINT8 vfreq );
