#pragma once

#include <windows.h> 
#include <stdlib.h> 
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <process.h>	  // for Thread example
#include <CommCtrl.h>
#include "resource.h"
#include "GLOBAL.h" 
#include "CCDUNIT.C"
#include "Direct2dViewer_c.h"
#include <stdint.h> // included for uint16_t

#ifdef _DLL
UINT8 NUMBER_OF_BOARDS = 0;

ULONG aFLAG816[5] = { 1, 1, 1, 1, 1 };  //AD-Flag
ULONG aPIXEL[5] = { 0, 0, 0, 0, 0 };	// pixel
ULONG aXCKDelay[5] = { 1000, 1000, 1000, 1000, 1000 };	// sensor specific delay
BOOL aINIT[5] = { FALSE, FALSE, FALSE, FALSE, FALSE };
#else
#include "BOARD.C"
#endif

#if defined (WIN32)
#define IS_WIN32 TRUE
#else
#define IS_WIN32 FALSE
#endif

#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95   (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

#define TESTBITMAP_WIDTH 1000
#define TESTBITMAP_HEIGTH 200

// global variables
HINSTANCE hInst;   // current instance

LPCTSTR lpszAppName = "CCDEXAMP";
LPCTSTR lpszTitle = "CCDEXAMP";

HWND     hwndTrack;
HWND     hwndTrack2;

DWORD cur_nospb = 0;
DWORD cur_nob = 0;

void* Direct2dViewer;
uint16_t *testbitmap;

// function declerations
BOOL RegisterWin95(CONST WNDCLASS* lpwc);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
BOOL InitApplication(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
BOOL RegisterWin95(CONST WNDCLASS* lpwc);
void AboutTiming(HWND hWnd);
void AboutKeys(HWND hWnd);
void AboutCFS(HWND hWnd);
void AboutDMA(HWND hWnd);
void AboutPCI(HWND hWnd);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SetupMeasure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AllocateBuf(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ChooseBoard(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SetupTLevel(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SetupEC(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void createTestBitmap(UINT blocks, UINT height, UINT width);