#pragma once

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

// C RunTime Header Files
#include <stdlib.h>

#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

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
	HWND getWindowHandler();
	void SetGammaValue
	(
		UINT16 white,
		UINT16 black
	);

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
	void CreateEffect();
	void ScaleRenderTarget();
	void DiscardDeviceResources();
	HRESULT OnRender();
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);
	HRESULT Load16bitGreyscaleBitmapFromMemory(

	);

private:
	HWND m_hwnd;
	ID2D1Factory *m_pD2DFactory;
	IWICImagingFactory *m_pWICFactory;
	IDWriteFactory *m_pDWriteFactory;
	ID2D1HwndRenderTarget *m_pRenderTarget;
	ID2D1DeviceContext *m_pDeviceContext;
	ID2D1Bitmap *m_pBitmap;
	ID2D1Effect *linearTransferEffect;
	FLOAT _gamma_amplitude = 1;
	INT32 _gamma_offset = 0;

	struct _BitmapSource
	{
		void	*addr;
		UINT	width;
		UINT	height;
	} _bitmapSource;
};


