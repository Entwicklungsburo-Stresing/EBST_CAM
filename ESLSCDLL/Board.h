/*****************************************************************//**
 * @file   Board.h
 * @brief  All functions for interacting with the Stresing PCIe board.
 * 
 * All functions are written in a plattform independent way. All stuff that needs to be done in a plattform dependent way is done in Board_ll.h.
 * @author Gerhard Stresing, Bastian Brabec, Florian Hahn
 * @date   before February 2016
 *********************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Board_ll.h"
#ifdef WIN32
#include "hdf5.h"
#endif
#include "Camera.h"

// High level API
// platform independent implementation
// Basic operation of Stresing cameras:
// 1) Initialize the driver. Call it once at startup.
es_status_codes InitDriver();
// 2) Initialize PCIe board. Call it once at startup.
es_status_codes InitBoard();
// 3) Set settings parameter according to your camera system. Call it once at startup and every time you changed settings.
void SetGlobalSettings(struct measurement_settings settings);
// 4) Initialize Hardware and Software for the Measurement. Call it once at startup and every time you changed settings.
es_status_codes InitMeasurement();
// 5) Start the measurement. Call it every time you want to measure.
es_status_codes StartMeasurement();
// 5b) Use this call, if you want to abort the measurement.
es_status_codes AbortMeasurement();
// 6) Get the data.
es_status_codes CopyOneSample(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest);
es_status_codes CopyOneBlock(uint32_t drvno, uint16_t block, uint16_t* pdest);
es_status_codes CopyAllData(uint32_t drvno, uint16_t* pdest);
es_status_codes CopyDataArbitrary(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t pixel, size_t length_in_pixel, uint16_t* pdest);
es_status_codes CopyOneBlockOfOneCamera(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t* pdest);
es_status_codes GetOneSamplePointer(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
es_status_codes GetOneBlockPointer(uint32_t drvno, uint32_t block, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
es_status_codes GetAllDataPointer(uint32_t drvno, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
es_status_codes GetPixelPointer(uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t camera, uint16_t** pdest, size_t* bytes_to_end_of_buffer);
es_status_codes SaveMeasurementDataToFile(const char* path, char* filename);
// 7) Before exiting your software, use this call for cleanup.
es_status_codes ExitDriver();


// Mid level API
// platform independent implementation
// hardware abstraction
es_status_codes ImportMeasurementDataFromFile(const char* filename);
es_status_codes ImportMeasurementDataFromFileBIN(const char* filename);
es_status_codes setBlockEn( uint32_t drvno );
es_status_codes resetBlockEn( uint32_t drvno );
es_status_codes setMeasureOn( uint32_t drvno );
es_status_codes resetMeasureOn( uint32_t drvno );
es_status_codes ResetDma( uint32_t drvno );
es_status_codes ClearAllUserRegs( uint32_t drvno );
es_status_codes SetPixelCountRegister(uint32_t drvno);
es_status_codes OpenShutter( uint32_t drvno );
es_status_codes SetCamCountRegister(uint32_t drvno);
es_status_codes SetSensorType( uint32_t drvno, uint16_t sensor_type );
es_status_codes SetCameraSystem( uint32_t drvno, uint16_t camera_system );
es_status_codes SetupFullBinning( uint32_t drvno, uint32_t lines, uint8_t vfreq );
es_status_codes SetupROI(uint32_t drvno, uint16_t number_of_regions, uint32_t lines, uint8_t* region_size, uint8_t vfreq);
es_status_codes SetupArea(uint32_t drvno, uint32_t lines_binning, uint8_t vfreq);
es_status_codes SetupVCLKReg( uint32_t drvno, uint32_t lines, uint8_t vfreq );
es_status_codes StopSTimer( uint32_t drvno );
es_status_codes RSFifo( uint32_t drvno );
es_status_codes allocateUserMemory( uint32_t drvno );
es_status_codes SetDMABufRegs( uint32_t drvno );
es_status_codes SetNosRegister(uint32_t drvno);
es_status_codes SetNobRegister(uint32_t drvno);
es_status_codes CloseShutter( uint32_t drvno );
es_status_codes SetSEC( uint32_t drvno, uint32_t ecin10ns );
es_status_codes SetTORReg( uint32_t drvno, uint8_t tor );
es_status_codes SetSSlope(uint32_t drvno, uint32_t sslope);
es_status_codes SetBSlope( uint32_t drvno, uint32_t slope );
es_status_codes SetSTI( uint32_t drvno, uint8_t sti_mode );
es_status_codes SetBTI( uint32_t drvno, uint8_t bti_mode );
es_status_codes SetSTimer( uint32_t drvno, uint32_t stime_in_microseconds );
es_status_codes SetBTimer( uint32_t drvno, uint32_t btime_in_microseconds );
es_status_codes SetGPXCtrl( uint32_t drvno, uint8_t GPXAddress, uint32_t GPXData );
es_status_codes ReadGPXCtrl( uint32_t drvno, uint8_t GPXAddress, uint32_t* GPXData );
es_status_codes InitGPX( uint32_t drvno, uint32_t delay );
es_status_codes SetSDAT( uint32_t drvno, uint32_t datin10ns );
es_status_codes SetBDAT( uint32_t drvno, uint32_t datin10ns );
es_status_codes Use_ENFFW_protection( uint32_t drvno, bool USE_ENFFW_PROTECT );
es_status_codes DAC8568_sendData( uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t ctrl, uint8_t addr, uint16_t data, uint8_t feature );
es_status_codes DAC8568_setOutput( uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output );
es_status_codes DAC8568_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, bool reorder_channels);
es_status_codes DAC8568_enableInternalReference(uint32_t drvno, uint8_t cameraPosition, uint8_t location);
es_status_codes SetBEC( uint32_t drvno, uint32_t bec_in_10ns);
es_status_codes SetXckdelay(uint32_t drvno, uint32_t xckdelay_in_10ns);
es_status_codes SetPartialBinning(uint32_t drvno, uint16_t number_of_regions);
es_status_codes ResetPartialBinning( uint32_t drvno );
es_status_codes SetupVPB(uint32_t drvno, uint32_t range, uint32_t lines);
es_status_codes SetDmaRegister( uint32_t drvno, uint32_t pixel );
es_status_codes SetDmaStartMode( uint32_t drvno, bool start_by_hardware);
es_status_codes FindCam( uint32_t drvno );
es_status_codes SetHardwareTimerStopMode( uint32_t drvno, bool stop_by_hardware );
es_status_codes ResetHardwareCounter( uint32_t drvno );
es_status_codes waitForBlockTrigger(uint32_t drvno);
es_status_codes countBlocksByHardware( uint32_t drvno );
es_status_codes StartSTimer( uint32_t drvno );
es_status_codes DoSoftwareTrigger(uint32_t drvno);
es_status_codes IsTimerOn( uint32_t drvno, bool* on );
es_status_codes GetLastBufPart( uint32_t drvno );
es_status_codes GetIndexOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t CAM, uint64_t* pIndex );
es_status_codes CheckFifoValid(uint32_t drvno, bool* valid);
es_status_codes CheckFifoOverflow(uint32_t drvno, bool* overflow);
es_status_codes CheckFifoEmpty(uint32_t drvno, bool* empty);
es_status_codes CheckFifoFull(uint32_t drvno, bool* full);
es_status_codes GetMeasureOn(uint32_t drvno, bool* measureOn);
es_status_codes OutTrigLow(uint32_t drvno);
es_status_codes OutTrigHigh(uint32_t drvno);
es_status_codes OutTrigPulse(uint32_t drvno, int64_t pulseWidthInMicroseconds);
es_status_codes readBlockTriggerState(uint32_t drv, uint8_t btrig_ch, bool* state);
es_status_codes WaitForBlockDone();
es_status_codes WaitForMeasureDone();
es_status_codes dumpS0Registers(uint32_t drvno, char** stringPtr);
es_status_codes dumpHumanReadableS0Registers(uint32_t drvno, char** stringPtr);
es_status_codes dumpDmaRegisters(uint32_t drvno, char** stringPtr);
es_status_codes dumpTlpRegisters(uint32_t drvno, char** stringPtr);
es_status_codes dumpMeasurementSettings(char** stringPtr);
es_status_codes dumpCameraSettings(uint32_t drvno, char** stringPtr);
es_status_codes dumpPciRegisters(uint32_t drvno, char** stringPtr);
es_status_codes _AboutDrv(uint32_t drvno, char** stringPtr);
es_status_codes _AboutGPX(uint32_t drvno, char** stringPtr);
es_status_codes _InitMeasurement(uint32_t drvno);
es_status_codes InitSoftware(uint32_t drvno);
es_status_codes InitPcieBoard(uint32_t drvno);
es_status_codes ResetDSC(uint32_t drvno, uint8_t DSCNumber);
es_status_codes SetDIRDSC(uint32_t drvno, uint8_t DSCNumber, bool dir);
es_status_codes GetDSC(uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC);
void PollDmaBufferToUserBuffer(uint32_t* drvno_p);
void GetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block);
void GetScanNumber(uint32_t drvno, int64_t offset, int64_t* sample, int64_t* block);
es_status_codes SetSticnt(uint32_t drvno, uint8_t divider);
es_status_codes SetBticnt(uint32_t drvno, uint8_t divider);
es_status_codes SetTocnt(uint32_t drvno, uint8_t divider);
es_status_codes GetIsTdc(uint32_t drvno, bool* isTdc);
es_status_codes GetIsDsc(uint32_t drvno, bool* isDsc);
void GetVerifiedDataDialog(struct verify_data_parameter* vd, char** resultString);
void SetContinuousMeasurement(bool on);
es_status_codes GetCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp);
es_status_codes GetCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood);
es_status_codes GetBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex);
es_status_codes GetScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex);
es_status_codes GetS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state);
es_status_codes GetS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state);
es_status_codes GetImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal);
es_status_codes GetImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal);
es_status_codes GetAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp);
void SetAllInterruptsDone(uint32_t drvno);
es_status_codes SetAbortMeasurementFlag();
es_status_codes ReadScanFrequencyBit(uint32_t drvno, bool* scanFrequencyTooHigh);
es_status_codes ResetScanFrequencyBit(uint32_t drvno);
es_status_codes ReadBlockFrequencyBit(uint32_t drvno, bool* blockFrequencyTooHigh);
es_status_codes ResetBlockFrequencyBit(uint32_t drvno);
es_status_codes SetS1S2ReadDelay(uint32_t drvno);
es_status_codes GetXckLength(uint32_t drvno, uint32_t* xckLengthIn10ns);
es_status_codes GetXckPeriod(uint32_t drvno, uint32_t* xckPeriodIn10ns);
es_status_codes GetBonLength(uint32_t drvno, uint32_t* bonLengthIn10ns);
es_status_codes GetBonPeriod(uint32_t drvno, uint32_t* bonPeriodIn10ns);
es_status_codes GetPcieCardVersion(uint32_t drvno, uint16_t* major_version, uint16_t* minor_version);
bool PcieCardVersionIsGreaterThan(uint32_t drvno, uint16_t major_version, uint16_t minor_version);
bool PcieCardVersionIsSmallerThan(uint32_t drvno, uint16_t major_version, uint16_t minor_version);
bool PcieCardVersionIsEqual(uint32_t drvno, uint16_t major_version, uint16_t minor_version);
es_status_codes GetBlockOn(uint32_t drvno, bool* block_on);
es_status_codes GetScanTriggerDetected(uint32_t drvno, bool* detected);
es_status_codes GetBlockTriggerDetected(uint32_t drvno, bool* detected);
es_status_codes ResetScanTriggerDetected(uint32_t drvno);
es_status_codes ResetBlockTriggerDetected(uint32_t drvno);
es_status_codes WaitForBlockOn(uint32_t drvno);
es_status_codes SetShiftS1S2ToNextScan(uint32_t drvno);
void manipulateData(uint32_t drvno, uint16_t* startAddress, uint32_t numberOfScansToManipulate);
void clearKeyStates();
#ifdef WIN32
es_status_codes SaveMeasurementDataToFileHDF5(const char* path, char* filename);
hid_t CreateNumericAttribute(hid_t parent_object_id, char* attr_name, hid_t goal_type, hid_t dataspace, void* data);
hid_t CreateStringAttribute(hid_t parent_object_id, char* attr_name, hid_t dataspace, void* data);
#endif

// helper functions
double CalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms);
double CalcRamUsageInMB(uint32_t nos, uint32_t nob);
es_status_codes CalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms);
void GetRmsVal(uint32_t nos, uint16_t *TRMSVals, double *mwf, double *trms);
es_status_codes ReturnStartMeasurement(es_status_codes status);
void FillUserBufferWithDummyData(uint32_t drvno);

// read and write functions
es_status_codes writeBitsS0_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint32_t address);
es_status_codes writeBitsS0_32_allBoards( uint32_t data, uint32_t bitmask, uint32_t address);
es_status_codes writeBitsS0_8( uint32_t drvno, uint8_t data, uint8_t bitmask, uint32_t address);
es_status_codes setBitS0_32(uint32_t drvno, uint32_t bitnumber, uint32_t address);
es_status_codes setBitS0_32_allBoards(uint32_t bitnumber, uint32_t address);
es_status_codes setBitS0_8(uint32_t drvno, uint32_t bitnumber, uint32_t address);
es_status_codes resetBitS0_32(uint32_t drvno, uint32_t bitnumber, uint32_t address);
es_status_codes resetBitS0_32_allBoards(uint32_t bitnumber, uint32_t address);
es_status_codes resetBitS0_8(uint32_t drvno, uint32_t bitnumber, uint32_t address);
es_status_codes readRegisterS0_32( uint32_t drvno, uint32_t* data, uint32_t address );
es_status_codes readRegisterS0_32_allBoards(uint32_t** data, uint32_t address);
es_status_codes readRegister_32_allBoards(uint32_t** data, uint32_t address);
es_status_codes readRegisterS0_16( uint32_t drvno, uint16_t* data, uint32_t address );
es_status_codes readRegisterS0_8( uint32_t drvno, uint8_t* data, uint32_t address );
es_status_codes writeRegisterS0_32( uint32_t drvno, uint32_t data, uint32_t address );
es_status_codes writeRegisterS0_32_allBoards( uint32_t data, uint32_t address );
es_status_codes writeRegisterS0_16( uint32_t drvno, uint16_t data, uint32_t address );
es_status_codes writeRegisterS0_8( uint32_t drv, uint8_t data, uint32_t address );
es_status_codes writeRegisterS0_8_allBoards( uint8_t data, uint32_t address );
es_status_codes writeRegisterDma_32(uint32_t drvno, uint32_t data, uint32_t address);
es_status_codes writeRegisterDma_8( uint32_t drvno, uint8_t data, uint32_t address );
es_status_codes readRegisterDma_32( uint32_t drvno, uint32_t* data, uint32_t address );
es_status_codes readRegisterDma_8( uint32_t drvno, uint8_t* data, uint32_t address );
es_status_codes writeBitsDma_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint32_t address);
es_status_codes writeBitsDma_8( uint32_t drvno, uint8_t data, uint8_t bitmask, uint32_t address);
es_status_codes pulseBitS0_32(uint32_t drvno, uint32_t bitnumber, uint32_t address, int64_t duration_in_microseconds);
es_status_codes pulseBitS0_8(uint32_t drvno, uint32_t bitnumber, uint32_t address, int64_t duration_in_microseconds);
es_status_codes ReadBitS0_32(uint32_t drvno, uint32_t address, uint8_t bitnumber, bool* isBitHigh);
es_status_codes ReadBitS0_8(uint32_t drvno, uint32_t address, uint8_t bitnumber, bool* isBitHigh);
#ifdef __cplusplus
}
#endif
