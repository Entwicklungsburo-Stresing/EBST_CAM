#pragma once
//  Board.h
//	all high level functions for managing Interfaceboard
#ifdef __cplusplus
extern "C" {
#endif

#include "Board_ll_old.h"
#include "UIAbstractionLayer.h"

#define DBGNOCAM FALSE	//TRUE if debug with no camera - geht nicht ohne gegenseite: kein clk!
#define DMA_BUFFER_SIZE_IN_SCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans
#define DMA_BUFFER_PARTS 2
// DMA
#define DMA_CONTIGBUF TRUE		// use if DMABigBuf is set by driver (data must be copied afterwards to DMABigBuf)
#define DMA_SGBUF FALSE			// use if DMABigBuf is set by application (pointer must be passed to SetupPCIE_DMA)
#define DMA_64BIT_EN FALSE
#define _FORCETLPS128 TRUE	//only use payload size 128byte
#define LEGACY_202_14_TLPCNT FALSE
#define MANUAL_OVERRIDE_TLP FALSE // values are defined in board.c -> SetManualTLP()
#define DMA_DMASPERINTR DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb 
#define HWDREQ_EN TRUE		// enables hardware start of DMA by XCK h->l slope
#define INTR_EN TRUE		// enables INTR

struct global_vars
{
	USHORT** userBuffer;
	WDC_DEVICE_HANDLE* hDev;
	//PWDC_DEVICE* pDev;
	ULONG* aPIXEL;
	ULONG* aCAMCNT;
	UINT32* Nospb;
	BOOL* useSWTrig;
};

extern UINT16** userBuffer;
extern UINT32 Nob;
extern UINT32* Nospb;
extern UINT32* aCAMCNT;	// cameras parallel
extern BOOL escape_readffloop;
extern BOOL CONTFFLOOP;
extern UINT32 CONTPAUSE;
extern UINT8 number_of_boards;
extern DWORD64 IsrCounter;
extern UINT32* aPIXEL;
extern BOOL Running;
extern UINT32 BOARD_SEL;
extern BOOL* useSWTrig;

//High level API
es_status_codes InitBoard(UINT32 drvno);	// init the board and alloc mem, call only once !
es_status_codes CCDDrvInit();
es_status_codes CCDDrvExit(UINT32 drvno);	// closes the driver
es_status_codes InitMeasurement(struct global_settings settings);
es_status_codes StartMeasurement();
es_status_codes abortMeasurement(UINT32 drv);
es_status_codes ReturnFrame(UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16* pdest, UINT32 length);

//Mid level API
void ErrMsgBoxOn();
void ErrMsgBoxOff(); // switch to suppress error message boxes
void ErrorMsg( char ErrMsg[100] );
void ValMsg( UINT64 val );
void AboutDMARegs( UINT32 drv );
es_status_codes AboutTLPs( UINT32 drvno );
es_status_codes AboutS0( UINT32 drvno );
//  same header file for ISA and PCI version
es_status_codes SetDMAReg( ULONG Data, ULONG Bitmask, ULONG Address, UINT32 drvno );
es_status_codes SetDMAAddrTlpRegs( UINT64 PhysAddrDMABuf64, ULONG tlpSize, ULONG no_tlps, UINT32 drvno );
es_status_codes SetDMABufRegs( UINT32 drvno );
es_status_codes SetDMAReset( UINT32 drvno );
es_status_codes SetDMAStart( UINT32 drvno );
ULONG GetScanindex( UINT32 drvno );
es_status_codes GetLastBufPart( UINT32 drvno );
void isr( UINT drvno, PVOID pData );
es_status_codes SetupPCIE_DMA( UINT32 drvno );
void StartPCIE_DMAWrite( UINT32 drvno );
es_status_codes CleanupPCIE_DMA( UINT32 drvno );
int GetNumofProcessors();
es_status_codes SetTLPS( UINT32 drvno, UINT32 pixel );
es_status_codes SetPixelCount(UINT32 drvno, UINT16 pixelcount);
es_status_codes SetCamCount(UINT32 drvno, UINT16 camcount);
es_status_codes Use_ENFFW_protection(UINT32 drvno, BOOL USE_ENFFW_PROTECT);
// clear camera with reads
es_status_codes AboutDrv( UINT32 drvno );	// displays the version and board ID = test if board is there
//	functions for managing controlbits in CtrlA register
es_status_codes OutTrigLow( UINT32 drvno );		//set output Trigger signal low
es_status_codes OutTrigHigh( UINT32 drvno );		//set output Trigger signal high
es_status_codes OutTrigPulse( UINT32 drvno, ULONG PulseWidth );	// pulses high output Trigger signal
es_status_codes WaitTrigger( UINT32 drvno, BOOL ExtTrigFlag, BOOL *SpaceKey, BOOL *EscapeKey );
// waits for trigger input or Key
es_status_codes CloseShutter(UINT32 drvno);	// set IFC=low
es_status_codes OpenShutter(UINT32 drvno);		// set IFC=high
BOOL GetShutterState( UINT32 drvno );	//get the actual state
es_status_codes SetSDAT( UINT32 drvno, UINT32 tin100ns ); // delay after trigger in 100ns
es_status_codes SetSEC( UINT32 drvno, UINT32 ecin100ns );
es_status_codes SetBDAT( UINT32 drvno, UINT32 tin100ns ); // delay after trigger in 100ns
es_status_codes SetBEC( UINT32 drvno, UINT32 ecin100ns );
es_status_codes SetTORReg( UINT32 drvno, BYTE fkt );
es_status_codes SetSensorType( UINT32 drvno, UINT8 sensor_type );
// FIFO functions
es_status_codes initReadFFLoop( UINT32 drv );
int waitForBlockTrigger( UINT32 board_sel );
int checkForPressedKeys();
es_status_codes ReadFFLoop( UINT32 board_sel );
es_status_codes countBlocksByHardware( UINT32 drvno );
unsigned int __stdcall ReadFFLoopThread(); //jungo dma
//start<0 is in the past, stop>0 is in the future, relative to call of this function
es_status_codes readBlockTriggerState( UINT32 drv, UINT8 btrig_ch, BOOL* state); //read state of trigger in signals during thread loop
es_status_codes StartSTimer( UINT32 drvno );	//starts 28bit timer of PCI board
es_status_codes StopSTimer( UINT32 drvno );					// stop timer
es_status_codes SetSTimer( UINT32 drvno, UINT32 stime_in_microseconds );
es_status_codes SetBTimer( UINT32 drvno, UINT32 btime_in_microseconds );
es_status_codes SWTrig( UINT32 drvno );						//start a read to FIFO by software
es_status_codes IsTimerOn( UINT32 drvno, BOOL* on );
es_status_codes checkFifoFlags( UINT32 drvno, BOOL* valid );						// TRUE if linecounter>0
BOOL FFFull( UINT32 drvno );
es_status_codes checkFifoOverflow(UINT32 drvno, BOOL* overflow);							//TRUE if FIFO overflow since last RSFifo call
es_status_codes RSFifo( UINT32 drvno );						// reset FIFO and linecounter
es_status_codes SetupVCLKReg( UINT32 drvno, ULONG lines, UCHAR vfreq );//setup hardware vclk generator
es_status_codes SetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, BOOL keep);
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
es_status_codes SetTemp( UINT32 drvno, UINT8 level );				//set temperature - 8 levels possible
es_status_codes RS_ScanCounter( UINT32 drv );
es_status_codes RS_BlockCounter( UINT32 drv );
es_status_codes RS_DMAAllCounter( UINT32 drv, BOOL hwstop );
es_status_codes FindCam( UINT32 drv );
es_status_codes SetADGain( UINT32 drvno, UINT8 fkt, UINT8 g1, UINT8 g2, UINT8 g3, UINT8 g4, UINT8 g5, UINT8 g6, UINT8 g7, UINT8 g8 );//B!test
es_status_codes SendFLCAM_DAC( UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature ); //function for sending 32 bits to DAC8568 in HS-FLCAM on PCB 2189-7(+)
es_status_codes DAC_setOutput( UINT32 drvno, UINT8 channel, UINT16 output ); //set output of DAC (PCB 2189-7)
void FreeMemInfo( UINT64 *pmemory_all, UINT64 *pmemory_free );
void GetRmsVal( UINT32 nos, UINT16 *TRMSVals, double *mwf, double *trms );
es_status_codes CalcTrms( UINT32 drvno, UINT32 firstSample, UINT32 lastSample, UINT32 TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms );
es_status_codes GetIndexOfPixel( UINT32 drvno, UINT16 pixel, UINT32 sample, UINT32 block, UINT16 CAM, UINT64* pIndex );
es_status_codes GetAddressOfPixel(UINT32 drvno, UINT16 pixel, UINT32 sample, UINT32 block, UINT16 CAM, UINT16** address);
UINT8 WaitforTelapsed( LONGLONG musec );
es_status_codes initCamera(UINT32 drvno, UINT8 camera_system, UINT16 pixel, UINT16 trigger_mode, UINT16 sensor_type, UINT8 ADC_Mode, UINT16 ADC_custom_pettern, UINT16 led_off, UINT16 gain_switch, UINT8 gain_3030);
es_status_codes InitCameraGeneral( UINT32 drvno, UINT16 pixel, UINT16 cc_trigger_input, UINT8 IS_FFT, UINT8 IS_AREA, UINT8 IS_COOLED );
es_status_codes InitCamera3001( UINT32 drvno, UINT8 gain_switch);
es_status_codes InitCamera3010( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern);
es_status_codes Cam3010_ADC_reset(UINT32 drvno);
es_status_codes Cam3010_ADC_setMode(UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern);
es_status_codes InitCamera3030( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain );
es_status_codes Cam3030_ADC_reset(UINT32 drvno);
es_status_codes Cam3030_ADC_twoWireModeEN(UINT32 drvno);
es_status_codes Cam3030_ADC_SetGain(UINT32 drvno, UINT8 gain);
es_status_codes Cam3030_ADC_RampOrPattern( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern);
es_status_codes SetGPXCtrl( UINT32 drvno, UINT8 GPXAddress, UINT32 GPXData );
es_status_codes ReadGPXCtrl( UINT32 drvno, UINT8 GPXAddress, UINT32* GPXData );
es_status_codes InitGPX( UINT32 drvno, UINT32 delay );
es_status_codes AboutGPX( UINT32 drvno );
double CalcRamUsageInMB( UINT32 nos, UINT32 nob );
double CalcMeasureTimeInSeconds( UINT32 drvno, UINT32 nos, double exposure_time_in_ms );
es_status_codes SetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq );
BOOL SetPartialBinning( UINT32 drvno, UINT16 number_of_regions );
es_status_codes ResetPartialBinning( UINT32 drvno );
void InitProDLL();
BOOL isDmaSet( UINT32 drvno );
es_status_codes allocateUserMemory( UINT32 drvno );
es_status_codes isMeasureOn( UINT32 drvno, BOOL* measureOn );
es_status_codes isBlockOn( UINT32 drvno, BOOL* blockOn );
es_status_codes waitForMeasureReady( UINT32 drvno );
es_status_codes waitForBlockReady( UINT32 drvno );
es_status_codes setBlockOn( UINT32 drvno );
es_status_codes resetBlockOn( UINT32 drvno );
es_status_codes setMeasureOn(UINT32 drvno);
es_status_codes resetMeasureOn(UINT32 drvno);
es_status_codes SetBTI( UINT32 drvno, UINT8 bti_mode );
es_status_codes SetSTI( UINT32 drvno, UINT8 sti_mode );
es_status_codes ClearAllUserRegs( UINT32 drv );
es_status_codes SetBSlope( UINT32 drvno, UINT32 slope );
es_status_codes SetSSlope(UINT32 drvno, UINT32 sslope);
es_status_codes SetMeasurementParameters( UINT32 drvno, UINT32 nos, UINT32 nob );
es_status_codes SetGain( UINT32 drvno, UINT16 gain_value );
es_status_codes LedOff( UINT32 drvno, UINT8 LED_OFF );
es_status_codes SetXckdelay(UINT32 drvno, UINT32 xckdelay);
es_status_codes checkDriverHandle(UINT32 drvno);

#ifdef __cplusplus
}
#endif