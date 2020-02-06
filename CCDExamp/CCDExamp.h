#pragma once

#include <windows.h> 
#include <stdlib.h> 
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <process.h>	  // for Thread example
#include <CommCtrl.h>
#include "resource.h"
#include "Global.h" 
#include "CCDUnit.c"
#include "shared_src/Direct2dViewer_c.h"

#ifdef _DLL
UINT8 NUMBER_OF_BOARDS = 0;

ULONG aFLAG816[5] = { 1, 1, 1, 1, 1 };  //AD-Flag
ULONG aPIXEL[5] = { 0, 0, 0, 0, 0 };	// pixel
ULONG aXCKDelay[5] = { 1000, 1000, 1000, 1000, 1000 };	// sensor specific delay
BOOL aINIT[5] = { FALSE, FALSE, FALSE, FALSE, FALSE };
#else
#include "shared_src/Board.c"
#endif

#if defined (WIN32)
#define IS_WIN32 TRUE
#else
#define IS_WIN32 FALSE
#endif

#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95   (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32
#define IDT_TIMER1 1

// global variables
HINSTANCE hInst;   // current instance
LPCTSTR lpszAppName = "CCDExamp";
LPCTSTR lpszTitle = "CCDExamp";
HWND     hwndTrack;
HWND     hwndTrack2;
DWORD cur_nospb = 0;
DWORD cur_nob = 0;
void* Direct2dViewer;
#if CAMERA_SYSTEM == camera_system_3030
UINT16 direct2dviewer_gamma_white = 0x3FFF;
#else
UINT16 direct2dviewer_gamma_white = 0xFFFF;
#endif
UINT16 direct2dviewer_gamma_black = 0;
UINT roi[6] = { 15, 42, 15, 42, 10, 6 };
BOOL keep[5] = { FALSE, TRUE, FALSE, TRUE, FALSE };
BOOL ROI_CALLING = FALSE;

// function declerations
BOOL RegisterWin95( CONST WNDCLASS* lpwc );
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow );
BOOL InitApplication( HINSTANCE hInstance );
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow );
BOOL RegisterWin95( CONST WNDCLASS* lpwc );
void AboutTiming( HWND hWnd );
void AboutKeys( HWND hWnd );
void AboutCFS( HWND hWnd );
void AboutDMA( HWND hWnd );
void AboutPCI( HWND hWnd );
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetupMeasure( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK AllocateBuf( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ChooseBoard( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetupTLevel( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetupEC( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Set3ROI( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Set5ROI( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ResetROI( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetGamma( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );