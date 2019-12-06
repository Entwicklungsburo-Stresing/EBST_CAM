// Microsoft C/C++ specific import/export specifier.
// These take the place of the EXPORTS and IMPORTS
// statements in the application and DLL .DEF files.
//..................................................
//
#ifdef _DLL
#define DllAccess __declspec( dllexport )

#else
#define DllAccess __declspec( dllimport )

#endif

#define DBGNOCAM	FALSE	//TRUE if debug with no camera - geht nicht ohne gegenseite: kein clk!


// set data array

typedef USHORT ArrayT; //!! USHORT for linear 12/16bit word array or resort
#define MAXVAL USHRT_MAX


//for Win98
typedef BYTE UINT8;
typedef short INT16;
struct ffloopparams {
	UINT32 board_sel;
	UINT32 exptus;
	UINT8 exttrig;
	UINT8 blocktrigger;
	UINT8 btrig_ch;
};


typedef ArrayT* pArrayT;


#include "shared_src/board.h"

// key to stop measure loop
#define _ScanCode_End	 57 //E=18   Space 57
#define _ScanCode_Cancel  01 //Q=16   ESC 01

//TRUE if you use PS2 Keyboard, FALSE else
// on WinNT/2000/xp the function GetAsyncKeystate works only
// if the Thread Class is not highest or 
// if the Sleep function is used inside Contimess->Measure
// so here a special hardware read of PS2Keyboard is used
// to avoid interrupt dependend routines; look BOARD->WaitTrigger
#define _PS2KEYBOARD FALSE 


#define	DRV		1	//1 if only one interface board LSCPCI1 or LSCISA1 in example
						// could be 2..4 for multiple boards

#define _HA_IR FALSE		//  must be true for Ha series G920x IR sensor - not for G11608
#define _IR2 FALSE			// must be true for 2 parallel IR sensors of series G920x
//#define _TI FALSE			// must be true for TI TC253 sensor
//#define _IS_C4350 FALSE		// must be TRUE for C4350 adapter
#define _PARALLEL FALSE		//TRUE for 2 Interface Boards
#define _RESORT FALSE		// resort array for PCI board with 2 cameras or IR
//#define _PPORT FALSE		//TRUE if used on printerport
//#define _OPTSTATE FALSE		//decrement pixel if shutterstate input with OPTO1 is used with old board 2105
#define _COOLER FALSE		//TRUE if PC managed cooler
#define _PRGRMVON FALSE		//TRUE for SENDCOMMAND with VON instead of ND

#define _HWCH2 FALSE			// TRUE if 2 cam data bytes are packet in one word (2cams parallel read with adaptor board)
							//TRUE for all double line systems

#define _FKT 1				// standard read=1, =5 testdata fill with 1024
#define Vfreqini 7			// only FIFO set in SetupVCLKReg
							//frequence of vclks for FFTs S703x
//#define _HILO FALSE			//default is FALSE

__int64 TPS = 0;				// ticks per second; is set in InitHRCounter
__int64 START = 0;				// global variable for sync to systemtimer
ULONG _PIXEL = 544;			// here as variable with defaults
ULONG ADRDELAY = 1000;			// -> is set in SetBoardVars


//Andanata specials
#define _TESTCAM FALSE		//test for data val(pix=320)<3000?
#define _TESTANDANTA FALSE	//display just 1 line
#define _TESTRUPED FALSE		//set toreg to outtrig 1 on line1 and 0 on line ROILINES-2
UINT32 ROILINES = 258;		//max. lines to read for ANdanta sensor
#define _DISPLAndantaLine 8 //display this line if Andanta sensor

// global Camera data

#define _FORCETOPLS128 TRUE	//only use payload size 128byte
ULONG NO_TLPS;//0x12; //was 0x11-> x-offset			//0x11=17*128  = 2176 Bytes  = 1088 WORDS
ULONG TLPSIZE;					//with0x21: crash

#define RAMPAGESIZE 4096
#define DMA_BUFSIZEINSCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans		//default: BUFSIZE=1000, INTR all 500
#define DMA_HW_BUFPARTS 2
#define DMA_DMASPERINTR DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb nochmal /2

#define _MAXDB	4					// no. of lines
#define Nos _MAXDB
#define DMA_64BIT_EN FALSE
#define KER_MODE FALSE
//for jungo projects
#define KERNEL_64BIT	
#define WINNT

//#define USERBUFINSCANS 100
//#define DMABufSizeInScans  100
//#define IntFreqInScans  (DMABufSizeInScans/4) //DMABufSizeInScans / 2;
//#define DMABufSize 600 *_PIXEL * 2
#define HWDREQ_EN TRUE		// enables hardware start of DMA by XCK h->l slope
#define INTR_EN TRUE		// enables INTR

#define MAXPCIECARDS 5

//globals
WORD UserBufInScans;

volatile USHORT*   pDMABigBufIndex[3] = { NULL, NULL, NULL };
USHORT*   pDMABigBufBase[3] = { NULL, NULL, NULL };
//UINT_PTR   pDMABigBufBase[3] = { NULL, NULL, NULL };

//jungo
pArrayT pBLOCKBUF[3] = { NULL, NULL, NULL };
//USHORT DMAUserBuf[1200][USERBUFINSCANS];//not for dll
//WORD UserBufInScans = USERBUFINSCANS;
//DMAUserBuf is the complete memory of the application
//PUSHORT pDMABigBuf; //not for dll
 //PUSHORT pDMABigBufBase[3]; //not for dll



ULONG DisplData[2][1200];//array for display for 2 cams parallel

//delete after deleting ringreadthread									// array type is defined in board.h
ArrayT DIODEN[_MAXDB][1200];		// global data array, could be 1 or 2 dim
ArrayT DIODENRingBuf[(100 + 10) * 1200 * sizeof( USHORT )];
DWORD FirstPageOffset;
pArrayT pDIODEN = (pArrayT)&DIODEN;


int Nob = 10;
int Nospb = 100;

int newDLL = 0;
void	*Direct2dViewer = NULL;