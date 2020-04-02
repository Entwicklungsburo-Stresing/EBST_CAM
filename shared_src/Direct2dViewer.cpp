// This is a Direct2D Viewer for bitmaps used in CCD example. It was created by using Microsofts example "SimpleDirect2DApplication", originally released under MIT license.
// https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/multimedia/Direct2D/SimpleDirect2DApplication

/*
This file is part of CCDExamp and ECLSCDLL.

CCDExamp and ECLSCDLL are free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CCDExamp and ECLSCDLL are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see < http://www.gnu.org/licenses/>.

Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
*/

#include "Direct2dViewer.h"

//
// Initialize members.
//
Direct2dViewer::Direct2dViewer() :
	m_hwnd( NULL ),
	m_pD2DFactory( NULL ),
	m_pWICFactory( NULL ),
	m_pDWriteFactory( NULL ),
	m_pRenderTarget( NULL ),
	m_pTextFormat( NULL ),
	m_pBitmap( NULL ),
	m_pBlackBrush( NULL )
{
}

//
// Release resources.
//
Direct2dViewer::~Direct2dViewer()
{
	SafeRelease( &m_pD2DFactory );
	SafeRelease( &m_pWICFactory );
	SafeRelease( &m_pDWriteFactory );
	SafeRelease( &m_pRenderTarget );
	SafeRelease( &m_pTextFormat );
	SafeRelease( &m_pBitmap );
	SafeRelease( &m_pBlackBrush );
}

//
// Creates the application window and initializes
// device-independent resources.
//
HRESULT Direct2dViewer::Initialize( HWND hWndParent )
{
	HRESULT hr = S_OK;

	if (SUCCEEDED( hr )) {
		// Initialize device-indpendent resources, such
		// as the Direct2D factory.
		hr = CreateDeviceIndependentResources();
	}

	if (SUCCEEDED( hr ))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof( WNDCLASSEX ) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Direct2dViewer::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof( LONG_PTR );
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
		wcex.lpszClassName = L"Direct2dViewer";

		RegisterClassEx( &wcex );

		// Create the application window.
		//
		// Because the CreateWindow function takes its size in pixels, we
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi( &dpiX, &dpiY );

		// Create the application window.
		m_hwnd = CreateWindow(
			L"Direct2dViewer",
			L"2D Viewer",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<INT>(ceil( 640.f * dpiX / 96.f )),
			static_cast<INT>(ceil( 480.f * dpiY / 96.f )),
			hWndParent,
			NULL,
			HINST_THISCOMPONENT,
			this
		);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED( hr ))
		{
			ShowWindow( m_hwnd, SW_SHOWNORMAL );
			UpdateWindow( m_hwnd );
		}
	}

	return hr;
}

//
// Create resources which are not bound
// to any device. Their lifetime effectively extends for the
// duration of the app. These resources include the Direct2D,
// DirectWrite, and WIC factories; and a DirectWrite Text Format object
// (used for identifying particular font characteristics) and
// a Direct2D geometry.
//
HRESULT Direct2dViewer::CreateDeviceIndependentResources()
{
	static const WCHAR msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 20;
	HRESULT hr = S_OK;

	if (SUCCEEDED( hr )) {
		SafeRelease( &m_pD2DFactory );
		// Create a Direct2D factory.
		hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory );
	}
	if (SUCCEEDED( hr ))
	{
		SafeRelease( &m_pWICFactory );
		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICFactory)
		);
	}
	if (SUCCEEDED( hr ))
	{
		// Create a DirectWrite factory.
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
		);
	}
	if (SUCCEEDED( hr ))
	{
		// Create a DirectWrite text format object.
		hr = m_pDWriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			&m_pTextFormat
		);
	}
	if (SUCCEEDED( hr ))
	{
		// Center the text horizontally and vertically.
		m_pTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
		m_pTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_FAR );
	}
	return hr;
}

