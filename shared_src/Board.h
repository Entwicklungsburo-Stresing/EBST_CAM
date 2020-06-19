#pragma once
//  Board.h				PCI V2.31
//	all functions for managing Interfaceboard
//	with & without Fifo  
//  new: data array ushort
//2.31: 64 bit dma acces

***REMOVED***#define LSCPCIEJ_STRESING_DRIVER_NAME "lscpciej"

//#include "stdafx.h"		// use in C++ only
//#include "global.h"		// use in C++ only
#include "ccdctl.h" //"ccdctrl.h"
#include <limits.h>
#include <process.h>
#include "Jungo/windrvr.h"
#include "Jungo/wdc_lib.h"
#include "Jungo/wdc_defs.h"
#include "wchar.h"
#include "lscpciej_lib.h"
#include "shared_src/ESLSCDLL_pro.h"
//#include "kp_lscpciej.c"

//#include "wd_kp.h"
//siehe beginn functions
//#include "lscpciej_lib.h" 

//Dont trust the debugger its CRAP

/* Error messages display */
#define LSCPCIEJ_ERR printf
// use LSCPCI1 on PCI Boards
#define	DRIVERNAME	"\\\\.\\LSCPCIE"

//try different methodes - only one can be TRUE!
#define DMA_CONTIGBUF TRUE		// use if DMABigBuf is set by driver (data must be copied afterwards to DMABigBuf)
#define DMA_SGBUF FALSE			// use if DMABigBuf is set by application (pointer must be passed to SetupPCIE_DMA)

// ExpTime is passed as global var here
// function is not used in WCCD
#ifndef ExpTime
ULONG ExpTime; //in micro sec - needed only in DLL, defined in DLL.h
#endif
#define BoardType  "PCI"
#define BoardVN  "2.31"
#define MAXPCIECARDS 5
#define Vfreqini 7			// only FIFO set in SetupVCLKReg
							//frequence of vclks for FFTs S703x
#define DMA_64BIT_EN FALSE
#define _FORCETOPLS128 TRUE	//only use payload size 128byte
#define DMA_BUFSIZEINSCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans
#define DMA_HW_BUFPARTS 2
#define DMA_DMASPERINTR DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb 
#define HWDREQ_EN TRUE		// enables hardware start of DMA by XCK h->l slope
#define INTR_EN TRUE		// enables INTR
#define DBGNOCAM	FALSE	//TRUE if debug with no camera - geht nicht ohne gegenseite: kein clk!
//TRUE if you use PS2 Keyboard, FALSE else
// on WinNT/2000/xp the function GetAsyncKeystate works only
// if the Thread Class is not highest or 
// if the Sleep function is used inside Contimess->Measure
// so here a special hardware read of PS2Keyboard is used
// to avoid interrupt dependend routines; look BOARD->WaitTrigger
#define _PS2KEYBOARD FALSE 

struct ffloopparams
{
	UINT32 board_sel;
	UINT32 exptus;
	UINT8 exttrig;
	UINT8 blocktrigger;
	UINT8 btrig_ch;
};

struct global_vars
{
	USHORT** pDMABigBufBase;
};

typedef USHORT ArrayT; //!! USHORT for linear 12/16bit word array or resort or highest speed
typedef ArrayT* pArrayT;

extern int newDLL;
extern USHORT** pDMABigBufBase;
extern int Nob;
extern int Nospb;
extern ULONG aCAMCNT[5];	// cameras parallel
extern BOOL escape_readffloop;
extern BOOL contffloop;
extern UINT8 NUMBER_OF_BOARDS;
extern DWORD64 IsrCounter;
extern ULONG aPIXEL[5];	// pixel
extern BOOL Running;
extern pArrayT pBLOCKBUF[3];
extern UINT32 BOARD_SEL;

static ULONG XCKDELAY = 3; //100ns+n*400ns, 1<n<8 Sony=7

