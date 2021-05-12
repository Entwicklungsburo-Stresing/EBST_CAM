#ifndef CROSSPLATTFORMBOARD_H
#define CROSSPLATTFORMBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "crossPlattformBoard_ll.h"
#include "enum.h"
#include "struct.h"
#include <stdbool.h>

#define MAXPCIECARDS 5
#define DMA_BUFFER_SIZE_IN_SCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans
#define DMA_BUFFER_PARTS 2
#define DMA_DMASPERINTR DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb 

extern uint32_t* aPIXEL;
extern uint32_t* aCAMCNT;
extern bool* useSWTrig;
extern uint16_t** userBuffer;
extern uint32_t BOARD_SEL;
extern uint8_t number_of_boards;
extern uint32_t Nob;
extern uint32_t* Nospb;

// High level API
// plattform independet implementation
es_status_codes InitBoard(uint32_t drvno);	// init the board and alloc mem, call only once !
es_status_codes CCDDrvInit();
es_status_codes CCDDrvExit(uint32_t drvno);	// closes the driver
es_status_codes InitMeasurement(struct global_settings settings);
es_status_codes StartMeasurement();
es_status_codes abortMeasurement(uint32_t drv);
es_status_codes ReturnFrame(uint32_t drv, uint32_t curr_nos, uint32_t curr_nob, uint16_t curr_cam, uint16_t* pdest, uint32_t length);

// Mid level API
// plattform independet implementation
// hardware abstraction
es_status_codes resetBlockOn( uint32_t drvno );
es_status_codes resetMeasureOn( uint32_t drvno );
es_status_codes SetDMAReset( uint32_t drvno );
es_status_codes ClearAllUserRegs( uint32_t drvno );
es_status_codes SetGlobalVariables( uint32_t drvno, uint32_t camcnt, uint32_t pixel );
es_status_codes SetBoardVars( uint32_t drvno );
es_status_codes SetPixelCount(uint32_t drvno, uint16_t pixelcount);
es_status_codes OpenShutter( uint32_t drvno );
es_status_codes SetCamCount(uint32_t drvno, uint16_t camcount);
es_status_codes SetSensorType( uint32_t drvno, uint8_t sensor_type );
es_status_codes SetupFullBinning( uint32_t drvno, uint32_t lines, uint8_t vfreq );
es_status_codes SetupVCLKReg( uint32_t drvno, uint32_t lines, uint8_t vfreq );
es_status_codes StopSTimer( uint32_t drvno );
es_status_codes SetMeasurementParameters( uint32_t drvno, uint32_t nos, uint32_t nob );
es_status_codes RSFifo( uint32_t drvno );
es_status_codes allocateUserMemory( uint32_t drvno );
es_status_codes SetDMABufRegs( uint32_t drvno );
es_status_codes CloseShutter( uint32_t drvno );
es_status_codes SetSEC( uint32_t drvno, uint32_t ecin100ns );
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
es_status_codes SetSDAT( uint32_t drvno, uint32_t datin100ns );
es_status_codes SetBDAT( uint32_t drvno, uint32_t datin100ns );
es_status_codes Use_ENFFW_protection( uint32_t drvno, bool USE_ENFFW_PROTECT );
es_status_codes InitCameraGeneral( uint32_t drvno, uint16_t pixel, uint16_t cc_trigger_input, uint8_t IS_FFT, uint8_t IS_AREA, uint8_t IS_COOLED, uint16_t led_off);
es_status_codes InitCamera3001( uint32_t drvno, uint8_t gain_switch );
es_status_codes InitCamera3010( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern, uint16_t gain_switch );
es_status_codes Cam3010_ADC_reset( uint32_t drvno );
es_status_codes InitCamera3030( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern, uint8_t gain );
es_status_codes Cam3030_ADC_reset( uint32_t drvno );
es_status_codes Cam3030_ADC_twoWireModeEN( uint32_t drvno );
es_status_codes Cam3030_ADC_SetGain( uint32_t drvno, uint8_t gain );
es_status_codes SetADGain( uint32_t drvno, uint8_t fkt, uint8_t g1, uint8_t g2, uint8_t g3, uint8_t g4, uint8_t g5, uint8_t g6, uint8_t g7, uint8_t g8 );
es_status_codes Cam3030_ADC_RampOrPattern( uint32_t drvno, uint8_t adc_mode, uint16_t custom_pattern );
es_status_codes SetTemp( uint32_t drvno, uint8_t level );
es_status_codes SendFLCAM_DAC( uint32_t drvno, uint8_t ctrl, uint8_t addr, uint16_t data, uint8_t feature );
es_status_codes DAC_setOutput( uint32_t drvno, uint8_t channel, uint16_t output );
es_status_codes SetBEC( uint32_t drvno, uint32_t ecin100ns );
es_status_codes SetXckdelay(uint32_t drvno, uint32_t xckdelay);
es_status_codes ResetPartialBinning( uint32_t drvno );

// read and write functions
es_status_codes writeBitsS0_32( uint32_t Data, uint32_t Bitmask, uint16_t Address, uint32_t drvno );
es_status_codes setBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes resetBitS0(uint32_t drvno, uint32_t bitnumber, uint16_t address);
es_status_codes readRegisterS0_32( uint32_t drvno, uint32_t* data, uint16_t address );
es_status_codes readRegisterS0_16( uint32_t drvno, uint16_t* data, uint16_t address );
es_status_codes readRegisterS0_8( uint32_t drvno, uint8_t* data, uint16_t address );
es_status_codes writeRegisterS0_32( uint32_t drvno, uint32_t data, uint16_t address );
es_status_codes writeRegisterS0_16( uint32_t drvno, uint16_t data, uint16_t address );
es_status_codes writeRegisterS0_8( uint32_t drv, uint8_t data, uint16_t address );
es_status_codes SendFLCAM( uint32_t drvno, uint8_t maddr, uint8_t adaddr, uint16_t data );

#ifdef __cplusplus
}
#endif
#endif // CROSSPLATTFORMBOARD_H