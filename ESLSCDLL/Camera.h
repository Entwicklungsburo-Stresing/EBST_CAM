#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../shared_src/es_status_codes.h"

es_status_codes Cam_Init(uint32_t drvno);
es_status_codes Cam_SendData(uint32_t drvno, uint8_t maddr, uint8_t adaddr, uint16_t data);
es_status_codes Cam_SetSensorResetOrHsirEc(uint32_t drvno, uint16_t sensor_reset_or_hsir_ec);
es_status_codes Cam_DoSoftReset(uint32_t drvno);
es_status_codes Cam_Initialize(uint32_t drvno);
es_status_codes Cam_SetTemp(uint32_t drvno, uint8_t level);
es_status_codes Cam_SetupFullBinning(uint32_t drvno);
es_status_codes Cam_SetupPartialBinning(uint32_t drvno);
es_status_codes Cam_SetVfreqRegister(uint32_t drvno);
es_status_codes Cam_SetLedOff(uint32_t drvno, uint8_t LED_OFF);
es_status_codes Cam_SetPosition(uint32_t drvno);
es_status_codes Cam_SetPixelRegister(uint32_t drvno);
es_status_codes Cam_SetTriggerInput(uint32_t drvno);
es_status_codes Cam_SetConfigRegister(uint32_t drvno);
es_status_codes Cam_SetupFFT(uint32_t drvno);
es_status_codes Cam3001_Init(uint32_t drvno);
es_status_codes Cam3010_Init(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern);
es_status_codes Cam3010_ADC_reset(uint32_t drvno);
es_status_codes Cam3010_ADC_setOutputMode(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern);
es_status_codes Cam3010_ADC_sendTestPattern(uint32_t drvno, uint16_t custom_pattern);
es_status_codes Cam3030_Init(uint32_t drvno);
es_status_codes Cam3030_ADC_reset(uint32_t drvno);
es_status_codes Cam3030_ADC_twoWireModeEN(uint32_t drvno);
es_status_codes Cam3030_ADC_SetGain(uint32_t drvno, uint8_t gain);
es_status_codes SetADGain(uint32_t drvno, uint8_t fkt, uint8_t g1, uint8_t g2, uint8_t g3, uint8_t g4, uint8_t g5, uint8_t g6, uint8_t g7, uint8_t g8);
es_status_codes Cam3030_ADC_RampOrPattern(uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern);
es_status_codes Cam3030_ADC_Global_En_Filter(uint32_t drvno, bool enable);
es_status_codes Cam3030_ADC_SetFilterSettings(uint32_t drvno, uint8_t channel, uint8_t coeff_set, uint8_t decimation_factor, uint8_t odd_tap, uint8_t use_filter, uint8_t hpf_corner, uint8_t en_hpf);
es_status_codes Cam3030_ADC_SetFilterCustomCoefficient(uint32_t drvno, uint8_t channel, uint8_t coefficient_number, uint8_t enable, uint16_t coefficient);
es_status_codes Cam3030_ADC_SetDataRate(uint32_t drvno, uint8_t data_rate);
es_status_codes Cam3030_ADC_SetLFNS(uint32_t drvno, bool enable);
es_status_codes Cam3030_ADC_SetSampleMode(uint32_t drvno, uint8_t sample_mode);
es_status_codes CamIOCtrl_setImpactStartPixel(uint32_t drvno, uint16_t startPixel);
es_status_codes CamIOCtrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns);
es_status_codes CamIOCtrl_setAllOutputs(uint32_t drvno, uint32_t* width_in_5ns, uint32_t* delay_in_5ns);
es_status_codes CamIOCtrl_setT0(uint32_t drvno, uint32_t period_in_10ns);
es_status_codes Cam_DAC8568_sendData(uint32_t drvno, uint32_t data, uint8_t cameraPosition);
