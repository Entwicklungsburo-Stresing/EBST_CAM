#pragma once
#include <Windows.h>
#include "shared_src/enum.h"

// GLOBAL.h
// all globals for measure loop
// can be used for PCIE board
// only valid for CCDExample

#define IDM_EXIT           100
#define CAMERA_SYSTEM camera_system_3030  // use 1 to 3 like in enum above
#define	DRV	1	//1 if only one interface board 
// could be 2..4 for multiple boards
#define CAMCNT 1		//number of sensors in the queue 
#define _PIXEL 1088		//discrete numbers = act pixel+64 -> 4160 - 2112 - 1088 - 576 - 320 - 192
#define ADC_MODE normal
#define ADC_CUSTOM_PATTERN 0xFFFF
//options for 3010
#define LED_ON FALSE
#define GAIN_HIGH FALSE
//options for 3030
#define GAIN 6
#define _MSHUT FALSE
#define _MINREPTIME 20
//settings for Y-scale of graphic display
//static int YSHIFT = 4;				// 12Bit=4 or more for addrep>1
//static int YSHIFT = 8;			// 16bit=8
static int YSHIFT = 6;			// 14 bit
// camera values for calling GETCCD and InitBoard
#define SENSOR_TYPE PDAsensor
//for FFT sensors
#define _FFTLINES 64 		// no of vertical lines of FFT sensors, usually 64
// =0 if not FFT
#define Vfreqini 7		//vclk freq for FFTs with FIFO in divider of12MHz (0..15)
						//=4 for highest speed with 7030-0906
#define AREA_LINES_BINNING 1
#define CCTRIGGER_MODE xck
#define XCKDELAY 0
// Display data
static BOOL PLOTFLAG = TRUE;		// TRUE for dense, FALSE for dots
static int	LOX = 21;				// left upper x-corner of plot
static int	LOY = 41;				// left upper x-corner of plot
static unsigned int XLENGTH = _PIXEL + 50;			// x-width of  plot 
static unsigned int YLENGTH = 255;			// y-width
//for trms calcs
#define TRMSpix  550 //(8-1)*150-100// _PIXEL/2	//pixel no for which the rms value is sampled
// global declarations for CCDEXAMP
HDC hMSDC;	// global stored measure DC of our window
HWND hMSWND; // global stored measure HWND of our window
UINT16 DisplData[2][1200 * CAMCNT];//array for display for 2 cams parallel
#if CAMERA_SYSTEM == 2
#define DIRECT2DVIEWER_GAMMA_WHITE_DEFAULT 0x3FFF
#else
#define DIRECT2DVIEWER_GAMMA_WHITE_DEFAULT 0xFFFF
#endif

extern int ExpTime; //in µs
extern int RepTime; //in ms
extern int Sec, Bec, Sdat, Bdat;
extern BOOL DISP2;
extern BOOL contimess_run_once;
extern BOOL _IsArea; //FALSE is just the init val
extern __int16 _IsROI; //FALSE is just the init val
extern int TrigMod;						//pos slope
extern int TrigMod_B;						//pos slope
extern int ItemIndex_S;						//combo box
extern int ItemIndex_B;						//combo box
extern __int64 TICKSDISP;			//display time in ticks
extern int XOFF;// _PIXEL / 600;			// index offset for display	
extern int XStart;						//start index of display
extern BOOL PixelOdd;				//display offset
extern int RepTime; //in ms for _MSHUT
extern __int16 Releasems; //>=1	>1 or keyboard does not work - could be exposuretime	
extern ULONG Threadp;  //<=15  8=default,15=highest in current process,31=time critical
extern int TempLevel;
extern BOOL GetNextScan;
extern BOOL UpdateDispl;
extern ULONG m_lfdTrmsNr;
extern BOOL ShowTrms;
extern ULONG tDAT; // delay after trigger
extern ULONG tXDLY; //  delay for integrators INT before start reading
extern BYTE	tTICNT; // trigger input divider
extern BYTE	tTOCNT; // trigger output divider
extern int m_TOmodus; //trigger out plug signal
extern int m_ECTrigmodus;
extern int m_ECmodus;
extern BOOL m_noPDARS;
extern double TRMSval_global[2];
extern UINT choosen_board;
extern BOOL both_boards;
extern BOOL cont_mode;
extern int fftMode;
extern UINT8 roi[8];
extern UINT16 dac[8];
