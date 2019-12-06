#pragma once

//cpp_connector.h is a wrapper of Direct2dViewer.cpp to provide c api
//inspired by this tutorial to call cpp methods from c: https://bytes.com/topic/c/insights/921728-calling-c-class-methods-c

#include <windows.h>
//also defined in Direct2dViewer.h
#define WM_2DVIEWER_CLOSED (WM_USER + 0x0001)

#ifdef __cplusplus
extern "C"
{
#endif
	void* Direct2dViewer_new();
	void Direct2dViewer_delete( void *D2dV );
	void Direct2dViewer_start2dViewer( void *D2dV, HWND hWndParent, void *bitmapAddr, UINT width, UINT height );
	void Direct2dViewer_showNewBitmap( void *D2dV, void *addr, UINT width, UINT height );
	void Direct2dViewer_setGammaValue( void *D2dV, UINT16 white, UINT16 black );
	HWND Direct2dViewer_getWindowHandler( void *D2dV );
#ifdef __cplusplus    
}
#endif