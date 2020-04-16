#pragma once
/** \file Direct2dViewer.h This is a Direct2D Viewer for bitmaps. It was created by using Microsofts example "SimpleDirect2DApplication", originally released under MIT license.
// https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/multimedia/Direct2D/SimpleDirect2DApplication
*/

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows 7 or later.
#define WINVER 0x0700       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows 7 or later.
#define _WIN32_WINNT 0x0700 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>

// C RunTime Header Files
#include <stdlib.h>

#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <string>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <d2d1_1.h>
/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/

template<class Interface>
inline void
SafeRelease(
	Interface **ppInterfaceToRelease
)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#define WM_2DVIEWER_CLOSED (WM_USER + 0x0001)
/******************************************************************
*                                                                 *
*  Direct2dViewer                                                 *
*                                                                 *
******************************************************************/

class Direct2dViewer
{
public:
	Direct2dViewer();
	~Direct2dViewer();

	HRESULT start2dViewer
	(
		HWND hWndParent,
		void *bitmapAddr,
		UINT width,
		UINT height
	);
	HRESULT showNewBitmap
	(
		void *addr,
		UINT width,
		UINT height
	);
	HRESULT reloadBitmap();
	HWND getWindowHandler();
	void SetGammaValue
	(
		UINT16 white,
		UINT16 black
	);
	UINT16 GetGammaWhite();
	UINT16 GetGammaBlack();

private:
	HRESULT Initialize( HWND hWndParent );
	void setBitmapSource
	(
		void *addr,
		UINT width,
		UINT height
	);
	HRESULT loadBitmap();
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();
	void DiscardDeviceResources();
	HRESULT OnRender();
	void OnResize(
		UINT width,
		UINT height
	);
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);
	HRESULT Load16bitGreyscaleBitmapFromMemory();
	void CalcCursorPos();
	void DrawScale();
	void DrawVerticalLine(
		D2D1_POINT_2F startPoint,
		FLOAT length,
		FLOAT strokeWidth
	);
	void DrawHorizontalLine(
		D2D1_POINT_2F startPoint,
		FLOAT length,
		FLOAT strokeWidth
	);
	void DrawNumber(
		D2D1_POINT_2F location, int number
	);
	HWND m_hwnd;
	ID2D1Factory *m_pD2DFactory;
	IWICImagingFactory *m_pWICFactory;
	IDWriteFactory *m_pDWriteFactory;
	ID2D1HwndRenderTarget *m_pRenderTarget;
	IDWriteTextFormat *m_pTextFormat;
	IDWriteTextFormat *m_pTextFormat_scale;
	ID2D1SolidColorBrush *m_pBlackBrush;
	ID2D1Bitmap *m_pBitmap;
	struct _Scale
	{
		FLOAT length = 12;
		FLOAT width = 1;
		FLOAT distance_x = 10;
		FLOAT distance_y = 10;
		int skip_x = 25;
		int skip_y = 5;
	} _scale;
	struct _Gamma
	{
		FLOAT amplitude = 1;
		INT32 offset = 0;
		UINT16 white = 0xFFFF;
		UINT16 black = 0;
	} _gamma;
	struct _Margin
	{
		FLOAT top = 0;
		FLOAT left = 35;
		FLOAT bottom = 50;
		FLOAT right = 0;
	} _margin;
	struct _CursorPos
	{
		// relative position to upper left corner of direct2d window
		int x = 0;
		int y = 0;
		// position in camera data
		int pixel = 0;
		int line = 0;
	} _cursorPos;
	struct _BitmapSource
	{
		void	*addr;
		UINT	width;
		UINT	height;
	} _bitmapSource;
};



