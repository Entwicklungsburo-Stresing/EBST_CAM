#pragma once
/*   **********************************************
	DLL for CCD Camera driver of

	Entwicklungsbuero Stresing
	Germany

	this DLL translates DLL calls from Labview or others
	to the unit Board.c
	the drivers must have been installed before calling!
*/

#ifdef WIN32
#include <windows.h>
#endif

// COMPILE_FOR_LABVIEW is defined in the preprocessor definitions of the project ESLSCDLL when Debug-Labview or Release-Labview is chosen as configuration
#ifdef COMPILE_FOR_LABVIEW
#include "LabVIEW 2015/cintools/extcode.h"
#endif
#include "Board.h"

#ifdef COMPILE_FOR_LABVIEW
extern LVUserEventRef measureStartLVEvent;
extern LVUserEventRef measureDoneLVEvent;
extern LVUserEventRef blockStartLVEvent;
extern LVUserEventRef blockDoneLVEvent;
extern LVUserEventRef allBlocksDoneLVEvent;
#endif


#ifdef WIN32
#define DllAccess __declspec( dllexport )

#else
#define DllAccess
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
BOOL WINAPI DLLMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
#endif

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
// 6) Get the data with one of the following calls. Call it how many times you want.
DllAccess es_status_codes DLLCopyOneSample(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest);
DllAccess es_status_codes DLLCopyOneSample_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest0, uint16_t* pdest1, uint16_t* pdest2, uint16_t* pdest3, uint16_t* pdest4);
DllAccess es_status_codes DLLCopyOneBlock(uint32_t drvno, uint16_t block, uint16_t* pdest);
DllAccess es_status_codes DLLCopyOneBlock_multipleBoards(uint16_t block, uint16_t* pdest0, uint16_t* pdest1, uint16_t* pdest2, uint16_t* pdest3, uint16_t* pdest4);
DllAccess es_status_codes DLLCopyAllData(uint32_t drvno, uint16_t* pdest);
DllAccess es_status_codes DLLCopyAllData_multipleBoards(uint16_t* pdest0, uint16_t* pdest1, uint16_t* pdest2, uint16_t* pdest3, uint16_t* pdest4);
DllAccess es_status_codes DLLCopyDataArbitrary(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t pixel, uint32_t length_in_pixel, uint16_t* pdest);
DllAccess es_status_codes DLLGetOneSamplePointer(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
DllAccess es_status_codes DLLGetOneBlockPointer(uint32_t drvno, uint32_t block, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
DllAccess es_status_codes DLLGetAllDataPointer(uint32_t drvno, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
DllAccess es_status_codes DLLGetPixelPointer(uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
// 7) Before exiting your software, use this call for cleanup.
DllAccess es_status_codes DLLExitDriver();

//************ Mid level API
//************ system info & control
DllAccess void DLLFreeMemInfo(uint64_t* pmemory_all, uint64_t* pmemory_free);
DllAccess int DLLGetProcessCount();
DllAccess int DLLGetThreadCount();
DllAccess double DLLCalcRamUsageInMB(uint32_t nos, uint32_t nob);
DllAccess double DLLCalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms);
DllAccess void DLLSetContinuousMeasurement(uint8_t on);
#ifdef COMPILE_FOR_LABVIEW
DllAccess void DLLRegisterLVEvents(LVUserEventRef* measureStartEvent, LVUserEventRef* measureDoneEvent, LVUserEventRef* blockStartEvent, LVUserEventRef* blockDoneEvent, LVUserEventRef* allBlocksDoneEvent);
#endif
DllAccess char* DLLConvertErrorCodeToMsg(es_status_codes status);
DllAccess void DLLFillUserBufferWithDummyData();
//************ Cam infos
DllAccess es_status_codes DLLwaitForMeasureReady();
DllAccess es_status_codes DLLwaitForBlockReady();
DllAccess es_status_codes DLLisMeasureOn(uint32_t drvno, uint8_t* measureOn);
DllAccess es_status_codes DLLisMeasureOn_multipleBoards(uint8_t* measureOn0, uint8_t* measureOn1, uint8_t* measureOn2, uint8_t* measureOn3, uint8_t* measureOn4);
DllAccess es_status_codes DLLisBlockOn(uint32_t drvno, uint8_t* blockOn);
DllAccess es_status_codes DLLisBlockOn_multipleBoards(uint8_t* blockOn0, uint8_t* blockOn1, uint8_t* blockOn2, uint8_t* blockOn3, uint8_t* blockOn4);
DllAccess void DLLGetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block);
DllAccess void DLLGetCurrentScanNumber_multipleBoards(int64_t* sample0, int64_t* block0, int64_t* sample1, int64_t* block1, int64_t* sample2, int64_t* block2, int64_t* sample3, int64_t* block3, int64_t* sample4, int64_t* block4);
DllAccess es_status_codes DLLReadScanFrequencyBit(uint32_t drvno, uint8_t* scanFrequencyTooHigh);
DllAccess es_status_codes DLLReadScanFrequencyBit_multipleBoards(uint8_t* scanFrequencyTooHigh0, uint8_t* scanFrequencyTooHigh1, uint8_t* scanFrequencyTooHigh2, uint8_t* scanFrequencyTooHigh3, uint8_t* scanFrequencyTooHigh4);
DllAccess es_status_codes DLLResetScanFrequencyBit(uint32_t drvno);
DllAccess es_status_codes DLLResetScanFrequencyBit_multipleBoards();
DllAccess es_status_codes DLLReadBlockFrequencyBit(uint32_t drvno, uint8_t* blockFrequencyTooHigh);
DllAccess es_status_codes DLLReadBlockFrequencyBit_multipleBoards(uint8_t* blockFrequencyTooHigh0, uint8_t* blockFrequencyTooHigh1, uint8_t* blockFrequencyTooHigh2, uint8_t* blockFrequencyTooHigh3, uint8_t* blockFrequencyTooHigh4);
DllAccess es_status_codes DLLResetBlockFrequencyBit(uint32_t drvno);
DllAccess es_status_codes DLLResetBlockFrequencyBit_multipleBoards();
DllAccess es_status_codes DLLGetCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* overTemp);
DllAccess es_status_codes DLLGetCameraStatusOverTemp_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* overTemp1, uint8_t* overTemp2, uint8_t* overTemp3, uint8_t* overTemp4, uint8_t* overTemp5);
DllAccess es_status_codes DLLGetCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* tempGood);
DllAccess es_status_codes DLLGetCameraStatusTempGood_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera_pos, uint8_t* tempGood1, uint8_t* tempGood2, uint8_t* tempGood3, uint8_t* tempGood4, uint8_t* tempGood5);
DllAccess es_status_codes DLLFindCam(uint32_t drvno);
DllAccess es_status_codes DLLFindCam_multipleBoards(uint8_t* cameraFound0, uint8_t* cameraFound1, uint8_t* cameraFound2, uint8_t* cameraFound3, uint8_t* cameraFound4);
DllAccess es_status_codes DLLGetBlockOn(uint32_t drvno, uint8_t* blockOn);
DllAccess es_status_codes DLLGetBlockOn_multipleBoards(uint8_t* blockOn0, uint8_t* blockOn1, uint8_t* blockOn2, uint8_t* blockOn3, uint8_t* blockOn4);
DllAccess es_status_codes DLLDumpS0Registers(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLDumpHumanReadableS0Registers(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLDumpDmaRegisters(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLDumpTlpRegisters(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLDumpMeasurementSettings(char** stringPtr);
DllAccess es_status_codes DLLDumpCameraSettings(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLDumpPciRegisters(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLAboutDrv(uint32_t drvno, char** stringPtr);
DllAccess es_status_codes DLLAboutGPX(uint32_t drvno, char** stringPtr);
DllAccess void DLLGetVerifiedDataDialog(struct verify_data_parameter* vd, char** resultString);
DllAccess uint8_t DLLGetIsRunning();
DllAccess es_status_codes DLLGetBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex);
DllAccess es_status_codes DLLGetScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex);
DllAccess es_status_codes DLLGetS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state);
DllAccess es_status_codes DLLGetS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state);
DllAccess es_status_codes DLLGetImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal);
DllAccess es_status_codes DLLGetImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal);
DllAccess uint32_t DLLGetVirtualCamcnt(uint32_t drvno);
DllAccess bool DLLGetTestModeOn();
//************  Control CAM
DllAccess es_status_codes DLLOutTrigHigh();
DllAccess es_status_codes DLLOutTrigLow();
DllAccess es_status_codes DLLOutTrigPulse(int64_t PulseWidth);
DllAccess es_status_codes DLLOpenShutter();
DllAccess es_status_codes DLLCloseShutter();
DllAccess es_status_codes DLLSetTemp(uint8_t level);
DllAccess es_status_codes DLLSetTORReg(uint32_t drvno, uint8_t tor);
DllAccess es_status_codes DLLSetTORReg_multipleBoards(uint8_t tor);
DllAccess es_status_codes DLLDAC8568_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, uint8_t reorder_channels);
DllAccess es_status_codes DLLDAC8568_setAllOutputs_multipleBoards(uint8_t location, uint8_t cameraPosition, uint32_t* output0, uint32_t* output1, uint32_t* output2, uint32_t* output3, uint32_t* output4, uint8_t reorder_channels);
DllAccess es_status_codes DLLIOCtrl_setAllOutputs(uint32_t* width_in_5ns, uint32_t* delay_in_5ns);
DllAccess es_status_codes DLLIOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns);
DllAccess es_status_codes DLLIOCtrl_setT0_multipleBoards(uint32_t period_in_10ns);
DllAccess es_status_codes DLLIOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns);
DllAccess es_status_codes DLLGetIsTdc(uint32_t drvno, uint8_t* isTdc);
DllAccess es_status_codes DLLGetIsTdc_multipleBoards(uint8_t* isTdc0, uint8_t* isTdc1, uint8_t* isTdc2, uint8_t* isTdc3, uint8_t* isTdc4);
DllAccess es_status_codes DLLGetIsDsc(uint32_t drvno, uint8_t* isDsc);
DllAccess es_status_codes DLLGetIsDsc_multipleBoards(uint8_t* isDsc0, uint8_t* isDsc1, uint8_t* isDsc2, uint8_t* isDsc3, uint8_t* isDsc4);
DllAccess es_status_codes DLLResetDSC(uint32_t drvno, uint8_t DSCNumber);
DllAccess es_status_codes DLLResetDSC_multipleBoards(uint8_t DSCNumber);
DllAccess es_status_codes DLLSetDIRDSC(uint32_t drvno, uint8_t DSCNumber, uint8_t dir);
DllAccess es_status_codes DLLSetDIRDSC_multipleBoards(uint8_t DSCNumber, uint8_t dir);
DllAccess es_status_codes DLLGetDSC(uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC);
DllAccess es_status_codes DLLGetDSC_multipleBoards(uint8_t DSCNumber, uint32_t* ADSC0, uint32_t* LDSC0, uint32_t* ADSC1, uint32_t* LDSC1, uint32_t* ADSC2, uint32_t* LDSC2, uint32_t* ADSC3, uint32_t* LDSC3, uint32_t* ADSC4, uint32_t* LDSC4);
DllAccess es_status_codes DLLInitGPX(uint32_t delay);
DllAccess es_status_codes DLLGetAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp);
DllAccess es_status_codes DLLGetAllSpecialPixelInformation_multipleBoards(uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp0, struct special_pixels* sp1, struct special_pixels* sp2, struct special_pixels* sp3, struct special_pixels* sp4);
DllAccess es_status_codes DLLSetSTimer(uint32_t drvno, uint32_t stime_in_microseconds);
DllAccess es_status_codes DLLSetBTimer(uint32_t drvno, uint32_t btime_in_microseconds);
DllAccess es_status_codes DLLGetXckLength(uint32_t drvno, uint32_t* xckLengthIn10ns);
DllAccess es_status_codes DLLGetXckPeriod(uint32_t drvno, uint32_t* xckPeriodIn10ns);
DllAccess es_status_codes DLLGetBonLength(uint32_t drvno, uint32_t* bonLengthIn10ns);
DllAccess es_status_codes DLLGetBonPeriod(uint32_t drvno, uint32_t* bonPeriodIn10ns);
DllAccess es_status_codes DLLGetXckLength_multipleBoards(uint32_t* xckLengthIn10ns0, uint32_t* xckLengthIn10ns1, uint32_t* xckLengthIn10ns2, uint32_t* xckLengthIn10ns3, uint32_t* xckLengthIn10ns4);
DllAccess es_status_codes DLLGetXckPeriod_multipleBoards(uint32_t* xckPeriodIn10ns0, uint32_t* xckPeriodIn10ns1, uint32_t* xckPeriodIn10ns2, uint32_t* xckPeriodIn10ns3, uint32_t* xckPeriodIn10ns4);
DllAccess es_status_codes DLLGetBonLength_multipleBoards(uint32_t* bonLengthIn10ns0, uint32_t* bonLengthIn10ns1, uint32_t* bonLengthIn10ns2, uint32_t* bonLengthIn10ns3, uint32_t* bonLengthIn10ns4);
DllAccess es_status_codes DLLGetBonPeriod_multipleBoards(uint32_t* bonPeriodIn10ns0, uint32_t* bonPeriodIn10ns1, uint32_t* bonPeriodIn10ns2, uint32_t* bonPeriodIn10ns3, uint32_t* bonPeriodIn10ns4);
DllAccess es_status_codes DLLGetScanTriggerDetected(uint32_t drvno, uint8_t* detected);
DllAccess es_status_codes DLLGetBlockTriggerDetected(uint32_t drvno, uint8_t* detected);
DllAccess es_status_codes DLLResetScanTriggerDetected(uint32_t drvno);
DllAccess es_status_codes DLLResetBlockTriggerDetected(uint32_t drvno);
DllAccess es_status_codes DLLGetScanTriggerDetected_multipleBoards(uint8_t* detected0, uint8_t* detected1, uint8_t* detected2, uint8_t* detected3, uint8_t* detected4);
DllAccess es_status_codes DLLGetBlockTriggerDetected_multipleBoards(uint8_t* detected0, uint8_t* detected1, uint8_t* detected2, uint8_t* detected3, uint8_t* detected4);
DllAccess es_status_codes DLLResetScanTriggerDetected_multipleBoards();
DllAccess es_status_codes DLLResetBlockTriggerDetected_multipleBoards();
DllAccess es_status_codes DLLDAC8568_setOutput(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output);
DllAccess es_status_codes DLLCheckFifoValid(uint32_t drvno, bool* valid);
DllAccess es_status_codes DLLCheckFifoOverflow(uint32_t drvno, bool* overflow);
DllAccess es_status_codes DLLCheckFifoEmpty(uint32_t drvno, bool* empty);
DllAccess es_status_codes DLLCheckFifoFull(uint32_t drvno, bool* full);
DllAccess void DLLSetMeasureStartHook(void(*hook)());
DllAccess void DLLSetMeasureDoneHook(void(*hook)());
DllAccess void DLLSetBlockStartHook(void(*hook)());
DllAccess void DLLSetBlockDoneHook(void(*hook)());
DllAccess void DLLSetAllBlocksDoneHook(void(*hook)());
//************ read and write functions
DllAccess es_status_codes DLLreadRegisterS0_8(uint32_t drvno, uint8_t* data, uint32_t address);
DllAccess es_status_codes DLLreadRegisterS0_8_multipleBoards(uint8_t* data0, uint8_t* data1, uint8_t* data2, uint8_t* data3, uint8_t* data4, uint32_t address);
DllAccess es_status_codes DLLwriteRegisterS0_8(uint8_t data, uint32_t address);
DllAccess es_status_codes DLLreadRegisterS0_32(uint32_t drvno, uint32_t* data, uint32_t address);
DllAccess es_status_codes DLLreadRegisterS0_32_multipleBoards(uint32_t* data0, uint32_t* data1, uint32_t* data2, uint32_t* data3, uint32_t* data4, uint32_t address);
DllAccess es_status_codes DLLwriteRegisterS0_32(uint32_t data, uint32_t address);
DllAccess es_status_codes DLLsetBitS0_32(uint32_t bitnumber, uint16_t address);
DllAccess es_status_codes DLLresetBitS0_32(uint32_t bitnumber, uint16_t address);
DllAccess es_status_codes DLLReadBitS0_32(uint32_t drvno, uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh);
DllAccess es_status_codes DLLReadBitS0_32_multipleBoards(uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh0, uint8_t* isBitHigh1, uint8_t* isBitHigh2, uint8_t* isBitHigh3, uint8_t* isBitHigh4);
DllAccess es_status_codes DLLReadBitS0_8(uint32_t drvno, uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh);
DllAccess es_status_codes DLLReadBitS0_8_multipleBoards(uint16_t address, uint8_t bitnumber, uint8_t* isBitHigh0, uint8_t* isBitHigh1, uint8_t* isBitHigh2, uint8_t* isBitHigh3, uint8_t* isBitHigh4);
#ifndef MINIMAL_BUILD
DllAccess es_status_codes DLLCalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double* mwf, double* trms);
DllAccess es_status_codes DLLCalcTrms_multipleBoards(uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double* mwf0, double* trms0, double* mwf1, double* trms1, double* mwf2, double* trms2, double* mwf3, double* trms3, double* mwf4, double* trms4);
DllAccess void DLLErrMsgBoxOn();
DllAccess void DLLErrMsgBoxOff();
DllAccess es_status_codes DLLAbout();
DllAccess void DLLErrorMsg(char ErrMsg[20]);
//************  2d greyscale viewer
DllAccess void DLLStart2dViewer(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos);
DllAccess void DLLShowNewBitmap(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos);
DllAccess void DLLDeinit2dViewer();
DllAccess void DLLSetGammaValue(uint16_t white, uint16_t black);
DllAccess uint16_t DLLGetGammaWhite();
DllAccess uint16_t DLLGetGammaBlack();
DllAccess es_status_codes DLLExportMeasurementHDF5(const char* path, char* filename);
#endif

#ifdef __cplusplus
}
#endif
