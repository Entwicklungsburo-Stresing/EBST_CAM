// This is a Direct2D Viewer for bitmaps used in CCD example. It was created by using Microsofts example "SimpleDirect2DApplication", originally released under MIT license.
// https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/multimedia/Direct2D/SimpleDirect2DApplication

#include "Direct2dViewer.h"

//
// Initialize members.
//
Direct2dViewer::Direct2dViewer() :
	m_hwnd(NULL),
	m_pD2DFactory(NULL),
	m_pWICFactory(NULL),
	m_pRenderTarget(NULL),
	m_pDeviceContext(NULL),
	m_pBitmap(NULL)
{
}

//
// Release resources.
//
Direct2dViewer::~Direct2dViewer()
{
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pWICFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pDeviceContext );
	SafeRelease(&m_pBitmap);
}

//
// Creates the application window and initializes
// device-independent resources.
//
HRESULT Direct2dViewer::Initialize(HWND hWndParent)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED(hr)) {
		// Initialize device-indpendent resources, such
		// as the Direct2D factory.
		hr = CreateDeviceIndependentResources();
	}

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Direct2dViewer::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = L"Direct2dViewer";

		RegisterClassEx(&wcex);

		// Create the application window.
		//
		// Because the CreateWindow function takes its size in pixels, we
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

		// Create the application window.
		m_hwnd = CreateWindow(
			L"Direct2dViewer",
			L"2D Viewer",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
			hWndParent,
			NULL,
			HINST_THISCOMPONENT,
			this
		);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
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
	HRESULT hr = S_OK;

	if (SUCCEEDED(hr)) {
		// Create a Direct2D factory.
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	}
	if (SUCCEEDED(hr))
	{
		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICFactory)
		);
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
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);
		if (SUCCEEDED(hr))
		{
			// Create a Direct2D render target.
			hr = m_pD2DFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(m_hwnd, size),
				&m_pRenderTarget
			);
		}

		if (SUCCEEDED(hr))
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
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pBitmap);
	SafeRelease(&m_pDeviceContext);
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

	if (SUCCEEDED(hr)) {
		hr = CreateDeviceResources();
	}

	if (SUCCEEDED(hr) && !(m_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{
		m_pRenderTarget->BeginDraw();
		m_pDeviceContext->DrawImage(linearTransferEffect, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
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
// The window message handler.
//
LRESULT CALLBACK Direct2dViewer::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		Direct2dViewer *D2DV = (Direct2dViewer *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(D2DV)
		);
	}
	else
	{
		Direct2dViewer *D2DV = reinterpret_cast<Direct2dViewer *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (D2DV)
		{
			switch (message)
			{
			case WM_PAINT:
			case WM_DISPLAYCHANGE:
			{
				PAINTSTRUCT ps;
				BeginPaint(hwnd, &ps);
				D2DV->OnRender();
				EndPaint(hwnd, &ps);
			}
				result = 0;
				wasHandled = true;
				break;

			case WM_DESTROY:
				//send custom message to main window about closing 2d viewer
				HWND hWndOwner = GetWindow(hwnd, GW_OWNER);
				SendMessage(hWndOwner, WM_2DVIEWER_CLOSED, NULL, NULL);
				result = 1;
				wasHandled = true;
				break;
			}
		}
		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
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
	WICRect rcLock = { 0, 0, _bitmapSource.width, _bitmapSource.height };

	if (SUCCEEDED(hr))
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

	//if (SUCCEEDED( hr ))
	//{
	//	// Obtain a bitmap lock for exclusive write.
	//	// The lock is for a  _bitmapSource.width x _bitmapSource.height rectangle starting at the top left of the
	//	// bitmap.
	//	hr = ppIBitmap->Lock( &rcLock, WICBitmapLockWrite, &pILock );

	//	//Process the pixel data that is now locked by the IWICBitmapLock object.
	//		
	//		if (SUCCEEDED( hr ))
	//		{
	//			UINT cbBufferSize = 0;
	//			BYTE *pv = NULL;

	//			// Retrieve a pointer to the pixel data.
	//			if (SUCCEEDED( hr ))
	//			{
	//				hr = pILock->GetDataPointer( &cbBufferSize, &pv );
	//			}

	//			for (UINT i=0; i < _bitmapSource.width; i++)
	//			{
	//				BYTE *p_pixel = pv + i*2;
	//				UINT16 new_value = (UINT16) (&p_pixel) * 10;
	//				memset( p_pixel, new_value, 1 );
	//			}

	//			// Release the bitmap lock.
	//			SafeRelease( &pILock );
	//		}
	//}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = m_pWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
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
	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = m_pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&m_pBitmap
		);
	}

	SafeRelease(&pConverter);
	SafeRelease(&ppIBitmap);

	return hr;
}

/*
* set information about which data is used for displaying a bitmap
*/
void Direct2dViewer::setBitmapSource(void *addr, UINT width, UINT height)
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

void Direct2dViewer::ScaleRenderTarget()
{
	// Retrieve the size of the render target.
	D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();
	// calculate scale factors
	float scale_x = renderTargetSize.width / _bitmapSource.width;
	float scale_y = renderTargetSize.height / _bitmapSource.height;
	// scale render target
	m_pRenderTarget->SetTransform( D2D1::Matrix3x2F::Scale(
		D2D1::Size( scale_x, scale_y ),
		D2D1::Point2F( 0.0f, 0.0f ) ) );
	return;
}

void Direct2dViewer::CreateGammaEffect()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED( hr ))
	{
		// Obtain hwndRenderTarget's deviceContext
		hr = m_pRenderTarget->QueryInterface( __uuidof(ID2D1DeviceContext), (void**)&m_pDeviceContext );
	}

	if (SUCCEEDED( hr ))
	{
		m_pDeviceContext->CreateEffect( CLSID_D2D1LinearTransfer, &linearTransferEffect );
		linearTransferEffect->SetInput( 0, m_pBitmap );
		linearTransferEffect->SetValue( D2D1_LINEARTRANSFER_PROP_RED_SLOPE, _gamma_amplitude );
		linearTransferEffect->SetValue( D2D1_LINEARTRANSFER_PROP_GREEN_SLOPE, _gamma_amplitude );
		linearTransferEffect->SetValue( D2D1_LINEARTRANSFER_PROP_BLUE_SLOPE, _gamma_amplitude );
		linearTransferEffect->SetValue( D2D1_LINEARTRANSFER_PROP_RED_Y_INTERCEPT, _gamma_offset );
		linearTransferEffect->SetValue( D2D1_LINEARTRANSFER_PROP_GREEN_Y_INTERCEPT, _gamma_offset );
		linearTransferEffect->SetValue( D2D1_LINEARTRANSFER_PROP_BLUE_Y_INTERCEPT, _gamma_offset );
	}
	return;
}

