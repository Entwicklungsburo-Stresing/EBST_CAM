//  DLL header    V 3.3    2/13

/*   **********************************************
	DLL for CCD Camera driver of
	for linking to labview

  Entwicklungsbuero Stresing
  Germany

 Version V2.022  3/2017

  this DLL translates DLL calls from Labview or others
  to the unit Board.c
  the drivers must have been installed before calling !


  for using the PCI Board, copy PCIB\board.c and .h to actual folder
	and make a rebuild all

	V1: 	all declarations use stdcall (WinAPI)
	V2.023: with block read functions
		- new function DLLFreeMemInfo
	V2.030 correct timer stop error in P202.2

	*/

#include <windows.h>
#include <tchar.h> // for FreeMem-Function
#include <stdlib.h> 
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <process.h>	// for Thread example
#include <malloc.h>		// msize

	// Make this data shared among all 
	// all applications that use this DLL.
	//....................................
#pragma data_seg( ".GLOBALS" )
int nProcessCount = 0;
int nThreadCount = 0;
//#pragma data_seg()
void	*dummy;

#include "GLOBAL.H"
#include "shared_src/board.c"
#include "shared_src/Direct2dViewer_c.h"
//extern volatile PUSHORT pDMABigBufBase[3];

volatile struct ffloopparams params, params2;