//
//  This method creates resources which are bound to a particular
//  Direct3D device. It's all centralized here, in case the resources
//  need to be recreated in case of Direct3D device loss (eg. display
//  change, remoting, removal of video card, etc).
//
HRESULT Direct2dViewer::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect( m_hwnd, &rc );

		D2D1_SIZE_U size = D2D1::SizeU(
			static_cast<UINT>(rc.right - rc.left),
			static_cast<UINT>(rc.bottom - rc.top)
		);
		if (SUCCEEDED( hr ))
		{
			SafeRelease( &m_pRenderTarget );
			// Create a Direct2D render target.
			hr = m_pD2DFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties( m_hwnd, size ),
				&m_pRenderTarget
			);
		}
		if (SUCCEEDED( hr ))
		{
			// Create a black brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF( D2D1::ColorF::Black ),
				&m_pBlackBrush
			);
		}
		if (SUCCEEDED( hr ))
		{
			hr = loadBitmap();
		}
	}

	return hr;
}

//
//  Discard device-specific resources which need to be recreated
//  when a Direct3D device is lost
//
void Direct2dViewer::DiscardDeviceResources()
{
	SafeRelease( &m_pRenderTarget );
	SafeRelease( &m_pBitmap );
}

//
//  Called whenever the application needs to display the client
//  window.
//
//  Note that this function will not render anything if the window
//  is occluded (e.g. when the screen is locked).
//  Also, this function will automatically discard device-specific
//  resources if the Direct3D device disappears during function
//  invocation, and will recreate the resources the next time it's
//  invoked.
//
HRESULT Direct2dViewer::OnRender()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED( hr )) {
		hr = CreateDeviceResources();
	}

	if (SUCCEEDED( hr ) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{
		CalcCursorPos();
		std::wstring position_string = L"pixel: " + std::to_wstring( _cursorPos.pixel ) + L"  line: " + std::to_wstring( _cursorPos.line+1 );
		// Retrieve the size of the render target.
		D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();

		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform( D2D1::Matrix3x2F::Identity() );
		m_pRenderTarget->Clear( D2D1::ColorF( D2D1::ColorF::White ) );
		m_pRenderTarget->DrawBitmap(
			m_pBitmap,
			D2D1::RectF(
				_margin.left,
				_margin.top,
				renderTargetSize.width-_margin.right,
				renderTargetSize.height-_margin.bottom ),
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
		);
		m_pRenderTarget->DrawText(
			position_string.c_str(),
			static_cast<UINT32>(position_string.size()),
			m_pTextFormat,
			D2D1::RectF( 0, 0, renderTargetSize.width, renderTargetSize.height ),
			m_pBlackBrush
		);
		//m_pRenderTarget->DrawLine(
		//	D2D1::Point2F( 0.0f, 0.0f ),
		//	D2D1::Point2F( renderTargetSize.width, renderTargetSize.height ),
		//	m_pBlackBrush,
		//	0.5f,
		//	NULL
		//);
		hr = m_pRenderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}
	}

	return hr;
}

//
//  If the application receives a WM_SIZE message, this method
//  resize the render target appropriately.
//
void Direct2dViewer::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;

		// Note: This method can fail, but it's okay to ignore the
		// error here -- it will be repeated on the next call to
		// EndDraw.
		m_pRenderTarget->Resize(size);
	}
}


