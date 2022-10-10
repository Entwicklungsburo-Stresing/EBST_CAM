#pragma once
/*   **********************************************
	DLL for CCD Camera driver of
	for linking to labview

  Entwicklungsbuero Stresing
  Germany

  this DLL translates DLL calls from Labview or others
  to the unit Board.c
  the drivers must have been installed before calling !

  for using the PCI Board, copy PCIB\board.c and .h to actual folder
	and make a rebuild all
*/

#include <windows.h>
// COMPILE_FOR_LABVIEW is defined in the preprocessor definitions of the project ESLSCDLL when Debug-Labview or Release-Labview is chosen as configuration
#ifdef COMPILE_FOR_LABVIEW
#include "LabVIEW 2015/cintools/extcode.h"
#endif
#include "shared_src/Board.h"

#ifdef COMPILE_FOR_LABVIEW
extern LVUserEventRef measureStartLVEvent;
extern LVUserEventRef measureDoneLVEvent;
extern LVUserEventRef blockStartLVEvent;
extern LVUserEventRef blockDoneLVEvent;
#endif

#ifdef _DLL
#define DllAccess __declspec( dllexport )

#else
#define DllAccess __declspec( dllimport )
#endif

BOOL WINAPI DLLMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

//************ High level API
DllAccess es_status_codes DLLInitBoard();
DllAccess es_status_codes DLLCCDDrvInit(UINT8* _number_of_boards);		// init the driver -> true if found
DllAccess es_status_codes DLLCCDDrvExit(UINT32 boad_sel);		// closes the driver
DllAccess es_status_codes DLLSetGlobalSettings(struct global_settings settings);
DllAccess es_status_codes DLLAbortMeasurement(UINT32 drv);
DllAccess es_status_codes DLLReturnFrame(UINT32 drv, UINT32 curr_nos, UINT32 curr_nob, UINT16 curr_cam, UINT16 *pdioden, UINT32 length);
DllAccess es_status_codes DLLCopyAllData(UINT32 drv, UINT16 *pdioden);
DllAccess es_status_codes DLLCopyOneBlock(UINT32 drv, UINT16 block, UINT16 *pdest);
DllAccess es_status_codes DLLInitMeasurement();
DllAccess void DLLReadFFLoop();

