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
			hr = Load16bitGreyscaleBitmapFromMemory(
				m_pRenderTarget,
				m_pWICFactory,
				&m_pBitmap
			);
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
}

//
// The main window message loop.
//
void Direct2dViewer::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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
		// Retrieve the size of the render target.
		D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();

		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		//D2D1_SIZE_F size = m_pBitmap->GetSize();

		// Draw bitmap
		m_pRenderTarget->DrawBitmap(
			m_pBitmap,
			D2D1::RectF(
				0,
				0,
				renderTargetSize.width,
				renderTargetSize.height)
		);

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
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				D2DV->OnResize(width, height);
			}
				result = 0;
				wasHandled = true;
				break;

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

//
// Creates a Direct2D bitmap from memory.
// Interpretes data in memory as 16 bit greyscale per pixel
//
HRESULT Direct2dViewer::Load16bitGreyscaleBitmapFromMemory(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	ID2D1Bitmap **ppBitmap
)
{
	HRESULT hr = S_OK;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmap            *ppIBitmap = NULL;


	if (SUCCEEDED(hr))
	{
		// Create the initial frame as WIC bitmap.
		hr = pIWICFactory->CreateBitmapFromMemory(
			_bitmapSource.width,
			_bitmapSource.height,
			GUID_WICPixelFormat16bppGray,
			_bitmapSource.width * 2, //two bytes per pixel
			_bitmapSource.height * _bitmapSource.width * 2, //two bytes per pixel
			reinterpret_cast<BYTE*>(_bitmapSource.addr),
			&ppIBitmap
		);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
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
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SafeRelease(&pConverter);
	SafeRelease(&ppIBitmap);

	return hr;
}

void Direct2dViewer::setBitmapSource(void *addr, UINT width, UINT height)
{
	_bitmapSource.addr = addr;
	_bitmapSource.width = width;
	_bitmapSource.height = height;
	return;
}

HWND Direct2dViewer::getWindowHandler()
{
	return m_hwnd;
}

HRESULT Direct2dViewer::updateBitmap() {
	HRESULT hr = S_OK;

	//release old bitmap
	SafeRelease(&m_pBitmap);

	//create new bitmap
	hr = Load16bitGreyscaleBitmapFromMemory(
		m_pRenderTarget,
		m_pWICFactory,
		&m_pBitmap
	);

	return hr;
}
