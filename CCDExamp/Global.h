// GLOBAL.h   V1.0
// all globals for measure loop
// can be used for PCIE board
// for CCDExample and ESLSCDLL

#define IDM_EXIT           100
#define _ERRTEST FALSE //test data for integrity

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
#define _HWCH2	TRUE	//true if 2 words are packet in one long ->resort to db1&db2 = PCI with adaptor board
						//all double line systems
#define		DRV		1	//1 if only one interface board LSCPCI1 or LSCISA1 in example
						// could be 2..4 for multiple boards
#define CAMCNT 1
#if CAMCNT == 1
BOOL DISP2 = FALSE;		//display 1 camera
#endif
#if CAMCNT == 2
BOOL DISP2 = TRUE;		//display 2 cameras parallel, TRUE for double line 
#endif
#define _MAXDB	4					// no. of lines
//#define Nos _MAXDB
#define DMA_64BIT_EN FALSE
#define KER_MODE FALSE
//for jungo projects
#define KERNEL_64BIT	
#define WINNT
//#define HWINTR_EN TRUE
#define HWDREQ_EN TRUE		// enables hardware start of DMA by XCK h->l slope
#define INTR_EN TRUE		// enables INTR
#define MAXPCIECARDS 5
typedef USHORT ArrayT; //!! USHORT for linear 12/16bit word array or resort or highest speed
BOOL contimess_run_once = FALSE;
typedef ArrayT* pArrayT;
//!! long for standard 
//this is the read from FIFO frequency
static BOOL SYM_PULSE = FALSE;		// read FIFO is allways higest speed
static BOOL BURSTMODE = TRUE;
static 	unsigned long WAITS = 0x0;
//valid for FIFO mode only
//static	unsigned long PCLK = 5;	
//this is the write to FIFO frequency / FREQ=datarate=pclk*2 if 12/16bit
static ULONG FREQ = 0; //=waits -> 0=33MHz;1=16MHz;2=8MHz max 6=0 
static ULONG DELAYini = 0;			//DELAY for WRFIFO
//only for ISA
static	unsigned long FOURMAX = _MAXDB * _PIXEL;
#define PPORTADR 0 //0x378	//PrinterPortaddress or 0 if ISA Board
// settings for 8 Bit cameras
//static	unsigned long FLAG816 = 2;  // 2=8Bit, 1=12/16Bit
//static int YSHIFT = 0;				// 8Bit=0, 12Bit=4 or more for ddrep>1
//static int YSHIFT = 4;				// 12Bit=4 or more for addrep>1
static int YSHIFT = 8;			// 16bit=8
//static int YSHIFT = 6;			// 14 bit
// camera values for calling GETCCD and InitBoard
static	unsigned long _FKT = 1;		// -1:clearread, 0:datab=0, 1:read 5: testdata
									//, 2:add ; not implemented in DMA is 3:sub 
#define _FFTLINES 64 		// no of vertical lines of FFT sensors, usually 64
							// =0 if not FFT
#define _ISPDA FALSE			//set RS after read; TRUE for HA S39xx
#define _ISFFT TRUE		//set vclk generator; TRUE for HA S703x
BOOL _IsArea = FALSE; //FALSE is just the init val
__int16 _IsROI = 0; //FALSE is just the init val
//#define _ISAREA FALSE	//set AREA Mode on CAMERA
//#define _HA_MODULE FALSE		//TRUE for HA module C7041 or C8061
//vclk frequency 

#define Vfreqini 7		//vclk freq for FFTs with FIFO in divider of12MHz (0..15)
						//=3 for highest speed with 7030-0906
//#define _ARRAYLINES 0		//=0 only for Andanta IR area sensor =256
//static	unsigned long ZADR = 1;		// Adress for adressed mode
									//for IR ZADR=0 -> both channels
// parameter for Loop
static int ADDREP = 1;			 	//addition loop for fkt=2; 1 else
enum trigger_mode
{
	xck = 0,
	exttrig = 1,
	dat = 2
};
#define TRIGGER_MODE xck
#if TRIGGER_MODE == 1
BOOL EXTTRIGFLAG = TRUE;		// run with external Trigger
#else
BOOL EXTTRIGFLAG = FALSE;		// run with external Trigger
#endif
									// DELAYMS is here wait after trigger!