//
// The window message handler.
//
LRESULT CALLBACK Direct2dViewer::WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		Direct2dViewer *D2DV = (Direct2dViewer *)pcs->lpCreateParams;
		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(D2DV)
		);
	}
	else
	{
		Direct2dViewer *D2DV = reinterpret_cast<Direct2dViewer *>(::GetWindowLongPtr( hwnd, GWLP_USERDATA ));

		bool wasHandled = false;

		if (D2DV)
		{
			switch (message)
			{
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				D2DV->OnResize(width, height);
			}
				result = 0;
				wasHandled = true;
				break;
			case WM_MOUSEMOVE:
				D2DV->_cursorPos.x = GET_X_LPARAM( lParam );
				D2DV->_cursorPos.y = GET_Y_LPARAM( lParam );
			case WM_PAINT:
			case WM_DISPLAYCHANGE:
			{
				PAINTSTRUCT ps;
				BeginPaint( hwnd, &ps );
				D2DV->OnRender();
				EndPaint( hwnd, &ps );
			}
			result = 0;
			wasHandled = true;
			break;
			case WM_DESTROY:
				//send custom message to main window about closing 2d viewer
				HWND hWndOwner = GetWindow( hwnd, GW_OWNER );
				SendMessage( hWndOwner, WM_2DVIEWER_CLOSED, NULL, NULL );
				result = 1;
				wasHandled = true;
				break;
			}
		}
		if (!wasHandled)
		{
			result = DefWindowProc( hwnd, message, wParam, lParam );
		}
	}
	return result;
}

/*
* Creates a Direct2D bitmap from memory.
* Interpretes data in memory as 16 bit greyscale per pixel
*/
HRESULT Direct2dViewer::Load16bitGreyscaleBitmapFromMemory()
{
	HRESULT hr = S_OK;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmap            *ppIBitmap = NULL;
	IWICBitmapLock *pILock = NULL;
	WICRect rcLock = { 0, 0, static_cast<INT>(_bitmapSource.width), static_cast<INT>(_bitmapSource.height) };

	if (SUCCEEDED( hr ))
	{
		// Create the initial frame as WIC bitmap.
		hr = m_pWICFactory->CreateBitmapFromMemory(
			_bitmapSource.width,
			_bitmapSource.height,
			GUID_WICPixelFormat16bppGray,
			_bitmapSource.width * 2, //two bytes per pixel
			_bitmapSource.height * _bitmapSource.width * 2, //two bytes per pixel
			reinterpret_cast<BYTE*>(_bitmapSource.addr),
			&ppIBitmap
		);
	}

	if (SUCCEEDED( hr ))
	{
		// Obtain a bitmap lock for exclusive write.
		// The lock is for a  _bitmapSource.width x _bitmapSource.height rectangle starting at the top left of the
		// bitmap.
		hr = ppIBitmap->Lock( &rcLock, WICBitmapLockWrite, &pILock );

		//Process the pixel data that is now locked by the IWICBitmapLock object.

		if (SUCCEEDED( hr ))
		{
			UINT cbBufferSize = 0;
			BYTE *pv = NULL;

			// Retrieve a pointer to the pixel data.
			if (SUCCEEDED( hr ))
			{
				hr = pILock->GetDataPointer( &cbBufferSize, &pv );
			}
			// manipulate data in every pixel of bitmap
			for (UINT i = 0; i < _bitmapSource.width * _bitmapSource.height; i++)
			{
				// pointer to one pixel: base pointer + iterator which is multiplied by two, for two bytes per pixel
				UINT16 *p_pixel = (UINT16*)(pv + (i * sizeof( UINT16 )));
				// manipulate pixel data here
				if (*p_pixel * _gamma.amplitude - _gamma.offset > 0xFFFF)
					*p_pixel = 0xFFFF;
				else if (*p_pixel * _gamma.amplitude - _gamma.offset < 0)
					*p_pixel = 0;
				else
					*p_pixel = static_cast<UINT16>(*p_pixel * _gamma.amplitude - _gamma.offset);
			}

			// Release the bitmap lock.
			SafeRelease( &pILock );
		}
	}

	if (SUCCEEDED( hr ))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = m_pWICFactory->CreateFormatConverter( &pConverter );
	}
	if (SUCCEEDED( hr ))
	{
		hr = pConverter->Initialize(
			ppIBitmap,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED( hr ))
	{
		SafeRelease( &m_pBitmap );
		//create a Direct2D bitmap from the WIC bitmap.
		hr = m_pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&m_pBitmap
		);
	}

	SafeRelease( &pConverter );
	SafeRelease( &ppIBitmap );

	return hr;
}

