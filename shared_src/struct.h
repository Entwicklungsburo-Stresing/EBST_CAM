#pragma once
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
	uint32_t camcnt; 
	uint32_t pixel; 
	uint32_t xckdelay; 
	uint32_t sensor_type; 
	uint32_t _mshut; 
	uint32_t ExpTime; 
	uint32_t m_TOmodus; 
	uint8_t FFTMode;
	uint32_t FFTLines; 
	uint16_t number_of_regions;
	uint32_t lines;
	uint8_t keep_first; 
	uint8_t* region_size;
	uint8_t Vfreq; 
	uint32_t lines_binning;
	uint32_t nos; 
	uint32_t nob; 
	uint8_t camera_system; 
	uint16_t trigger_mode; 
	uint8_t ADC_Mode; 
	uint16_t ADC_custom_pettern;
	uint16_t led_on; 
	uint16_t gain_high; 
	uint8_t gain; 
	uint8_t TrigMod; 
	uint8_t TOR_fkt; 
	uint8_t sti_mode; 
	uint8_t bti_mode; 
	uint32_t stime_in_microsec; 
	uint32_t btime_in_microsec; 
	uint8_t enable_gpx; 
	uint32_t gpx_offset; 
	uint16_t isIRSensor; 
	uint8_t Temp_level;
};