int TrigMod = 0;						//pos slope
//static BOOL HISLOPE = TRUE;			// Slope for external Trigger
static BOOL HIAMP = FALSE;			// Amplification for switchable sensors
//__int64 DELAY = 0;			   //also set in InitHRCounter
__int64 TICKSDISP = 0;			//display time in ticks
// Display data
static BOOL PLOTFLAG = TRUE;		// TRUE for dense, FALSE for dots
int XOFF = 1;// _PIXEL / 600;			// index offset for display	
int XStart = 0;						//start index of display
static int	LOX = 21;				// left upper x-corner of plot
static int	LOY = 41;				// left upper x-corner of plot
static unsigned int XLENGTH = _PIXEL + 50;			// x-width of  plot 
static unsigned int YLENGTH = 255;			// y-width
//static unsigned int YLENGTH = 510;			// zoom y
BOOL PixelOdd = FALSE;				//display offset
// key to stop measure loop
#define _ScanCode_End	 57 //E=18   Space 57
#define _ScanCode_Cancel  01 //Q=16   ESC 01
//for ReadSoftFifo example
int ExpTime = 1000; //in µs
int RepTime = 1 * _MINREPTIME; //in ms for _MSHUT
__int16 Releasems = 1; //>=1	>1 or keyboard does not work - could be exposuretime	
ULONG Threadp = 31;  //<=15  8=default,15=highest in current process,31=time critical
int TempLevel = 0;
#define _COOLER FALSE
//for 2 thread display
BOOL GetNextScan = FALSE;
BOOL UpdateDispl = FALSE;
//for trms calcs
#define TRMSpix  1 //(8-1)*150-100// _PIXEL/2	//pixel no for which the rms value is sampled
#define NOS  300  //number of samples for trms calcs
ULONG m_lfdTrmsNr = 0;
double TRMSval[2];
ULONG TRMSVals[2][NOS];
BOOL ShowTrms = FALSE;
//globals for cds setup
//BOOL m_SHA = TRUE;  //TRUE for SHA, FALSE for CDS
//UINT m_Amp =2;
//int m_Ofs =-50;
//UINT m_TIgain =0;
// globals for EC control with FIFO
ULONG	tDAT = 0; // delay after trigger
ULONG	tXDLY = 0; // exposure control for special sensors: PDA, ILC6, TH78xx
//ULONG   tECADJ=0; //delay adjust for EC
BYTE	tTICNT = 0; // trigger input divider
BYTE	tTOCNT = 0; // trigger output divider
int m_TOmodus = 1; //trigger out plug signal
int m_ECTrigmodus = 1;
int m_ECmodus = 1;
BOOL m_noPDARS = FALSE;
// global declarations for CCDEXAMP
HDC hMSDC;	// global stored measure DC of our window
HWND hMSWND; // global stored measure HWND of our window
HANDLE hTHREAD;
HANDLE hPROCESS;
HANDLE hCopyToDispBuf;//Mutex for data buffer write
//#define USERBUFINSCANS 1000
//#define DMABufSizeInScans  1000
//globals
WORD UserBufInScans;
//INT_PTR pDMABigBufBase = NULL;
//jungo
//USHORT DMAUserBuf[1200][USERBUFINSCANS];//not for dll
//WORD UserBufInScans = USERBUFINSCANS;
//DMAUserBuf is the complete memory of the application
//PUSHORT pDMABigBuf; //not for dll
//DWORD dwDMABufSize;// = 100 * RAMPAGESIZE * 2;// 100: ringbufsize 2:because  we need the size in bytes
ULONG DisplData[2][1200 * CAMCNT];//array for display for 2 cams parallel
//delete after deleting ringreadthread									// array type is defined in board.h
//ArrayT DIODEN[_MAXDB][1200];		// global data array, could be 1 or 2 dim
//ArrayT DIODENRingBuf[(DMABufSizeInScans + 10) * 1200 * sizeof(USHORT)];
DWORD FirstPageOffset;
//pArrayT pDIODEN = (pArrayT)&DIODEN;
#if _MSHUT == TRUE
//Nospb = 100;
#else
//Nospb = 1000;
#endif
#define _FORCETOPLS128 TRUE	//only use payload size 128byte
ULONG NO_TLPS = 0x12; //was 0x11-> x-offset			//0x11=17*128  = 2176 Bytes  = 1088 WORDS
ULONG TLPSIZE = 0x20;


