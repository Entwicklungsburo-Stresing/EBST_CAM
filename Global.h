// GLOBAL.h   V1.0
// all globals for measure loop
// can be used for PCIE board
// for CCDExample and ESLSCDLL

#define IDM_EXIT           100

#define DBGNOCAM	FALSE	//TRUE if debug with no camera - geht nicht ohne gegenseite: kein clk!

#define _ERRTEST FALSE //test data for integrity

#define _33MHz FALSE //true for highest speed cams

//set TRUE if you want to change the thread priority:
#define _USETHREAD	TRUE
//#define _USETHREAD !_FIFO  // with FIFO don't change priority
#define _THREADPRI 15

//TRUE if you use PS2 Keyboard, FALSE else
// on WinNT/2000/xp the function GetAsyncKeystate works only
// if the Thread Class is not highest or 
// if the Sleep function is used inside Contimess->Measure
// so here a special hardware read of PS2Keyboard is used
// to avoid interrupt dependend routines; look BOARD->WaitTrigger
#define _PS2KEYBOARD FALSE 


//#define _HA_IR FALSE		//  must be true for Ha series G920x IR sensor - not for G11608
//#define _HA_IRSingleCH FALSE// TRUE for Ha series G920x if IR Sensor has 256 pixel, FALSE if 512 - not G11608
//#define _IR2 FALSE			// must be true for 2 parallel IR sensors of type G920x - not G11608
//#define _TI FALSE			// must be true for TI TC253 sensor
//#define _IS_C4350 FALSE		// must be TRUE for C4350 adapter
#define _PARALLEL FALSE		//TRUE for 2 Interface Boards
#define _RESORT FALSE		// resort array for IR G920x - not G11608
//#define _PPORT FALSE		//TRUE if used on printerport
//#define _AD16cds FALSE		//TRUE for AD9826 cds 10Mhz
//#define _AD16ADC FALSE		//TRUE for ADC16061 2Mhz or LTC2204
#define _AD16ADS TRUE		//TRUE if act AD is used
#define _HILO FALSE			//FALSE for default
#define _USESHUTTER FALSE	//TRUE if sensor has shutter function
//#define _PRGRMVON FALSE		//TRUE for SENDCOMMAND with VON instead of ND
//#define _OPTSTATE FALSE //decrement pixel if shutterstate input with OPTO1 is used on old boards

//ADC16061 specials
//#define _FFPCI133 FALSE		// PCI version 133.x
//BOOL _OLDCAL = FALSE;		// TRUE for  ADC16061


//display 2 graphics
BOOL DISP2 = FALSE;		//display 2 cameras parallel, TRUE for double line 
#define _HWCH2	FALSE	//true if 2 words are packet in one long ->resort to db1&db2 = PCI with adaptor board
						//all double line systems
#define _RESORTDB	FALSE	//true if ->resort to db1&db2 - only no FIFO

#define		DRV		1	//1 if only one interface board LSCPCI1 or LSCISA1 in example
						// could be 2..4 for multiple boards

// global Camera data
#define  _NO_TLPS  0xa //oldpc: 0x11				//0x11=17*128  = 2176 Bytes  = 1088 WORDS
#define TLPSize 0x40	//0x20 on old pc
#define RAMPAGESIZE 4096
#define DMA_BUFSIZEINSCANS 1000		//default: BUFSIZE=1000, INTR all 500
#define DMA_HW_BUFPARTS 2
#define DMA_SCANSPERINTR DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb nochmal /2

#define _PIXEL  1088				// no of pixels min 300, should be multiple of 300, max 8100
#define _MAXDB	4					// no. of lines
#define Nos _MAXDB
#define DMA_64BIT_EN FALSE
#define KER_MODE FALSE
//for jungo projects
#define KERNEL_64BIT	
#define WINNT



	typedef USHORT ArrayT; //!! USHORT for linear 12/16bit word array or resort or highest speed
	#define MAXVAL USHRT_MAX


typedef ArrayT* pArrayT; 
//!! long for standard


