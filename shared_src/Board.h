#pragma once
//  Board.h
//	all high level functions for managing Interfaceboard


#include "Board_ll.h"
#include "UIAbstractionLayer.h"

#define DBGNOCAM FALSE	//TRUE if debug with no camera - geht nicht ohne gegenseite: kein clk!
#define DMA_BUFSIZEINSCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans
#define DMA_HW_BUFPARTS 2
// DMA
#define DMA_CONTIGBUF TRUE		// use if DMABigBuf is set by driver (data must be copied afterwards to DMABigBuf)
#define DMA_SGBUF FALSE			// use if DMABigBuf is set by application (pointer must be passed to SetupPCIE_DMA)
#define DMA_64BIT_EN FALSE
#define _FORCETLPS128 TRUE	//only use payload size 128byte
#define LEGACY_202_14_TLPCNT FALSE
#define MANUAL_OVERRIDE_TLP FALSE // values are defined in board.c -> SetManualTLP()
#define DMA_DMASPERINTR DMA_BUFSIZEINSCANS / DMA_HW_BUFPARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb 
#define HWDREQ_EN TRUE		// enables hardware start of DMA by XCK h->l slope
#define INTR_EN TRUE		// enables INTR

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
	USHORT** pBigBufBase;
	WDC_DEVICE_HANDLE* hDev;
	//PWDC_DEVICE* pDev;
	ULONG* aPIXEL;
	ULONG* aCAMCNT;
	int* Nospb;
};

extern int newDLL;
extern USHORT** pBigBufBase;
extern int Nob;
extern int* Nospb;
extern ULONG* aCAMCNT;	// cameras parallel
extern ULONG ADRDELAY;
extern BOOL escape_readffloop;
extern BOOL CONTFFLOOP;
extern UINT32 CONTPAUSE;
extern UINT8 number_of_boards;
extern DWORD64 IsrCounter;
extern ULONG* aPIXEL;
extern BOOL Running;
extern UINT32 BOARD_SEL;

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
BOOL SetDMAReg( ULONG Data, ULONG Bitmask, ULONG Address, UINT32 drvno );
BOOL SetDMAAddrTlpRegs( UINT64 PhysAddrDMABuf64, ULONG tlpSize, ULONG no_tlps, UINT32 drvno );
BOOL SetDMAAddrTlp( UINT32 drvno );
void SetManualTLP_vars( void );
BOOL SetDMABufRegs( UINT32 drvno );
void SetDMAReset( UINT32 drvno );
void SetDMAStart( UINT32 drvno );
ULONG GetScanindex( UINT32 drvno );
void GetLastBufPart( UINT32 drvno );
void isr( UINT drvno, PVOID pData );
BOOL SetupPCIE_DMA( UINT32 drvno );
void StartPCIE_DMAWrite( UINT32 drvno );
void CleanupPCIE_DMA( UINT32 drvno );
int GetNumofProcessors();
BOOL SetGlobalVariables( UINT32 drvno, UINT32 camcnt, UINT32 pixel, UINT32 xckdelay );
BOOL SetBoardVars( UINT32 drvno );

