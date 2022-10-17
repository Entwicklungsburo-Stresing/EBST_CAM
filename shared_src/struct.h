#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

#define MAXPCIECARDS 5

// All settings are uint32_t to ensure the correct memory layout. This is important for the communication with labview.
// Don't change the order or you will have to change it for LabVIEW in InitMeasurement.vi.
struct global_settings
{
	/**
	 * useSoftwarePolling determines which method is used to copy data from DMA to user buffer.
	 *
	 * - true: Use Software Polling. When there is new available data in the DMA buffer, a thread copies the data one scan at a time to the user buffer. This method is reliable up to about 33kHz. This was measured with a 3030 camera system with 1088 pixels, priority of the thread is not changed, 14.12.2021, FH.
	 * - false: Use Interrupt. Every dma_buffer_size_in_scans/2 scan the interrupt starts a copy process, which copies dma_buffer_size_in_scans/2 scans to the user buffer. 1000 is our default value for dma_buffer_size_in_scans, so interrupt is started every 500 scans.
	 */
	uint32_t useSoftwarePolling;
	/**
	 * Number of samples
	 */
	uint32_t nos;
	/**
	 * Number of blocks
	 */
	uint32_t nob;
	/**
	 * Scan trigger input mode. See enum sti_mode in enum.h for options.
	 */
	uint32_t sti_mode;
	/**
	 * Block trigger input mode. See enum bti_mode in enum.h for options.
	 */
	uint32_t bti_mode;
	/**
	 * Scan timer in microseconds.
	 */
	uint32_t stime_in_microsec;
	/**
	 * Block timer in microseconds.
	 */
	uint32_t btime_in_microsec;
	/**
	 * Scan delay after trigger in 10 ns steps.
	 */
	uint32_t sdat_in_10ns;
	/**
	 * Block delay after trigger in 10 ns steps.
	 */
	uint32_t bdat_in_10ns;
	/**
	 * Scan trigger slope.
	 */
	uint32_t sslope;
	/**
	 * Block trigger slope.
	 */
	uint32_t bslope;
	/**
	 * XCK delay in 10 ns steps.
	 */
	uint32_t xckdelay_in_10ns;
	/**
	 * SEC in 10 ns steps.
	 */
	uint32_t sec_in_10ns;
	/**
	 * Trigger mode of camera control.
	 */
	uint32_t trigger_mode_cc;
	/**
	 * Board select
	 * 
	 * TODO: Check if there are places where board_sel is expected as 0, 1, 2 and not 1, 2, 3.
	 * - 1: board 1
	 * - 2: board 2
	 * - 3: both boards
	 */
	uint32_t board_sel;
	/**
	 * Sensor type.
	 * - 0: PDA (line sensor)
	 * - 1: FFT (area sensor)
	 */
	uint32_t sensor_type;
	/**
	 * Camera system
	 * - 0: 3001
	 * - 1: 3010
	 * - 2: 3030
	 */
	uint32_t camera_system;
	/**
	 * Camera count
	 */
	uint32_t camcnt;
	/**
	 * Pixel count
	 */
	uint32_t pixel;
	/**
	 * Mechanical shutter
	 */
	uint32_t mshut;
	/**
	 * Turn leds off
	 */
	uint32_t led_off;
	/**
	 * Sensor gain
	 */
	uint32_t sensor_gain;
	/**
	 * ADC gain
	 */
	uint32_t adc_gain;
	/**
	 * Temperatur level
	 */
	uint32_t Temp_level;
	/**
	 * Turn digital analog converter on / off
	 * - 0: off
	 * - >0: on
	 */
	uint32_t dac;
	/**
	 * unused
	 */
	uint32_t unused;
	/**
	 * GPX offset
	 */
	uint32_t gpx_offset;
	/**
	 * Count of lines for FFT sensors
	 */
	uint32_t FFTLines;
	/**
	 * Vertical frequency for FFT sensors
	 */
	uint32_t Vfreq;
	/**
	 * Mode for FFT sensors
	 */
	uint32_t FFTMode;
	/**
	 * Count of lines which are binned in area mode for FFT sensors
	 */
	uint32_t lines_binning;
	/**
	 * Number of regions for fft mode range of interest
	 */
	uint32_t number_of_regions;
	/**
	 * Deprecated: Keep. Was a parameter to determine whether a region for region of interest mode is filled with real or dummy data.
	 */
	uint32_t keep;
	/**
	 * Size for each region for region of interest mode for FFT sensors. When the first region is set to 0, all regions are automatically same sized.
	 */
	uint32_t region_size[8];
	/**
	 * Array for output levels of each digital to analog converter
	 */
	uint32_t dac_output[MAXPCIECARDS][8];
	uint32_t TORmodus; 
	uint32_t ADC_Mode;
	uint32_t ADC_custom_pattern;
	uint32_t bec_in_10ns;
	uint32_t cont_pause_in_microseconds;
	uint32_t isIr;
	uint32_t IOCtrl_impact_start_pixel;
	uint32_t IOCtrl_output_width_in_5ns[8];
	uint32_t IOCtrl_output_delay_in_5ns[8];
	uint32_t IOCtrl_T0_period_in_10ns;
	/**
	 * Size of DMA buffer in scans. 1000 is our default. 60 is also working with highspeed (expt=0,02ms). 30 could be with one wrong scan every 10000 scans.
	 */
	uint32_t dma_buffer_size_in_scans;
	uint32_t tocnt;
	uint32_t ticnt;
	uint32_t use_ec;
};

#endif // STRUCT_H
