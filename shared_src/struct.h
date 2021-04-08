#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

struct ffloopparams
{
	uint32_t board_sel;
	uint32_t exptus;
	uint8_t exttrig;
	uint8_t blocktrigger;
	uint8_t btrig_ch;
};

struct global_settings
{
	uint32_t drvno;
	//measurement tab
	uint32_t nos;
	uint32_t nob;
	uint8_t sti_mode;
	uint8_t bti_mode;
	uint32_t stime_in_microsec;
	uint32_t btime_in_microsec;
	uint32_t sdat_in_100ns;
	uint32_t bdat_in_100ns;
	uint32_t sslope;
	uint32_t bslope;
	uint32_t xckdelay;
	uint32_t ShutterExpTime;
	uint16_t trigger_mode_cc;
	//camerasetup tab
	uint32_t sensor_type;
	uint8_t camera_system;
	uint32_t camcnt;
	uint32_t pixel;
	uint32_t mshut;
	uint16_t led_on;
	uint16_t gain_3010;
	uint8_t gain_3030;
	uint8_t Temp_level;
	uint8_t dac;
	uint8_t enable_gpx;
	uint32_t gpx_offset;
	//fftmodes tab
	uint32_t FFTLines;
	uint8_t Vfreq;
	uint8_t FFTMode;
	uint32_t lines_binning;
	uint16_t number_of_regions;
	uint8_t keep_first;
	uint8_t region_size[8];
	uint16_t dac_output[8];
	uint32_t TORmodus; 
	uint8_t ADC_Mode; 
	uint16_t ADC_custom_pettern;
	uint16_t isIRSensor; 
};

#endif // STRUCT_H
