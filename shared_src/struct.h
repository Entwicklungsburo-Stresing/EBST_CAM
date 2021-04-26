#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

struct global_settings
{
	uint32_t drvno;
	//measurement tab
	uint32_t nos;
	uint32_t nob;
	uint32_t sti_mode;
	uint32_t bti_mode;
	uint32_t stime_in_microsec;
	uint32_t btime_in_microsec;
	uint32_t sdat_in_100ns;
	uint32_t bdat_in_100ns;
	uint32_t sslope;
	uint32_t bslope;
	uint32_t xckdelay;
	uint32_t ShutterExpTime;
	uint32_t trigger_mode_cc;
	//camerasetup tab
	uint32_t board_sel;
	uint32_t sensor_type;
	uint32_t camera_system;
	uint32_t camcnt;
	uint32_t pixel;
	uint32_t mshut;
	uint32_t led_on;
	uint32_t gain_3010;
	uint32_t gain_3030;
	uint32_t Temp_level;
	uint32_t dac;
	uint32_t enable_gpx;
	uint32_t gpx_offset;
	//fftmodes tab
	uint32_t FFTLines;
	uint32_t Vfreq;
	uint32_t FFTMode;
	uint32_t lines_binning;
	uint32_t number_of_regions;
	uint32_t keep_first;
	uint32_t region_size[8];
	uint32_t dac_output[8];
	uint32_t TORmodus; 
	uint32_t ADC_Mode;
	uint32_t ADC_custom_pettern;
	uint32_t isIRSensor;
};

#endif // STRUCT_H