/*
//delete after deleting ringreadthread									// array type is defined in board.h
ArrayT DIODEN[_MAXDB][_PIXEL];		// global data array, could be 1 or 2 dim
ArrayT DIODENRingBuf[100 * RAMPAGESIZE];
DWORD FirstPageOffset;
pArrayT pDIODEN = (pArrayT) &DIODEN;

//jungo

USHORT DMAUserBuf[_PIXEL][USERBUFINSCANS];
PUSHORT pDMAUserBuf = &DMAUserBuf[0][0];
//DWORD dwDMABufSize;// = 100 * RAMPAGESIZE * 2;// 100: ringbufsize 2:because  we need the size in bytes
ULONG DisplData[2][_PIXEL];//array for display for 2 cams parallel
*/

//this is the read from FIFO frequency
static BOOL SYM_PULSE = FALSE;		// read FIFO is allways higest speed
static BOOL BURSTMODE = TRUE;
static 	unsigned long WAITS = 0x0;	





//valid for FIFO mode only
//static	unsigned long PCLK = 5;	
//this is the write to FIFO frequency / FREQ=datarate=pclk*2 if 12/16bit
static ULONG FREQ = 0; //=waits -> 0=33MHz;1=16MHz;2=8MHz max 6=0 
static ULONG DELAYini = 0;			//DELAY for WRFIFO

static ULONG XCKDELAY = 3; //100ns+n*400ns, 1<n<8 Sony=7





//only for ISA
static	unsigned long FOURMAX = _MAXDB * _PIXEL;
#define PPORTADR 0 //0x378	//PrinterPortaddress or 0 if ISA Board


// settings for 8 Bit cameras
//static	unsigned long FLAG816 = 2;  // 2=8Bit, 1=12/16Bit
//static int YSHIFT = 0;				// 8Bit=0, 12Bit=4 or more for ddrep>1

// use these values for 12 or 16 Bit cameras 
static	unsigned long FLAG816 = 1;  // 2=8Bit, 1=12/16Bit
//static int YSHIFT = 4;				// 12Bit=4 or more for addrep>1
//static int YSHIFT = 8;			// 16bit=8
//static int YSHIFT = 6;			// 14 bit
static int YSHIFT = 6;			// 14 bit

// camera values for calling GETCCD and InitBoard

static	unsigned long _FKT = 1;		// -1:clearread, 0:datab=0, 1:read 5: testdata
									//, 2:add ; not implemented in DMA is 3:sub 
#define _FFTLINES 0 		// no of vertical lines of FFT sensors, usually 64
							// =0 if not FFT
#define _ISPDA FALSE			//set RS after read; TRUE for HA S39xx
#define _ISFFT FALSE		//set vclk generator; TRUE for HA S703x

//#define _HA_MODULE FALSE		//TRUE for HA module C7041 or C8061


//vclk frequency 

#define Vfreqini 7		//vclk freq for FFTs with FIFO in divider of12MHz (0..15)
						//=3 for highest speed with 7030-0906





//#define _ARRAYLINES 0		//=0 only for Andanta IR area sensor =256

//static	unsigned long ZADR = 1;		// Adress for adressed mode
									//for IR ZADR=0 -> both channels

											

// parameter for Loop
static int ADDREP = 1;			 	//addition loop for fkt=2; 1 else
BOOL EXTTRIGFLAG = FALSE;		// run with external Trigger
									// DELAYMS is here wait after trigger!
int TrigMod=0;						//pos slope
//static BOOL HISLOPE = TRUE;			// Slope for external Trigger

static BOOL HIAMP = FALSE;			// Amplification for switchable sensors



__int64 TPS = 0;				// ticks per second; is set in InitHRCounter
//__int64 DELAY = 0;			   //also set in InitHRCounter
__int64 TICKSDISP =0;			//display time in ticks


// Display data
static BOOL PLOTFLAG = TRUE;		// TRUE for dense, FALSE for dots
int XOFF = 1;// _PIXEL / 600;			// index offset for display	
int XStart =0;						//start index of display
static int	LOX = 21;				// left upper x-corner of plot
static int	LOY = 41;				// left upper x-corner of plot
static unsigned int XLENGTH = 1100;			// x-width of  plot
static unsigned int YLENGTH = 255;			// y-width
//static unsigned int YLENGTH = 510;			// zoom y
BOOL PixelOdd = FALSE;				//display offset



