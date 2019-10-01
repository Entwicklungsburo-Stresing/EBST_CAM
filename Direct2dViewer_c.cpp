#include "Direct2dViewer_c.h"
#include "Direct2dViewer.h"

void* Direct2dViewer_new() {
	return new Direct2dViewer();
}

void Direct2dViewer_delete(void *D2dV) {
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	delete D2d;
	return;
}

void Direct2dViewer_Initialize(void* D2dV) {
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->Initialize();
	return;
}

void Direct2dViewer_show(void *D2dV) {
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->ShowViewer();
	return;
}