BOOL WINAPI DLLMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );
DllAccess int DLLGetProcessCount();
DllAccess int DLLGetThreadCount();
DllAccess  void DLLErrMsgBoxOn( void );	//BOARD.C sends error messages on default
DllAccess  void DLLErrMsgBoxOff( void );	//general deactivate of error message boxes
DllAccess UINT8 nDLLCCDDrvInit( void );		// init the driver -> true if found
DllAccess void DLLCCDDrvExit( UINT32 drvno );		// closes the driver
DllAccess UINT8 n2DLLInitBoard( UINT32 drv, UINT32 camcnt, UINT32 pixel, UINT32 flag816, UINT32 pclk, UINT32 xckdelay );		// init the driver -> true if found
DllAccess UINT8 DLLReadByteS0( UINT32 drvno, UINT8 *data, UINT32 PortOff );// read byte from Port, PortOff = Regs of Board
DllAccess UINT8 DLLWriteByteS0( UINT32 drv, UINT8 DataByte, UINT32 PortOff ); // writes DataByte to Port
DllAccess UINT8 DLLReadLongS0( UINT32 drvno, UINT32 * DWData, UINT32 PortOff );	// read long from Port, PortOff Regs of Board
DllAccess UINT8 DLLWriteLongS0( UINT32 drvno, UINT32 DWData, UINT32 PortOff ); // writes DataLong to Port
DllAccess UINT8 DLLReadLongDMA( UINT32 drvno, UINT32* pDWData, UINT32 PortOff );	// read long from Port, PortOff Regs of Board
DllAccess UINT8 DLLWriteLongDMA( UINT32 drvno, UINT32 DWData, UINT32 PortOff ); // writes DataLong to Port
DllAccess UINT8 DLLReadLongIOPort( UINT32 drvno, UINT32 * DWData, UINT32 PortOff ); // writes DataByte to Port
DllAccess UINT8 DLLWriteLongIOPort( UINT32 drvno, UINT32 DataL, UINT32 PortOff ); // writes DataByte to Port
DllAccess void DLLAboutDrv( UINT32 drvno );	// displays the version and board ID = test if board is there
DllAccess double DLLCalcRamUsageInMB( UINT32 nos, UINT32 nob );
DllAccess double DLLCalcMeasureTimeInSeconds( UINT32 nos, UINT32 nob, double exposure_time_in_ms );
//************	functions for managing controlbits in CtrlA register
DllAccess void DLLHighSlope( UINT32 drvno );		//set input Trigger slope high
DllAccess void DLLLowSlope( UINT32 drvno );		//set input Trigger slope low
DllAccess void DLLBothSlope( UINT32 drvno );	//trigger on each slope
DllAccess void DLLOutTrigHigh( UINT32 drvno );		//set output Trigger signal high
DllAccess void DLLOutTrigLow( UINT32 drvno );		//set output Trigger signal low
DllAccess void DLLOutTrigPulse( UINT32 drvno, UINT32 PulseWidth );	// pulses high output Trigger signal
DllAccess void DLLOpenShutter( UINT32 drvno );	// set IFC=high
DllAccess void DLLCloseShutter( UINT32 drvno );	// set IFC=low
//************ FIFO version functions
DllAccess void DLLSWTrig( UINT32 drvno );						//start a read to FIFO by software
DllAccess UINT8 DLLFFValid( UINT32 drvno );						// TRUE if linecounter>0
DllAccess void DLLSetExtTrig( UINT32 drvno );					// read to FIFO is triggered by external input I of PCI board
DllAccess void DLLSetIntTrig( UINT32 drvno );					// read to FIFO is triggered by Timer
DllAccess UINT8 DLLFFOvl( UINT32 drvno );					//TRUE if fifo overflow occured
DllAccess void DLLSetupVCLK( UINT32 drvno, UINT32 lines, UINT8 vfreq );//set the VCLK regs
DllAccess void DLLReadRingLine( pArrayT pdioden, UINT32 lno ); //read ring buffer line number lno 
DllAccess UINT8 DLLBlockTrig( UINT32 drv, UCHAR btrig_ch ); //read trigger input ->ch=1:pci in, ch=2:opto1, ch=3:opto2
//************ camera reads FIFO version
DllAccess UINT8 DLLSetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
DllAccess UINT8 DLLResetS0Bit( ULONG bitnumber, CHAR Address, UINT32 drvno );
//************ system timer
DllAccess UINT64 DLLTicksTimestamp( void );
DllAccess UINT32 DLLTickstous( UINT64 tks );
DllAccess void DLLSetupDMA( UINT32 drv, void*  pdioden, UINT32 nos, UINT32 blocks );
DllAccess void nDLLSetupDMA( UINT32 drv, UINT32 nos, UINT32 blocks );
DllAccess void DLLReturnFrame( UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16 *pdioden, UINT32 length );
DllAccess void nDLLReadFFLoop( UINT32 board_sel, UINT32 exptus, UINT8 exttrig, UINT8 blocktrigger, UINT8 btrig_ch );
DllAccess void DLLStopFFLoop( void );
DllAccess void DLLSetContFFLoop( UINT8 activate );
//************  Cooling
DllAccess void DLLSetTemp( UINT32 drvno, UINT8 level );
DllAccess void DLLSetEC( UINT32 drvno, UINT64 ecin100ns );
DllAccess void DLLResetEC( UINT32 drvno );
DllAccess void DLLSetTORReg( UINT32 drvno, UINT8 fkt );
DllAccess void DLLSetupDELAY( UINT32 drvno, UINT32 delay );
DllAccess void DLLSetISPDA( UINT32 drvno, UINT8 set );
DllAccess void DLLSetPDAnotFFT( UINT32 drvno, UINT8 set );
DllAccess void DLLSetISFFT( UINT32 drvno, UINT8 set );
DllAccess void DLLRsTOREG( UINT32 drvno );
DllAccess void DLLSetupHAModule( UINT8 irsingle, UINT32 fftlines );
DllAccess void DLLSetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, UINT8 keep);
DllAccess void DLLSetupROI(UINT32 drvno, UINT16 number_of_regions, UINT32 lines, UINT8 keep_first, UINT8* region_size);
DllAccess void DLLAboutS0( UINT32 drvno );
DllAccess void DLLSendFLCAM( UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data );
DllAccess void DLLSendFLCAM_DAC( UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature );
DllAccess void DLLDAC_setOutput( UINT32 drvno, UINT8 channel, UINT16 output ); //set output of DAC (PCB 2189-7)
DllAccess void DLLFreeMemInfo( UINT64 * pmemory_all, UINT64 * pmemory_free );
DllAccess void DLLErrorMsg( char ErrMsg[20] );
DllAccess void DLLCalcTrms( UINT32 drvno, UINT32 nos, ULONG TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms );
//************  2d greyscale viewer
DllAccess void DLLStart2dViewer( UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos );
DllAccess void DLLShowNewBitmap( UINT32 drvno, UINT16 cur_nob, UINT16 cam, UINT pixelAmount, UINT nos );
DllAccess void DLLDeinit2dViewer();
DllAccess void DLLSetGammaValue( UINT16 white, UINT16 black );
//************  GPX
DllAccess void DLLInitGPX( UINT32 drvno, UINT32 delay );
DllAccess void DLLAboutGPX( UINT32 drvno );
//************  Init CAM
DllAccess void DLLInitCamera3001( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT16 IS_FFT, UINT16 IS_AREA );
DllAccess void DLLInitCamera3010( UINT32 drvno, UINT16 pixel, UINT16 trigger_input, UINT8 adc_mode, UINT16 custom_pattern, UINT16 led_on, UINT16 gain_high );
DllAccess void DLLInitCamera3030( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 gain );
DllAccess void DLLBlockSyncStart( UINT32 drvno, UINT8 S1, UINT8 S2 );