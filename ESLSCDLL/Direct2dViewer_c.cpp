/*
This file is part of CCDExamp and ESLSCDLL.

CCDExamp and ESLSCDLL are free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CCDExamp and ESLSCDLL are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see < http://www.gnu.org/licenses/>.

Copyright 2020 Entwicklungsbuero Stresing (http://www.stresing.de/)
*/

#ifndef MINIMAL_BUILD

#include "Direct2dViewer_c.h"
#include "Direct2dViewer.h"

/**
 * @copydoc Direct2dViewer::Direct2dViewer
 * @return Handle of new Direct2dViewer instance.
 */
void* Direct2dViewer_new()
{
	return new Direct2dViewer();
}

/**
 * @copydoc Direct2dViewer::~Direct2dViewer
 * @param D2dV Handle of Direct2dViewer instance.
 */
void Direct2dViewer_delete( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	delete D2d;
	return;
}

/**
 * @copydoc Direct2dViewer::start2dViewer
 * @param D2dV Handle of Direct2dViewer instance.
 */
void Direct2dViewer_start2dViewer( void *D2dV, HWND hWndParent, void *bitmapAddr, UINT width, UINT height )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->start2dViewer( hWndParent, bitmapAddr, width, height );
	return;
}

/**
 * @copydoc Direct2dViewer::showNewBitmap
 * @param D2dV Handle of Direct2dViewer instance.
 */
void Direct2dViewer_showNewBitmap( void *D2dV, void *addr, UINT width, UINT height )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->showNewBitmap( addr, width, height );
	return;
}

/**
 * @copydoc Direct2dViewer::repaintWindow
 * @param D2dV Handle of Direct2dViewer instance.
 */
void Direct2dViewer_repaintWindow( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->repaintWindow();
	return;
}

/**
 * @copydoc Direct2dViewer::getWindowHandler
 * @param D2dV Handle of Direct2dViewer instance.
 */
HWND Direct2dViewer_getWindowHandler( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	return D2d->getWindowHandler();
}

/**
 * @copydoc Direct2dViewer::SetGammaValue
 * @param D2dV Handle of Direct2dViewer instance.
 */
void Direct2dViewer_setGammaValue( void *D2dV, UINT16 white, UINT16 black )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	D2d->SetGammaValue( white, black );
	return;
}

/**
 * @copydoc Direct2dViewer::GetGammaWhite
 * @param D2dV Handle of Direct2dViewer instance.
 */
UINT16 Direct2dViewer_getGammaWhite( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	return D2d->GetGammaWhite();
}

/**
 * @copydoc Direct2dViewer::GetGammaBlack
 * @param D2dV Handle of Direct2dViewer instance.
 */
UINT16 Direct2dViewer_getGammaBlack( void *D2dV )
{
	Direct2dViewer *D2d = (Direct2dViewer *)D2dV;
	return D2d->GetGammaBlack();
}

#endif