/*****************************************************************//**
 * @file   Direct2dViewer_c.h
 * @brief  Wrapper for Direct2dViewer.cpp to provide a C API.
 * 
 * Inspired by this tutorial to call cpp methods from c: https://bytes.com/topic/c/insights/921728-calling-c-class-methods-c
 * @author Florian Hahn
 * @date   30.09.2019
 *********************************************************************/

#pragma once
#ifndef MINIMAL_BUILD

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
	void Direct2dViewer_repaintWindow( void *D2dV );
	void Direct2dViewer_setGammaValue( void *D2dV, UINT16 white, UINT16 black );
	UINT16 Direct2dViewer_getGammaWhite( void *D2dV );
	UINT16 Direct2dViewer_getGammaBlack( void *D2dV );
	HWND Direct2dViewer_getWindowHandler( void *D2dV );
#ifdef __cplusplus
}
#endif

#endif