// key to stop measure loop
#define _ScanCode_End	 57 //E=18   Space 57
#define _ScanCode_Cancel  01 //Q=16   ESC 01


//for ReadSoftFifo example
int ExpTime = 20; //in µs
__int16 Releasems =1; //>=1	>1 or keyboard does not work - could be exposuretime	
ULONG Threadp = 31;  //<=15  8=default,15=highest in current process,31=time critical

int TempLevel =0;
#define _COOLER FALSE

//for 2 thread display
BOOL GetNextScan = FALSE;
BOOL Running=FALSE;
BOOL UpdateDispl=FALSE;

//for trms calcs
#define TRMSpix  950 //(8-1)*150-100// _PIXEL/2	//pixel no for which the rms value is sampled

#define NOS  300  //number of samples for trms calcs
ULONG m_lfdTrmsNr=0;
double TRMSval[2];
ULONG TRMSVals[2][NOS];
BOOL ShowTrms=FALSE;

//globals for cds setup
//BOOL m_SHA = TRUE;  //TRUE for SHA, FALSE for CDS
//UINT m_Amp =2;
//int m_Ofs =-50;
//UINT m_TIgain =0;

// globals for EC control with FIFO
ULONG	tDAT=0; // delay after trigger
ULONG	tXDLY=0; // exposure control for special sensors: PDA, ILC6, TH78xx
//ULONG   tECADJ=0; //delay adjust for EC
BYTE	tTICNT=0; // trigger input divider
BYTE	tTOCNT=0; // trigger output divider
int m_TOmodus=1; //trigger out plug signal
int m_ECTrigmodus=1;
int m_ECmodus=1;

BOOL m_noPDARS=FALSE;



// global declarations for CCDEXAMP
HDC hMSDC;	// global stored measure DC of our window
HWND hMSWND; // global stored measure HWND of our window
HANDLE hTHREAD;
HANDLE hPROCESS;

HANDLE hCopyToDispBuf ;//Mutex for data buffer write



#define USERBUFINSCANS 1000
#define DMABufSizeInScans  100
#define IntFreqInScans  (DMABufSizeInScans/4) //DMABufSizeInScans / 2;
//#define DMABufSize 600 *_PIXEL * 2
#define HWINTR_EN TRUE
#define IS_DLL FALSE

#define HWDREQ_EN TRUE		// enables hardware start of DMA by XCK h->l slope
#define INTR_EN TRUE		// enables INTR

//globals
WORD UserBufInScans;
INT_PTR pDMABigBufIndex = NULL;
//INT_PTR pDMABigBufBase = NULL;


//jungo

USHORT DMAUserBuf[1200][USERBUFINSCANS];//not for dll
WORD UserBufInScans = USERBUFINSCANS;
//DMAUserBuf is the complete memory of the application
PUSHORT pDMABigBuf = &DMAUserBuf[0][0]; //not for dll
PUSHORT pDMABigBufBase = &DMAUserBuf[0][0]; //not for dll

//DWORD dwDMABufSize;// = 100 * RAMPAGESIZE * 2;// 100: ringbufsize 2:because  we need the size in bytes
ULONG DisplData[2][1200];//array for display for 2 cams parallel

//delete after deleting ringreadthread									// array type is defined in board.h
ArrayT DIODEN[_MAXDB][1200];		// global data array, could be 1 or 2 dim
ArrayT DIODENRingBuf[(DMABufSizeInScans + 10) * 1200 * sizeof(USHORT)];
DWORD FirstPageOffset;
pArrayT pDIODEN = (pArrayT)&DIODEN;

int Nob = 10;
int Nospb = 100;
pArrayT pBLOCKBUF = 0;






// Prototypes 
LRESULT CALLBACK WndProc  (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About    (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SetupMeasure( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam);

LRESULT CALLBACK SetupTLevel( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam);

LRESULT CALLBACK SetupEC( HWND hDlg,           
                        UINT message,        
                        WPARAM wParam,       
                        LPARAM lParam);
LRESULT CALLBACK AllocateBuf(HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

