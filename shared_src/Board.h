#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Board_ll.h"

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
es_status_codes ReturnFrame(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t pixel, uint16_t* pdest);
// 7) Before exiting your software, use this call for cleanup.
es_status_codes ExitDriver();


// Mid level API
// platform independent implementation
// hardware abstraction
es_status_codes setBlockOn( uint32_t drvno );
es_status_codes resetBlockOn( uint32_t drvno );
es_status_codes setMeasureOn( uint32_t drvno );
es_status_codes resetMeasureOn( uint32_t drvno );
es_status_codes ResetDma( uint32_t drvno );
es_status_codes ClearAllUserRegs( uint32_t drvno );
es_status_codes SetPixelCountRegister(uint32_t drvno);
es_status_codes OpenShutter( uint32_t drvno );
es_status_codes SetCamCountRegister(uint32_t drvno);
es_status_codes SetSensorType( uint32_t drvno, uint8_t sensor_type );
es_status_codes SetupFullBinning( uint32_t drvno, uint32_t lines, uint8_t vfreq );
es_status_codes SetupROI(uint32_t drvno, uint16_t number_of_regions, uint32_t lines, uint8_t keep, uint8_t* region_size, uint8_t vfreq);
es_status_codes SetupArea(uint32_t drvno, uint32_t lines_binning, uint8_t vfreq);
es_status_codes SetupVCLKReg( uint32_t drvno, uint32_t lines, uint8_t vfreq );
es_status_codes StopSTimer( uint32_t drvno );
es_status_codes RSFifo( uint32_t drvno );
es_status_codes allocateUserMemory( uint32_t drvno );
es_status_codes SetDMABufRegs( uint32_t drvno );
es_status_codes CloseShutter( uint32_t drvno );
es_status_codes SetSEC( uint32_t drvno, uint32_t ecin10ns );
es_status_codes SetEnEc3030(uint32_t drvno, bool enable);
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
es_status_codes InitCamera3001( uint32_t drvno );
es_status_codes InitCamera3010( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern );
es_status_codes Cam3010_ADC_reset( uint32_t drvno );
es_status_codes Cam3010_ADC_setOutputMode(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern);
es_status_codes Cam3010_ADC_sendTestPattern(uint32_t drvno, uint16_t custom_pattern);
es_status_codes InitCamera3030(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern, uint8_t adc_gain, uint32_t** dac_output, bool is_hs_ir);
es_status_codes Cam3030_ADC_reset( uint32_t drvno );
es_status_codes Cam3030_ADC_twoWireModeEN( uint32_t drvno );
es_status_codes Cam3030_ADC_SetGain( uint32_t drvno, uint8_t gain );
es_status_codes SetADGain( uint32_t drvno, uint8_t fkt, uint8_t g1, uint8_t g2, uint8_t g3, uint8_t g4, uint8_t g5, uint8_t g6, uint8_t g7, uint8_t g8 );
es_status_codes Cam3030_ADC_RampOrPattern( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern );
es_status_codes Cam3030_ADC_Global_En_Filter(uint32_t drvno, bool enable);
es_status_codes Cam3030_ADC_SetFilterSettings(uint32_t drvno, uint8_t channel, uint8_t coeff_set, uint8_t decimation_factor, uint8_t odd_tap, uint8_t use_filter, uint8_t hpf_corner, uint8_t en_hpf);
es_status_codes Cam3030_ADC_SetFilterCustomCoefficient(uint32_t drvno, uint8_t channel, uint8_t coefficient_number, uint8_t enable, uint16_t coefficient);
es_status_codes Cam3030_ADC_SetDataRate(uint32_t drvno, uint8_t data_rate);
es_status_codes Cam3030_ADC_SetLFNS(uint32_t drvno, bool enable);
es_status_codes Cam3030_ADC_SetSampleMode(uint32_t drvno, uint8_t sample_mode);
es_status_codes Cam3030_ADC_SetIfcMode(uint32_t drvno, uint16_t ifc_mode);
es_status_codes SetTemp( uint32_t drvno, uint8_t level );
es_status_codes DAC8568_sendData( uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t ctrl, uint8_t addr, uint16_t data, uint8_t feature );
es_status_codes DAC8568_setOutput( uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output );
es_status_codes DAC8568_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, bool reorder_channels);
es_status_codes DAC8568_enableInternalReference(uint32_t drvno, uint8_t cameraPosition, uint8_t location);
es_status_codes SetBEC( uint32_t drvno, uint32_t ecin10ns );
es_status_codes SetXckdelay(uint32_t drvno, uint32_t xckdelay_in_10ns);
es_status_codes SetPartialBinning(uint32_t drvno, uint16_t number_of_regions);
es_status_codes ResetPartialBinning( uint32_t drvno );
es_status_codes SetupVPB(uint32_t drvno, uint32_t range, uint32_t lines, bool keep);
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
es_status_codes GetAddressOfPixel( uint32_t drvno, uint16_t pixel, uint32_t sample, uint32_t block, uint16_t CAM, uint16_t** address );
es_status_codes checkFifoFlags(uint32_t drvno, bool* valid);
es_status_codes checkFifoOverflow(uint32_t drvno, bool* overflow);
es_status_codes isBlockOn(uint32_t drvno, bool* blockOn);
es_status_codes isMeasureOn(uint32_t drvno, bool* measureOn);
es_status_codes LedOff(uint32_t drvno, uint8_t LED_OFF);
es_status_codes setUseEC(uint32_t drvno, uint16_t use_EC);
es_status_codes OutTrigLow(uint32_t drvno);
es_status_codes OutTrigHigh(uint32_t drvno);
es_status_codes OutTrigPulse(uint32_t drvno, uint32_t PulseWidth);
es_status_codes readBlockTriggerState(uint32_t drv, uint8_t btrig_ch, bool* state);
es_status_codes SetGain(uint32_t drvno, uint16_t gain_value);
es_status_codes waitForBlockReady(uint32_t board_sel);
es_status_codes waitForMeasureReady(uint32_t board_sel);
es_status_codes dumpS0Registers(uint32_t drvno, char** stringPtr);
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
es_status_codes InitCamera(uint32_t drvno);
es_status_codes SetConfigRegister(uint32_t drvno);
es_status_codes SetupFFT(uint32_t drvno);
es_status_codes SetupFullBinningInCamera(uint32_t drvno);
es_status_codes SetupPartialBinningInCamera(uint32_t drvno);
es_status_codes SetVfreqRegister(uint32_t drvno);
es_status_codes IOCtrl_setImpactStartPixel(uint32_t drvno, uint16_t startPixel);
es_status_codes IOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns);
es_status_codes IOCtrl_setAllOutputs(uint32_t drvno, uint32_t* width_in_5ns, uint32_t* delay_in_5ns);
es_status_codes IOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns);
es_status_codes ResetDSC(uint32_t drvno, uint8_t DSCNumber);
es_status_codes SetDIRDSC(uint32_t drvno, uint8_t DSCNumber, bool dir);
es_status_codes GetDSC(uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC);
void PollDmaBufferToUserBuffer(uint32_t* drvno_p);
void GetCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block);
void GetScanNumber(uint32_t drvno, int64_t offset, int64_t* sample, int64_t* block);
es_status_codes SetTicnt(uint32_t drvno, uint8_t divider);
es_status_codes SetTocnt(uint32_t drvno, uint8_t divider);
es_status_codes GetIsTdc(uint32_t drvno, bool* isTdc);
es_status_codes GetIsDsc(uint32_t drvno, bool* isDsc);
es_status_codes SetMshut( uint32_t drvno, bool mshut );
es_status_codes SetSensorResetEnable(uint32_t drvno, bool enable);
es_status_codes SetSensorResetShort(uint32_t drvno, bool enable_short);
es_status_codes SetSensorResetEarly(uint32_t drvno, bool enable_early);
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
es_status_codes GetOneBlockOfOneCamera(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t** data);
void SetAllInterruptsDone(uint32_t drvno);
es_status_codes SetCameraPosition(uint32_t drvno);
es_status_codes SetAbortMeasurementFlag();

