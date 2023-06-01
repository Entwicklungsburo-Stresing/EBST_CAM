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
extern LVUserEventRef allBlocksDoneLVEvent;
#endif

#ifdef _DLL
#define DllAccess __declspec( dllexport )

#else
#define DllAccess __declspec( dllimport )
#endif

#ifdef __cplusplus
extern "C" {
#endif

BOOL WINAPI DLLMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

//************ High level API
// Basic operation of Stresing cameras:
// 1) Initialize the driver. Call it once at startup.
DllAccess es_status_codes DLLInitDriver(uint8_t* _number_of_boards);
// 2) Initialize PCIe board. Call it once at startup.
DllAccess es_status_codes DLLInitBoard();
// 3) Set settings parameter according to your camera system. Call it once at startup and every time you changed settings.
DllAccess es_status_codes DLLSetGlobalSettings(struct measurement_settings settings);
DllAccess es_status_codes DLLSetGlobalSettings_matlab(struct measurement_settings_matlab measurement_s, struct camera_settings camera_s0, struct camera_settings camera_s1, struct camera_settings camera_s2, struct camera_settings camera_s3, struct camera_settings camera_s4);
// 4) Initialize Hardware and Software for the Measurement. Call it once at startup and every time you changed settings.
DllAccess es_status_codes DLLInitMeasurement();
// 5) Start the measurement. Call it every time you want to measure.
DllAccess es_status_codes DLLStartMeasurement_blocking();
DllAccess void DLLStartMeasurement_nonblocking();
// 5b) Use this call, if you want to abort the measurement.
DllAccess es_status_codes DLLAbortMeasurement();
// 6) Get the data with one of the following 3 calls. Call it how many times you want.
DllAccess es_status_codes DLLReturnFrame(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest, uint32_t pixel);
DllAccess es_status_codes DLLCopyAllData(uint32_t drvno, uint16_t* pdest);
DllAccess es_status_codes DLLCopyOneBlock(uint32_t drvno, uint16_t block, uint16_t* pdest);
// 7) Before exiting your software, use this call for cleanup.
DllAccess es_status_codes DLLExitDriver();

//************ Mid level API
//************ system info & control
DllAccess void DLLFreeMemInfo(uint64_t * pmemory_all, uint64_t * pmemory_free);
DllAccess int DLLGetProcessCount();
DllAccess int DLLGetThreadCount();
DllAccess double DLLCalcRamUsageInMB(uint32_t nos, uint32_t nob);
DllAccess double DLLCalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms);
DllAccess void DLLSetContinuousMeasurement(uint8_t on);
#ifdef COMPILE_FOR_LABVIEW
DllAccess void DLLRegisterLVEvents(LVUserEventRef *measureStartEvent, LVUserEventRef *measureDoneEvent, LVUserEventRef *blockStartEvent, LVUserEventRef *blockDoneEvent, LVUserEventRef* allBlocksDoneEvent);
#endif
DllAccess char* DLLConvertErrorCodeToMsg( es_status_codes status );
DllAccess void DLLFillUserBufferWithDummyData(uint32_t board_sel);
//************ Cam infos
DllAccess es_status_codes DLLwaitForMeasureReady(uint32_t board_sel);
DllAccess es_status_codes DLLwaitForBlockReady(uint32_t board_sel);
DllAccess es_status_codes DLLisMeasureOn(uint32_t board_sel, uint8_t* measureOn0, uint8_t* measureOn1);
DllAccess es_status_codes DLLisBlockOn(uint32_t board_sel, uint8_t* blockOn0, uint8_t* blockOn1);
DllAccess void DLLGetCurrentScanNumber(uint32_t board_sel, int64_t* sample, int64_t* block);
//************  Control CAM
DllAccess es_status_codes DLLOutTrigHigh(uint32_t board_sel);
DllAccess es_status_codes DLLOutTrigLow(uint32_t board_sel);
DllAccess es_status_codes DLLOutTrigPulse(uint32_t board_sel, uint32_t PulseWidth);
DllAccess es_status_codes DLLOpenShutter(uint32_t board_sel);
DllAccess es_status_codes DLLCloseShutter(uint32_t board_sel);
DllAccess es_status_codes DLLSetTemp(uint32_t board_sel, uint8_t level);
DllAccess es_status_codes DLLSetTORReg(uint32_t board_sel, uint8_t tor);
DllAccess es_status_codes DLLDAC8568_setAllOutputs(uint32_t board_sel, uint8_t location, uint8_t cameraPosition, uint32_t* output0, uint32_t* output1, uint32_t* output2, uint32_t* output3, uint32_t* output4, uint8_t reorder_channel);
DllAccess es_status_codes DLLIOCtrl_setAllOutputs(uint32_t board_sel, uint32_t* width_in_5ns, uint32_t* delay_in_5ns);
DllAccess es_status_codes DLLIOCtrl_setT0(uint32_t board_sel, uint32_t period_in_10ns);
DllAccess es_status_codes DLLGetIsTdc(uint32_t board_sel, uint8_t* isTdc0, uint8_t* isTdc1, uint8_t* isTdc2, uint8_t* isTdc3, uint8_t* isTdc4);
DllAccess es_status_codes DLLGetIsDsc(uint32_t board_sel, uint8_t* isDsc0, uint8_t* isDsc1, uint8_t* isDsc2, uint8_t* isDsc3, uint8_t* isDsc4);
DllAccess es_status_codes DLLResetDSC(uint32_t board_sel, uint8_t DSCNumber);
DllAccess es_status_codes DLLSetDIRDSC(uint32_t board_sel, uint8_t DSCNumber, uint8_t dir);
DllAccess es_status_codes DLLGetDSC(uint32_t board_sel, uint8_t DSCNumber, uint32_t* ADSC0, uint32_t* LDSC0, uint32_t* ADSC1, uint32_t* LDSC1);
DllAccess es_status_codes DLLInitGPX(uint32_t board_sel, uint32_t delay);
DllAccess es_status_codes DLLGetAllSpecialPixelInformation(uint32_t board_sel, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp);
//************ read and write functions
DllAccess es_status_codes DLLreadRegisterS0_8(uint32_t board_sel, uint8_t* data, uint32_t address);
DllAccess es_status_codes DLLwriteRegisterS0_8(uint32_t board_sel, uint8_t data, uint32_t address);
DllAccess es_status_codes DLLreadRegisterS0_32(uint32_t board_sel, uint32_t* data, uint32_t address);
DllAccess es_status_codes DLLwriteRegisterS0_32(uint32_t board_sel, uint32_t data, uint32_t address);
DllAccess es_status_codes DLLsetBitS0_32(uint32_t board_sel, uint32_t bitnumber, uint16_t address);
DllAccess es_status_codes DLLresetBitS0_32(uint32_t board_sel, uint32_t bitnumber, uint16_t address);
#ifndef MINIMAL_BUILD
DllAccess es_status_codes DLLCalcTrms(uint32_t board_sel, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double* mwf0, double* trms0, double* mwf1, double* trms1);
DllAccess void DLLErrMsgBoxOn();
DllAccess void DLLErrMsgBoxOff();
DllAccess es_status_codes DLLAbout(uint32_t board_sel);
DllAccess void DLLErrorMsg(char ErrMsg[20]);
//************  2d greyscale viewer
DllAccess void DLLStart2dViewer(UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos);
DllAccess void DLLShowNewBitmap(UINT32 drvno, UINT32 cur_nob, UINT16 cam, UINT16 pixel, UINT32 nos);
DllAccess void DLLDeinit2dViewer();
DllAccess void DLLSetGammaValue(UINT16 white, UINT16 black);
DllAccess UINT16 DLLGetGammaWhite();
DllAccess UINT16 DLLGetGammaBlack();
#endif

#ifdef __cplusplus
}
#endif
