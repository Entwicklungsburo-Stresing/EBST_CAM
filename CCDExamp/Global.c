#include "Global.h"

int ExpTime = 1000; //in �s
#if CAMCNT == 1
BOOL DISP2 = FALSE;		//display 1 camera
#endif
#if CAMCNT == 2
BOOL DISP2 = TRUE;		//display 2 cameras parallel, TRUE for double line 
#endif
BOOL contimess_run_once = FALSE;
BOOL _IsArea = FALSE; //FALSE is just the init val
__int16 _IsROI = 0; //FALSE is just the init val
#if TRIGGER_MODE == 1
BOOL EXTTRIGFLAG = TRUE;		// run with external Trigger
#else
BOOL EXTTRIGFLAG = FALSE;		// run with external Trigger
#endif
int TrigMod = 0;						//pos slope
__int64 TICKSDISP = 0;			//display time in ticks
int XOFF = 1;// _PIXEL / 600;			// index offset for display	
int XStart = 0;						//start index of display
BOOL PixelOdd = FALSE;				//display offset
int RepTime = 1 * _MINREPTIME; //in ms for _MSHUT
__int16 Releasems = 1; //>=1	>1 or keyboard does not work - could be exposuretime	
ULONG Threadp = 31;  //<=15  8=default,15=highest in current process,31=time critical
int TempLevel = 0;
BOOL GetNextScan = FALSE;
BOOL UpdateDispl = FALSE;
ULONG m_lfdTrmsNr = 0;
BOOL ShowTrms = FALSE;
ULONG	tDAT = 0; // delay after trigger
ULONG	tXDLY = 0; // exposure control for special sensors: PDA, ILC6, TH78xx
BYTE	tTICNT = 0; // trigger input divider
BYTE	tTOCNT = 0; // trigger output divider
int m_TOmodus = 1; //trigger out plug signal
int m_ECTrigmodus = 1;
int m_ECmodus = 1;
BOOL m_noPDARS = FALSE;