// helper functions
double CalcMeasureTimeInSeconds(uint32_t nos, uint32_t nob, double exposure_time_in_ms);
double CalcRamUsageInMB(uint32_t nos, uint32_t nob);
es_status_codes CalcTrms(uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms);
void GetRmsVal(uint32_t nos, uint16_t *TRMSVals, double *mwf, double *trms);
es_status_codes ReturnStartMeasurement(es_status_codes status);
void FillUserBufferWithDummyData(uint32_t drvno);

// read and write functions
es_status_codes writeBitsS0_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint16_t address);
es_status_codes writeBitsS0_32_allBoards( uint32_t board_sel, uint32_t data, uint32_t bitmask, uint16_t address);
es_status_codes writeBitsS0_8( uint32_t drvno, uint8_t data, uint8_t bitmask, uint16_t address);
es_status_codes setBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes setBitS0_32_allBoards(uint32_t board_sel, uint32_t bitnumber, uint16_t address);
es_status_codes setBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes resetBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes resetBitS0_32_allBoards(uint32_t board_sel, uint32_t bitnumber, uint16_t address);
es_status_codes resetBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes readRegisterS0_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes readRegisterS0_32_allBoards(uint32_t board_sel, uint32_t** data, uint16_t address);
es_status_codes readRegister_32_allBoards(uint32_t board_sel, uint32_t** data, uint16_t address);
es_status_codes readRegisterS0_16( uint32_t drvno, uint16_t* data, uint16_t address );
es_status_codes readRegisterS0_8( uint32_t drvno, uint8_t* data, uint16_t address );
es_status_codes writeRegisterS0_32( uint32_t drvno, uint32_t data, uint16_t address );
es_status_codes writeRegisterS0_32_allBoards( uint32_t board_sel, uint32_t data, uint16_t address );
es_status_codes writeRegister_32_allBoards(uint32_t board_sel, uint32_t data, uint16_t address);
es_status_codes writeRegisterS0_16( uint32_t drvno, uint16_t data, uint16_t address );
es_status_codes writeRegisterS0_8( uint32_t drv, uint8_t data, uint16_t address );
es_status_codes writeRegisterS0_8_allBoards( uint32_t board_sel, uint8_t data, uint16_t address );
es_status_codes writeRegister_8_allBoards( uint32_t board_sel, uint8_t data, uint16_t address );
es_status_codes SendFLCAM( uint32_t drvno, uint8_t maddr, uint8_t adaddr, uint16_t data );
es_status_codes writeRegisterDma_32( uint32_t drvno, uint32_t data, uint16_t address );
es_status_codes writeRegisterDma_8( uint32_t drvno, uint8_t data, uint16_t address );
es_status_codes readRegisterDma_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes readRegisterDma_8( uint32_t drvno, uint8_t* data, uint16_t address );
es_status_codes writeBitsDma_32( uint32_t drvno, uint32_t data, uint32_t bitmask, uint16_t address);
es_status_codes writeBitsDma_8( uint32_t drvno, uint8_t data, uint8_t bitmask, uint16_t address);
es_status_codes pulseBitS0_32(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes pulseBitS0_8(uint32_t drvno, uint32_t bitnumber, uint16_t address);
#ifdef __cplusplus
}
#endif
#endif // BOARD_H
