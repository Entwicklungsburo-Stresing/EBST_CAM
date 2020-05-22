#include <windows.h>
#include "Direct2dViewer_c.h"
// Make this data shared among all 
// all applications that use this DLL.
//....................................
#pragma data_seg( ".GLOBALS" )
int nProcessCount = 0;
int nThreadCount = 0;
//#pragma data_seg()
void	*dummy;

#include "GLOBAL.H"
#include "shared_src/board.c"

BOOL WINAPI DLLMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );
DllAccess void DLLInitProDll( UINT64 ppDMABigBufBase );
//************  2d greyscale viewer
DllAccess void DLLStart2dViewer( UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos );
DllAccess void DLLShowNewBitmap( UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos );
DllAccess void DLLDeinit2dViewer();
DllAccess void DLLSetGammaValue( UINT16 white, UINT16 black );