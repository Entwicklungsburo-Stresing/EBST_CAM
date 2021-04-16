#pragma once

#include <windows.h> 
#include <stdlib.h> 
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <process.h>	  // for Thread example
#include <CommCtrl.h>
#include "resource.h"
#include "Direct2dViewer_c.h"

#ifdef _DLL
NUMBER_OF_BOARDS = 0;

ULONG aFLAG816[5] = { 1, 1, 1, 1, 1 };  //AD-Flag
ULONG aPIXEL[5] = { 0, 0, 0, 0, 0 };	// pixel
ULONG aXCKDelay[5] = { 1000, 1000, 1000, 1000, 1000 };	// sensor specific delay
BOOL aINIT[5] = { FALSE, FALSE, FALSE, FALSE, FALSE };
#else
#include "shared_src/Board.h"
#endif
#include "Global.h" 
#include "CCDUnit.h"

#if defined (WIN32)
#define IS_WIN32 TRUE
#else
#define IS_WIN32 FALSE
#endif
#define LEGACY_202_14_TLPCNT FALSE
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95   (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32
#define IDT_TIMER1 1

extern DWORD cur_nos;
extern DWORD cur_nob;

// function declerations
BOOL InitApplication( HINSTANCE hInstance );
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow );
void AboutTiming( HWND hWnd );
void AboutKeys( HWND hWnd );
void AboutCFS( HWND hWnd );
void AboutDMA(HWND hWnd);
void AboutDAC(HWND hWnd);
void AboutPCI( HWND hWnd );
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetupMeasure( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK AllocateBuf( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK ChooseBoard( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetupTLevel( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SetupEC( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Set3ROI( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Set5ROI(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SetDAC(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK FullBinning(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AreaMode(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SetGamma( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
void enableAllControls();
