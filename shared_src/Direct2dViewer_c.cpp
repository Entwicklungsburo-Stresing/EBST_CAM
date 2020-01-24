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

#include "Direct2dViewer_c.h"
#include "Direct2dViewer.h"

void* Direct2dViewer_new() {
	return new Direct2dViewer();
}

void Direct2dViewer_delete( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	delete D2d;
	return;
}

void Direct2dViewer_start2dViewer( void *D2dV, HWND hWndParent, void *bitmapAddr, UINT width, UINT height )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->start2dViewer( hWndParent, bitmapAddr, width, height );
	return;
}

void Direct2dViewer_showNewBitmap( void *D2dV, void *addr, UINT width, UINT height )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->showNewBitmap( addr, width, height );
	return;
}

HWND Direct2dViewer_getWindowHandler( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	return D2d->getWindowHandler();
}

void Direct2dViewer_setGammaValue( void *D2dV, UINT16 white, UINT16 black )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->SetGammaValue( white, black );
	return;
}