//************ Mid level API
//************ system info & control
DllAccess UINT64 DLLTicksTimestamp( void );
DllAccess UINT32 DLLTickstous( UINT64 tks );
DllAccess void DLLFreeMemInfo(UINT64 * pmemory_all, UINT64 * pmemory_free);
DllAccess void DLLErrorMsg(char ErrMsg[20]);
DllAccess es_status_codes DLLCalcTrms(UINT32 drvno, UINT32 firstSample, UINT32 lastSample, UINT32 TRMS_pixel, UINT16 CAMpos, double *mwf, double *trms);
DllAccess int DLLGetProcessCount();
DllAccess int DLLGetThreadCount();
DllAccess void DLLErrMsgBoxOn();	//BOARD.C sends error messages on default
DllAccess void DLLErrMsgBoxOff();	//general deactivate of error message boxes
DllAccess double DLLCalcRamUsageInMB(UINT32 nos, UINT32 nob);
DllAccess double DLLCalcMeasureTimeInSeconds(UINT32 nos, UINT32 nob, double exposure_time_in_ms);
DllAccess void DLLInitProDLL();
#ifdef COMPILE_FOR_LABVIEW
DllAccess void DLLRegisterLVEvents(LVUserEventRef *measureStartEvent, LVUserEventRef *measureDoneEvent, LVUserEventRef *blockStartEvent, LVUserEventRef *blockDoneEvent);
DllAccess CStr DLLConvertErrorCodeToMsg(es_status_codes status);
#endif
//************ Cam infos
DllAccess es_status_codes DLLAbout(UINT32 drvno);
DllAccess es_status_codes DLLAboutS0( UINT32 drvno );
DllAccess es_status_codes DLLFFValid(UINT32 drvno, UINT8* valid);						// TRUE if linecounter>0
DllAccess es_status_codes DLLreadBlockTriggerState(UINT32 drv, UCHAR btrig_ch, UINT8* state); //read trigger input ->ch=1:pci in, ch=2:opto1, ch=3:opto2
DllAccess es_status_codes DLLAboutGPX(UINT32 drvno);
DllAccess es_status_codes DLLwaitForMeasureReady(UINT32 drvno);
DllAccess es_status_codes DLLwaitForBlockReady(UINT32 drvno);
DllAccess es_status_codes DLLisMeasureOn(UINT32 drvno, UINT8* measureOn);
DllAccess es_status_codes DLLisBlockOn(UINT32 drvno, UINT8* blockOn);
DllAccess void DLLGetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block);
//************  Control CAM
DllAccess void setSWTrig(UINT8 on);
DllAccess void DLLSetContFFLoop(UINT8 activate, UINT32 pause_in_microseconds);
DllAccess es_status_codes DLLSWTrig(UINT32 drvno);	//start a read to FIFO by software
DllAccess es_status_codes DLLOutTrigHigh(UINT32 drvno);	//set output Trigger signal high
DllAccess es_status_codes DLLOutTrigLow(UINT32 drvno);	//set output Trigger signal low
DllAccess es_status_codes DLLOutTrigPulse(UINT32 drvno, UINT32 PulseWidth);	// pulses high output Trigger signal
DllAccess es_status_codes DLLOpenShutter(UINT32 drvno);	// set IFC=high
DllAccess es_status_codes DLLCloseShutter(UINT32 drvno);	// set IFC=low
DllAccess es_status_codes DLLLedOff(UINT32 drvno, UINT8 LED_OFF);
DllAccess es_status_codes DLLsetUseEC(UINT32 drvno, UINT8 use_EC);
DllAccess es_status_codes DLLInitCameraGeneral( UINT32 drvno, UINT16 pixel, UINT16 cc_trigger_input, UINT8 is_fft, UINT8 is_area, UINT8 IS_COOLED, UINT16 led_off, UINT16 sensor_gain, UINT16 use_EC);
DllAccess es_status_codes DLLInitCamera3001( UINT32 drvno );
DllAccess es_status_codes DLLInitCamera3010( UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern);
DllAccess es_status_codes DLLInitCamera3030(UINT32 drvno, UINT8 adc_mode, UINT16 custom_pattern, UINT8 adc_gain, UINT8 useDac, UINT32* dac_output, UINT8 isIr);
DllAccess es_status_codes DLLSetupFullBinning( UINT32 drvno, UINT32 lines, UINT8 vfreq );
DllAccess es_status_codes DLLSetBTI( UINT32 drvno, UINT8 bti_mode );
DllAccess es_status_codes DLLSetSTI( UINT32 drvno, UINT8 sti_mode );
DllAccess es_status_codes DLLSetSTimer( UINT32 drvno, UINT32 stime_in_microseconds );
DllAccess es_status_codes DLLSetBTimer( UINT32 drvno, UINT32 btime_in_microseconds );
DllAccess es_status_codes DLLSetSSlope(UINT32 drvno, UINT32 sslope);
DllAccess es_status_codes DLLSetBSlope( UINT32 drvno, UINT32 slope );
DllAccess es_status_codes DLLSetGain( UINT32 drvno, UINT16 gain_value );
DllAccess es_status_codes DLLClearAllUserRegs(UINT32 drvno);
DllAccess es_status_codes DLLSetupPCIE_DMA(UINT32 drvno);
DllAccess es_status_codes DLLSetTLPS(UINT32 drvno, UINT32 pixel);
DllAccess es_status_codes DLLSetSDAT(UINT32 drvno, UINT32 datin10ns);
DllAccess es_status_codes DLLSetBDAT(UINT32 drvno, UINT32 datin10ns);
DllAccess es_status_codes DLLSetTemp(UINT32 drvno, UINT8 level);
DllAccess es_status_codes DLLSetSEC(UINT32 drvno, UINT32 ecin10ns);
DllAccess es_status_codes DLLSetBEC(UINT32 drvno, UINT32 ecin10ns);
DllAccess es_status_codes DLLSetTORReg(UINT32 drvno, UINT8 tor);
DllAccess es_status_codes DLLSetSensorType(UINT32 drvno, UINT8 set);
DllAccess es_status_codes DLLSetupVPB(UINT32 drvno, UINT32 range, UINT32 lines, UINT8 keep);
DllAccess es_status_codes DLLDAC_setOutput(UINT32 drvno, UINT8 channel, UINT16 output); //set output of DAC (PCB 2189-7)
DllAccess es_status_codes DLLDAC_setAllOutputs(UINT32 drvno, UINT32* output, UINT8 isIR); //set output of DAC (PCB 2189-7)
DllAccess es_status_codes DLLSetMeasurementParameters(UINT32 drvno, UINT32 nos, UINT32 nob);
DllAccess es_status_codes DLLSetupVCLK(UINT32 drvno, UINT32 lines, UINT8 vfreq);//set the VCLK regs
DllAccess es_status_codes DLLInitGPX(UINT32 drvno, UINT32 delay);
DllAccess es_status_codes DLLIOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns);
DllAccess es_status_codes DLLIOCtrl_setAllOutputs(uint32_t drvno, uint16_t* width_in_5ns, uint16_t* delay_in_5ns);
DllAccess es_status_codes DLLIOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns);
DllAccess es_status_codes DLLSetTicnt(uint32_t drvno, uint8_t divider);
DllAccess es_status_codes DLLSetTocnt(uint32_t drvno, uint8_t divider);
DllAccess es_status_codes DLLGetIsTdc(UINT32 drvno, UINT8* isTdc);
DllAccess es_status_codes DLLGetIsDsc(UINT32 drvno, UINT8* isDsc);
//************ read and write functions
DllAccess es_status_codes DLLReadByteS0(UINT32 drvno, UINT8 *data, UINT32 address);
DllAccess es_status_codes DLLWriteByteS0(UINT32 drvno, UINT8 data, UINT32 address);
DllAccess es_status_codes DLLReadLongS0(UINT32 drvno, UINT32 * data, UINT32 address);
DllAccess es_status_codes DLLWriteLongS0(UINT32 drvno, UINT32 data, UINT32 address);
DllAccess es_status_codes DLLReadLongDMA(UINT32 drvno, UINT32* data, UINT32 address);
DllAccess es_status_codes DLLWriteLongDMA(UINT32 drvno, UINT32 data, UINT32 address);
DllAccess es_status_codes DLLReadLongIOPort(UINT32 drvno, UINT32 * data, UINT32 address);
DllAccess es_status_codes DLLWriteLongIOPort(UINT32 drvno, UINT32 data, UINT32 address);
DllAccess es_status_codes DLLSetS0Bit(ULONG bitnumber, CHAR address, UINT32 drvno);
DllAccess es_status_codes DLLResetS0Bit(ULONG bitnumber, CHAR address, UINT32 drvno);
DllAccess es_status_codes DLLSendFLCAM(UINT32 drvno, UINT8 maddr, UINT8 adaddr, UINT16 data);
DllAccess es_status_codes DLLSendFLCAM_DAC(UINT32 drvno, UINT8 ctrl, UINT8 addr, UINT16 data, UINT8 feature);
