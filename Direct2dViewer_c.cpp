#include "Direct2dViewer_c.h"
#include "Direct2dViewer.h"

void* Direct2dViewer_new() {
	return new Direct2dViewer();
}

void Direct2dViewer_delete(void *D2dV)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	delete D2d;
	return;
}

void Direct2dViewer_Initialize(void* D2dV, HWND hWndParent)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->Initialize(hWndParent);
	return;
}

void Direct2dViewer_show(void *D2dV)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->ShowViewer();
	return;
}

void Direct2dViewer_setBitmapSource(void *D2dV, void *addr, UINT width, UINT height)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->setBitmapSource(addr, width, height);
	return;
}

HWND Direct2dViewer_getWindowHandler(void *D2dV)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	return D2d->getWindowHandler();
}

void Direct2dViewer_updateBitmap(void* D2dV)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->updateBitmap();
	return;
}