/*
* creates the graphic rescource bitmap from memory,
* scales render target and applies gamma effects depending on bit setting to bitmap
* use setBitmapSource before when you want to show a new bitmap
*/
HRESULT Direct2dViewer::loadBitmap()
{
	HRESULT hr = S_OK;

	//release old bitmap
	if(m_pBitmap != NULL) SafeRelease(&m_pBitmap);

	//create new bitmap
	hr = Load16bitGreyscaleBitmapFromMemory();

	//scale render target to window size
	ScaleRenderTarget();

	//apply gamma effects
	CreateGammaEffect();

	return hr;
}

/*
* set gamma value
* param1 white: set value for maximum brightness
* param2 black: set value for minimum brightness
* default values are: black = 0, white = 0xFFFF (16 bit), amplitude = 0x3FFF (14 bit)
*/
void Direct2dViewer::SetGammaValue(UINT white, UINT black)
{
	_gamma_amplitude = (FLOAT) 0xFFFF / white; //default = 1
	//offset doesn't work as I expected, so I hardcoded it to 0
	_gamma_offset = (FLOAT) - (black / UINT(0xFFFF)); //default = 0

	//apply gamma effects
	if(m_pRenderTarget) CreateGammaEffect();

	return;
}

/*
* starts 2d viewer with a initial bitmap
* use this to start 2d viewer. Call constructor before.
*/
HRESULT Direct2dViewer::start2dViewer(HWND hWndParent, void *bitmapAddr, UINT width, UINT height)
{
	//tell 2D viewer which data to use
	setBitmapSource(bitmapAddr, width, height);
	//start 2D viewer
	HRESULT hr = Initialize(hWndParent);
	return hr;
}

/*
* updates the displayed bitmap in 2d viewer
*/
HRESULT Direct2dViewer::showNewBitmap(void *addr, UINT width, UINT height)
{
	//tell 2D viewer which data to use
	setBitmapSource(addr, width, height);
	//update 2D viewer bitmap graphic rescource
	HRESULT hr = loadBitmap();
	//send message to 2d viewer window to repaint
	SendMessage(getWindowHandler(), WM_PAINT, NULL, NULL);
	return hr;
}