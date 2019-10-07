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
	void Direct2dViewer_Initialize(void* D2dV, HWND hWndParent);
	void Direct2dViewer_delete(void *D2dV);
	void Direct2dViewer_show(void *D2dV);
	void Direct2dViewer_setBitmapSource(void *D2dV, void *addr, UINT width, UINT height);
	HWND Direct2dViewer_getWindowHandler(void *D2dV);
	void Direct2dViewer_updateBitmap(void* D2dV);
#ifdef __cplusplus    
}
#endif