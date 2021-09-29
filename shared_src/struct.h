#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

#define MAXPCIECARDS 5

// All settings are uin32_t to ensure the correct memory layout. This is important for the communication with labview.
// Don't change the order or you will have to change it for LabVIEW in InitMeasurement.vi.
struct global_settings
{
	uint32_t unused;
	uint32_t nos;
	uint32_t nob;
	uint32_t sti_mode;
	uint32_t bti_mode;
	uint32_t stime_in_microsec;
	uint32_t btime_in_microsec;
	uint32_t sdat_in_10ns;
	uint32_t bdat_in_10ns;
	uint32_t sslope;
	uint32_t bslope;
	uint32_t xckdelay_in_10ns;
	uint32_t sec_in_10ns;
	uint32_t trigger_mode_cc;
	uint32_t board_sel;
	uint32_t sensor_type;
	uint32_t camera_system;
	uint32_t camcnt;
	uint32_t pixel;
	uint32_t mshut;
	uint32_t led_off;
	uint32_t gain_switch;
	uint32_t gain_3030;
	uint32_t Temp_level;
	uint32_t dac;
	uint32_t enable_gpx;
	uint32_t gpx_offset;
	uint32_t FFTLines;
	uint32_t Vfreq;
	uint32_t FFTMode;
	uint32_t lines_binning;
	uint32_t number_of_regions;
	uint32_t keep;
	uint32_t region_size[8];
	uint32_t dac_output[MAXPCIECARDS][8];
	uint32_t TORmodus; 
	uint32_t ADC_Mode;
	uint32_t ADC_custom_pattern;
	uint32_t bec_in_10ns;
	uint32_t cont_pause;
	uint32_t isIr;
	uint32_t IOCtrl_impact_start_pixel;
	uint32_t IOCtrl_output_width_in_5ns[8];
	uint32_t IOCtrl_output_delay_in_5ns[8];
	uint32_t IOCtrl_T0_period_in_10ns;
};

#endif // STRUCT_H