BOOL Use_ENFFW_protection( UINT32 drvno, BOOL USE_ENFFW_PROTECT );

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
void CloseShutter(UINT32 drvno);	// set IFC=low
void OpenShutter(UINT32 drvno);		// set IFC=high
BOOL GetShutterState( UINT32 drvno );	//get the actual state
void SetSDAT( UINT32 drvno, UINT32 tin100ns ); // delay after trigger in 100ns
void ResetSDAT( UINT32 drvno ); // disable delay after trigger in S0+0x20
void SetSEC( UINT32 drvno, UINT32 ecin100ns );
void ResetSEC( UINT32 drvno );
void SetBDAT( UINT32 drvno, UINT32 tin100ns ); // delay after trigger in 100ns
void ResetBDAT( UINT32 drvno ); // disable delay after trigger in S0+0x20
void SetBEC( UINT32 drvno, UINT32 ecin100ns );
void ResetBEC( UINT32 drvno );
void SetTORReg( UINT32 drvno, BYTE fkt );
void SetISPDA( UINT32 drvno, BOOL set );		//hardware switch for IFC and VON if PDA
void SetISFFT( UINT32 drvno, BOOL set );		//hardware switch for IFC and VON if FFT
void SetSensorType( UINT32 drvno, UINT8 sensor_type );
void RsTOREG( UINT32 drvno );					//reset the TOREG - should be called before SetISPDA or SetISFFT
// FIFO functions
void initReadFFLoop( UINT32 drv, UINT32 * Blocks );
int waitForBlockTrigger( UINT32 board_sel );
int checkForPressedKeys();
void ReadFFLoop( UINT32 board_sel );
void countBlocksByHardware( UINT32 drvno );
unsigned int __stdcall ReadFFLoopThread( void *parg ); //jungo dma
//start<0 is in the past, stop>0 is in the future, relative to call of this function
BOOL BlockTrig( UINT32 drv, UINT8 btrig_ch ); //read state of trigger in signals during thread loop
void StartSTimer( UINT32 drvno );	//starts 28bit timer of PCI board
void StopSTimer( UINT32 drvno );					// stop timer
BOOL SetSTimer( UINT32 drvno, UINT32 stime_in_microseconds );
BOOL SetBTimer( UINT32 drvno, UINT32 btime_in_microseconds );
void SWTrig( UINT32 drvno );						//start a read to FIFO by software
BOOL IsTimerOn( UINT32 drvno );
BOOL FFValid( UINT32 drvno );						// TRUE if linecounter>0
BOOL FFFull( UINT32 drvno );
BOOL FFOvl( UINT32 drvno );							//TRUE if FIFO overflow since last RSFifo call
void RSFifo( UINT32 drvno );						// reset FIFO and linecounter
void SetExtFFTrig( UINT32 drvno );					// read to FIFO is triggered by external input I of PCI board
void SetIntFFTrig( UINT32 drvno );					// read to FIFO is triggered by Timer
BOOL SetupVCLKReg( UINT32 drvno, ULONG lines, UCHAR vfreq );//setup hardware vclk generator
BOOL SetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, BOOL keep);
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
void GetRmsVal( UINT32 nos, UINT16 *TRMSVals, double *mwf, double *trms );
void CalcTrms( UINT32 drvno, UINT32 nos, UINT32 TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms );
UINT32 GetIndexOfPixel( UINT32 drvno, UINT16 pixel, UINT32 sample, UINT32 block, UINT16 CAM );
void* GetAddressOfPixel( UINT32 drvno, UINT16 pixel, UINT32 sample, UINT32 block, UINT16 CAM );
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
BOOL SetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq );
BOOL SetPartialBinning( UINT32 drvno, UINT16 number_of_regions );
BOOL ResetPartialBinning( UINT32 drvno );
void InitProDLL();
BOOL isDmaSet( UINT32 drvno );
BOOL allocateUserMemory( UINT drvno );
BOOL isMeasureOn( UINT32 drvno );
BOOL isBlockOn( UINT32 drvno );
void waitForMeasureReady( UINT32 drvno );
void waitForBlockReady( UINT32 drvno );
BOOL setBlockOn( UINT32 drvno );
BOOL resetBlockOn( UINT32 drvno );
BOOL SetBTI( UINT32 drvno, UINT8 bti_mode );
BOOL SetSTI( UINT32 drvno, UINT8 sti_mode );
void ClearAllUserRegs( UINT32 drv );
BOOL SetBSlope( UINT32 drvno, UINT32 slope );
BOOL SetMeasurementParameters( UINT32 drvno, UINT32 nos, UINT32 nob );
void ReturnFrame( UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16 *pdest, UINT32 length );
void abortMeasurement( UINT32 drv );
