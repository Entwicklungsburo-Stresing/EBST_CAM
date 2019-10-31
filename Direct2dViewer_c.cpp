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

void Direct2dViewer_start2dViewer(void *D2dV, HWND hWndParent, void *bitmapAddr, UINT width, UINT height)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->start2dViewer(hWndParent, bitmapAddr, width, height);
	return;
}

void Direct2dViewer_showNewBitmap(void *D2dV, void *addr, UINT width, UINT height)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->showNewBitmap(addr, width, height);
	return;
}

HWND Direct2dViewer_getWindowHandler(void *D2dV)
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	return D2d->getWindowHandler();
}

void Direct2dViewer_setGammaValue( void *D2dV, FLOAT amplitude, FLOAT offset )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->SetGammaValue( amplitude, offset);
	return;
}