void ErrMsgBoxOn( void );
void ErrMsgBoxOff( void ); // switch to suppress error message boxes
void ErrorMsg( char ErrMsg[100] );
void ValMsg( UINT64 val );
void AboutDMARegs( UINT32 drv );
void AboutTLPs( UINT32 drvno );
void AboutS0( UINT32 drvno );
//  same header file for ISA and PCI version
BOOL CCDDrvInit( void );
void CCDDrvExit( UINT32 drvno );	// closes the driver
BOOL InitBoard( UINT32 drvno );	// init the board and alloc mem, call only once !
char CntBoards( void );
BOOL SetDMAReg( ULONG Data, ULONG Bitmask, ULONG Address, UINT32 drvno );
BOOL SetS0Reg( ULONG Data, ULONG Bitmask, CHAR Address, UINT32 drvno );
BOOL SetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
BOOL ResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
BOOL SetDMAAddrTlpRegs( UINT64 PhysAddrDMABuf64, ULONG tlpSize, ULONG no_tlps, UINT32 drvno );
BOOL SetDMAAddrTlp( UINT32 drvno );
BOOL SetDMABufRegs( UINT32 drvno, ULONG nos, ULONG nob, ULONG camcnt );
void SetDMAReset( UINT32 drvno );
void SetDMAStart( UINT32 drvno );
ULONG GetScanindex( UINT32 drvno );
void GetLastBufPart( UINT32 drvno );
void isr( UINT drvno, PVOID pData );
BOOL SetupPCIE_DMA( UINT32 drvno, ULONG nos, ULONG nob );
void StartPCIE_DMAWrite( UINT32 drvno );
void CleanupPCIE_DMA( UINT32 drvno );
int GetNumofProcessors();
void RSInterface( UINT32 drvno );		//set all registers to zero
BOOL SetBoardVars( UINT32 drvno, UINT32 camcnt, ULONG pixel, ULONG xckdelay );
void Resort( UINT32 drvno, void* ptarget, void* psource );
BOOL CallWRFile( UINT32 drvno, void* pdioden, ULONG arraylength, ULONG fkt );
BOOL CallIORead( UINT32 drvno, void* pdioden, ULONG fkt );
BOOL ReadLongIOPort( UINT32 drvno, ULONG *DWData, ULONG PortOff );// read long from IO runreg
BOOL ReadLongS0( UINT32 drvno, UINT32 * DWData, ULONG PortOff );	// read long from space0
BOOL ReadLongDMA( UINT32 drvno, UINT32 * DWData, ULONG PortOff );
BOOL ReadByteS0( UINT32 drvno, BYTE *data, ULONG PortOff );	// read byte from space0
BOOL WriteLongIOPort( UINT32 drvno, ULONG DataL, ULONG PortOff );// write long to IO runreg
BOOL WriteLongS0( UINT32 drvno, UINT32 DWData, ULONG PortOff );// write long to space0
BOOL WriteLongDMA( UINT32 drvno, ULONG DWData, ULONG PortOff );
BOOL WriteByteS0( UINT32 drv, BYTE DataByte, ULONG PortOff ); // write byte to space0
// clear camera with reads
void AboutDrv( UINT32 drvno );	// displays the version and board ID = test if board is there
//	functions for managing controlbits in CtrlA register
void LowSlope( UINT32 drvno );		//set input Trigger slope low
void HighSlope( UINT32 drvno );		//set input Trigger slope high
void BothSlope( UINT32 drvno );		//set trigger on both slopes 
void NotBothSlope( UINT32 drvno );		//set trigger on both slopes off
void OutTrigLow( UINT32 drvno );		//set output Trigger signal low
void OutTrigHigh( UINT32 drvno );		//set output Trigger signal high
void OutTrigPulse( UINT32 drvno, ULONG PulseWidth );	// pulses high output Trigger signal
void WaitTrigger( UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *EscapeKey );
// waits for trigger input or Key
void WaitTriggerShort( UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *EscapeKey );
void EnTrigShort( UINT32 drvno );
void RSTrigShort( UINT32 drvno );
void DisTrigShort( UINT32 drvno );
void CloseShutter( UINT32 drvno );	// set IFC=low
void OpenShutter( UINT32 drvno );		// set IFC=high
BOOL GetShutterState( UINT32 drvno );	//get the actual state
void V_On( UINT32 drvno );			// set V_On signal low (V = V_Fak)
void V_Off( UINT32 drvno );			// set V_On signal high (V = 1)
void SetOpto( UINT32 drvno, BYTE ch );  // set opto channel if output
void RsetOpto( UINT32 drvno, BYTE ch ); // reset opto channel if output
BOOL GetOpto( UINT32 drvno, BYTE ch );	//read opto channel if input
void SetDAT( UINT32 drvno, UINT32 tin100ns ); // delay after trigger in 100ns
void RSDAT( UINT32 drvno ); // disable delay after trigger in S0+0x20
void SetEC( UINT32 drvno, UINT32 ecin100ns );
void ResetEC( UINT32 drvno );
void SetTORReg( UINT32 drvno, BYTE fkt );
void SetISPDA( UINT32 drvno, BOOL set );		//hardware switch for IFC and VON if PDA
void SetISFFT( UINT32 drvno, BOOL set );		//hardware switch for IFC and VON if FFT
void SetPDAnotFFT( UINT32 drvno, BOOL set );
void RsTOREG( UINT32 drvno );					//reset the TOREG - should be called before SetISPDA or SetISFFT
void SendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data );
void RSEC( UINT32 drvno );
BOOL CheckFFTrig( UINT32 drvno );		// trigger sets FF - clear via write CtrlA 0x10
// new Keyboard read which is not interrupt dependend
// reads OEM scan code directly on port 0x60
// FIFO functions
void StartReadWithDma( UINT32 drvno );
void StopRingReadThread( void ); //starts and ends background thread 
void initReadFFLoop( UINT32 drv, UINT32 exptus, UINT8 exttrig, UINT32 * Blocks );
void allBlocksOnSingleTrigger( UINT32 board_sel, UINT8 btrig_ch, BOOL* StartByTrig );
void oneTriggerPerBlock( UINT32 board_sel, UINT8 btrig_ch );
int  keyCheckForBlockTrigger( UINT32 board_sel );
void ReadFFLoop( UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch );
unsigned int __stdcall ReadFFLoopThread( void *parg ); //jungo dma
ULONG GetLastMaxLines( void );
UINT64 GetISRTime( void );
//start<0 is in the past, stop>0 is in the future, relative to call of this function
BOOL BlockTrig( UINT32 drv, UINT8 btrig_ch ); //read state of trigger in signals during thread loop
void SetExtSWTrig( BOOL ext );
void StartFFTimer( UINT32 drvno, UINT32 exptime );	//starts 28bit timer of PCI board
void SWTrig( UINT32 drvno );						//start a read to FIFO by software
void StopFFTimer( UINT32 drvno );					// stop timer
BOOL IsTimerOn( UINT32 drvno );
BOOL FFValid( UINT32 drvno );						// TRUE if linecounter>0
BOOL FFFull( UINT32 drvno );
BOOL FFOvl( UINT32 drvno );							//TRUE if FIFO overflow since last RSFifo call
void RSFifo( UINT32 drvno );						// reset FIFO and linecounter
void SetExtFFTrig( UINT32 drvno );					// read to FIFO is triggered by external input I of PCI board
void SetIntFFTrig( UINT32 drvno );					// read to FIFO is triggered by Timer
BOOL SetupVCLKReg( UINT32 drvno, ULONG lines, UCHAR vfreq );//setup hardware vclk generator
void SetupVCLKrt( ULONG vfreq );					//setup vclkfreq for rt version(noFIFO)
BOOL SetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, BOOL keep);
void SetupDELAY( UINT32 drvno, ULONG delay );		//setup DELAY for WRFIFO
void SetupHAModule( BOOL irsingle, ULONG fftlines );//set the module C8061&C7041 inits
BOOL ThreadToPriClass( ULONG threadp, DWORD *priclass, DWORD *prilevel );
// Class & Thread priority functions
BOOL SetPriority( ULONG threadp );		//set priority threadp 1..31 / 8 = normal and keep old in global variable
UINT64 LargeToInt( LARGE_INTEGER li );
// System Timer
UINT64 InitHRCounter();				//init system counter and returns TPS: ticks per sec
UINT64 ticksTimestamp();				//reads actual ticks of system counter
UINT64 ustoTicks( ULONG us );			//calcs microsec to ticks  
UINT32 Tickstous( UINT64 tks );			//calcs ticks to microsec
// Cooler& special functions
BOOL TempGood( UINT32 drvno, UINT32 ch );						//high if temperature is reached
void SetTemp( UINT32 drvno, UINT8 level );				//set temperature - 8 levels possible
void RS_ScanCounter( UINT32 drv );
void RS_BlockCounter( UINT32 drv );
void RS_DMAAllCounter( UINT32 drv, BOOL hwstop );
BOOL FindCam( UINT32 drv );
void SetADGain( UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8 );//B!test
void SendFLCAM_DAC( UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature ); //function for sending 32 bits to DAC8568 in HS-FLCAM on PCB 2189-7(+)
void DAC_setOutput( UINT32 drvno, UINT8 channel, UINT16 output ); //set output of DAC (PCB 2189-7)
void FreeMemInfo( UINT64 *pmemory_all, UINT64 *pmemory_free );
void GetRmsVal( ULONG nos, ULONG *TRMSVals, double *mwf, double *trms );
void CalcTrms( UINT32 drvno, UINT32 nos, UINT16 TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms );
UINT32 GetIndexOfPixel( UINT32 drvno, UINT16 pixel, UINT16 sample, UINT16 block, UINT16 CAM );
void* GetAddressOfPixel( UINT32 drvno, UINT16 pixel, UINT16 sample, UINT16 block, UINT16 CAM );
UINT8 WaitforTelapsed( LONGLONG musec );
void InitCamera3001( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT16 IS_FFT, UINT16 IS_AREA );
void InitCamera3010( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT8 adc_mode, UINT16 custom_pattern, UINT16 led_on, UINT16 gain_high );
void Cam3010_ADC_reset(UINT32 drvno);
void Cam3010_ADC_setMode(UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern);
void InitCamera3030( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain );
void Cam3030_ADC_reset(UINT32 drvno);
void Cam3030_ADC_twoWireModeEN(UINT32 drvno);
void Cam3030_ADC_SetGain(UINT32 drvno, UINT8 gain);
void Cam3030_ADC_RampOrPattern( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern);
BOOL SetGPXCtrl( UINT32 drvno, UINT8 GPXAddress, UINT32 GPXData );
BOOL ReadGPXCtrl( UINT32 drvno, UINT8 GPXAddress, UINT32* GPXData );
void InitGPX( UINT32 drvno, UINT32 delay );
void AboutGPX( UINT32 drvno );
double CalcRamUsageInMB( UINT32 nos, UINT32 nob );
double CalcMeasureTimeInSeconds( UINT32 drvno, UINT32 nos, double exposure_time_in_ms );
void BlockSyncStart( UINT32 drvno, UINT8 S1, UINT8 S2 );
BOOL SetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq );
BOOL SetPartialBinning( UINT32 drvno, UINT16 number_of_regions );
BOOL ResetPartialBinning( UINT32 drvno );
BOOL AutostartXckForLines( UINT32 drvno );
BOOL ResetAutostartXck( UINT32 drvno );
void InitProDLL();
BOOL isDmaSet( UINT32 drvno );
BOOL BufLock( UINT drvno, UINT camcnt, int nob, int nospb );