/*
* set information about which data is used for displaying a bitmap
*/
void Direct2dViewer::setBitmapSource( void *addr, UINT width, UINT height )
{
	_bitmapSource.addr = addr;
	_bitmapSource.width = width;
	_bitmapSource.height = height;
	return;
}

/*
* returns the window handler of 2d viewer
*/
HWND Direct2dViewer::getWindowHandler()
{
	return m_hwnd;
}

/*
* creates the graphic rescource bitmap from memory,
* scales render target and applies gamma effects depending on bit setting to bitmap
* use setBitmapSource before when you want to show a new bitmap
*/
HRESULT Direct2dViewer::loadBitmap()
{
	HRESULT hr = S_OK;
	//create new bitmap
	hr = Load16bitGreyscaleBitmapFromMemory();
	return hr;
}

/*
* set gamma value
* param1 white: set value for maximum brightness
* param2 black: set value for minimum brightness
* default values are: black = 0, white = 0xFFFF (16 bit), amplitude = 0x3FFF (14 bit)
*/
void Direct2dViewer::SetGammaValue( UINT16 white, UINT16 black )
{
	if (black >= white) black = white - 1;
	_gamma.amplitude = (FLOAT)0xFFFF / (white - black); //default = 1
	_gamma.offset = static_cast<INT32>(_gamma.amplitude * white - 0xFFFF); //default = 0
	_gamma.white = white;
	_gamma.black = black;
	return;
}

/*
* return gamma value white
*/
UINT16 Direct2dViewer::GetGammaWhite()
{
	return _gamma.white;
}

/*
* return gamma value black
*/
UINT16 Direct2dViewer::GetGammaBlack()
{
	return _gamma.black;
}

/*
* starts 2d viewer with a initial bitmap
* use this to start 2d viewer. Call constructor before.
*/
HRESULT Direct2dViewer::start2dViewer( HWND hWndParent, void *bitmapAddr, UINT width, UINT height )
{
	//tell 2D viewer which data to use
	setBitmapSource( bitmapAddr, width, height );
	//start 2D viewer
	HRESULT hr = Initialize( hWndParent );
	return hr;
}

/*
* show a new bitmap in 2d viewer
*/
HRESULT Direct2dViewer::showNewBitmap( void *addr, UINT width, UINT height )
{
	//tell 2D viewer which data to use
	setBitmapSource( addr, width, height );
	//update 2D viewer bitmap
	HRESULT hr = reloadBitmap();
	return hr;
}

/*
* reload the displayed bitmap in 2d viewer
*/
HRESULT Direct2dViewer::reloadBitmap()
{
	//load same bitmap again
	HRESULT hr = loadBitmap();
	//send message to 2d viewer window to repaint
	SendMessage( getWindowHandler(), WM_PAINT, NULL, NULL );
	return hr;
}

void Direct2dViewer::CalcCursorPos()
{
	D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();
	// horizontal
	DOUBLE widthScaleFactor = (_cursorPos.x - _margin.left) / (renderTargetSize.width - _margin.right - _margin.left);
	if (widthScaleFactor < 0 || widthScaleFactor > 1) _cursorPos.pixel = -1;
	else _cursorPos.pixel = static_cast<INT>(widthScaleFactor * _bitmapSource.width);
	if (_cursorPos.pixel == _bitmapSource.width) _cursorPos.pixel = _bitmapSource.width - 1;
	// vertical
	DOUBLE heightScaleFactor = (_cursorPos.y - _margin.top) / (renderTargetSize.height - _margin.bottom - _margin.top);
	if (heightScaleFactor < 0 || heightScaleFactor > 1)_cursorPos.line = -2; //-2 because it is displayed with +1
	else _cursorPos.line = static_cast<INT>(heightScaleFactor * _bitmapSource.height);
	if (_cursorPos.line == _bitmapSource.height) _cursorPos.line = _bitmapSource.height - 1;
	return;
}
