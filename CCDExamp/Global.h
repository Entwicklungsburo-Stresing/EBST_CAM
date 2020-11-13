#pragma once
#include <Windows.h>

// GLOBAL.h
// all globals for measure loop
// can be used for PCIE board
// for CCDExample and ESLSCDLL

#define IDM_EXIT           100

//camera system select
enum camera_system
{
	camera_system_3001 = 1,
	camera_system_3010 = 2,
	camera_system_3030 = 3
};
#define CAMERA_SYSTEM 1  // use 1 to 3 like in enum above
enum adc_mode
{
	normal = 0,
	ramp = 1,
	custom_pattern = 2
};
#define ADC_MODE normal
#define ADC_CUSTOM_PATTERN 0xFFFF
//options for 3010
#define LED_ON FALSE
#define GAIN_HIGH FALSE
//options for 3030
#define GAIN 6
#define _MSHUT FALSE
#define _MINREPTIME 20
#define	DRV	1	//1 if only one interface board LSCPCI1 or LSCISA1 in example
						// could be 2..4 for multiple boards
#define CAMCNT 1
#define _PIXEL 1088
//settings for 8 Bit cameras
//static	unsigned long FLAG816 = 2;  // 2=8Bit, 1=12/16Bit
//static int YSHIFT = 0;				// 8Bit=0, 12Bit=4 or more for ddrep>1
//static int YSHIFT = 4;				// 12Bit=4 or more for addrep>1
static int YSHIFT = 8;			// 16bit=8
//static int YSHIFT = 6;			// 14 bit
// camera values for calling GETCCD and InitBoard
#define _FFTLINES 64 		// no of vertical lines of FFT sensors, usually 64
							// =0 if not FFT
#define _ISPDA FALSE			//set RS after read; TRUE for HA S39xx
#define _ISFFT TRUE		//set vclk generator; TRUE for HA S703x
#define Vfreqini 7		//vclk freq for FFTs with FIFO in divider of12MHz (0..15)
						//=3 for highest speed with 7030-0906
enum trigger_mode
{
	xck = 0,
	exttrig = 1,
	dat = 2
};
#define TRIGGER_MODE xck
// Display data
static BOOL PLOTFLAG = TRUE;		// TRUE for dense, FALSE for dots
static int	LOX = 21;				// left upper x-corner of plot
static int	LOY = 41;				// left upper x-corner of plot
static unsigned int XLENGTH = _PIXEL + 50;			// x-width of  plot 
static unsigned int YLENGTH = 255;			// y-width
//for trms calcs
#define TRMSpix  1 //(8-1)*150-100// _PIXEL/2	//pixel no for which the rms value is sampled
// global declarations for CCDEXAMP
HDC hMSDC;	// global stored measure DC of our window
HWND hMSWND; // global stored measure HWND of our window
ULONG DisplData[2][1200 * CAMCNT];//array for display for 2 cams parallel

extern int ExpTime; //in µs
extern int RepTime; //in ms
extern int sec, bec, sdat, bdat;
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
extern ULONG tXDLY; // exposure control for special sensors: PDA, ILC6, TH78xx
extern BYTE	tTICNT; // trigger input divider
extern BYTE	tTOCNT; // trigger output divider
extern int m_TOmodus; //trigger out plug signal
extern int m_ECTrigmodus;
extern int m_ECmodus;
extern BOOL m_noPDARS;