#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

#define MAXPCIECARDS 5

// All settings are uint32_t to ensure the correct memory layout. This is important for the communication with labview.
// Don't change the order or you will have to change it for LabVIEW in InitMeasurement.vi.
struct global_settings
{
	/**
	 * use_software_polling determines which method is used to copy data from DMA to user buffer.
	 *
	 * - >0: Use Software Polling. When there is new available data in the DMA buffer, a thread copies the data one scan at a time to the user buffer. This method is reliable up to about 33kHz. This was measured with a 3030 camera system with 1088 pixels, priority of the thread is not changed, 14.12.2021, FH.
	 * - =0: Use Interrupt. Every dma_buffer_size_in_scans/2 scan the interrupt starts a copy process, which copies dma_buffer_size_in_scans/2 scans to the user buffer. 1000 is our default value for dma_buffer_size_in_scans, so interrupt is started every 500 scans.
	 */
	uint32_t use_software_polling;
	/**
	 * Number of samples. 32 bit. Min: 2, max: max of uint32
	 */
	uint32_t nos;
	/**
	 * Number of blocks. 32 bit. Min: 1, max: max of uint32
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
	 * Scan timer in microseconds. Used when sti mode is stimer. 28 bit.
	 */
	uint32_t stime_in_microsec;
	/**
	 * Block timer in microseconds. Used when bti mode is btimer. 28 bit.
	 */
	uint32_t btime_in_microsec;
	/**
	 * Scan delay after trigger in 10 ns steps. 31 bit.
	 */
	uint32_t sdat_in_10ns;
	/**
	 * Block delay after trigger in 10 ns steps. 31 bit.
	 */
	uint32_t bdat_in_10ns;
	/**
	 * Scan trigger slope. See enum sslope in enum.h for options.
	 */
	uint32_t sslope;
	/**
	 * Block trigger slope. See enum bslope in enum.h for options.
	 */
	uint32_t bslope;
	/**
	 * XCK delay in 10 ns steps. 31 bit.
	 */
	uint32_t xckdelay_in_10ns;
	/**
	 * exposure control in 10 ns steps. 31 bit.
	 */
	uint32_t sec_in_10ns;
	/**
	 * Trigger mode of camera control. See enum trigger_mode in enum.h for options.
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
	 * Sensor type. See enum sensor_type in enum.h for options.
	 */
	uint32_t sensor_type;
	/**
	 * Camera system. See enum camera_system in enum.h for options.
	 */
	uint32_t camera_system;
	/**
	 * Camera count. 1 ... 16. 4 bit.
	 */
	uint32_t camcnt;
	/**
	 * Pixel count. Only 64*n are allowed. 16 bit.
	 */
	uint32_t pixel;
	/**
	 * Mechanical shutter.
	 *	- =0 don't use mechanical shutter
	 *	- >0 use mechanical shutter
	 */
	uint32_t mshut;
	/**
	 * Turn leds off.
	 *	=0 led on
	 *	>0 led off
	 */
	uint32_t led_off;
	/**
	 * Sensor gain for IR sensors. 0 for no gain, >0 for gain.
	 *	- camera system 3001/3010: 0 to 1
	 *	- camera system 3030: 0 to 3
	 */
	uint32_t sensor_gain;
	/**
	 * ADC gain. 0...12. default: 5. 8 bit
	 */
	uint32_t adc_gain;
	/**
	 * Temperatur level for cooled cameras. 0...7 / 0=off, 7=min
	 */
	uint32_t temp_level;
	/**
	 * Turn digital analog converter on / off
	 * - =0: off
	 * - >0: on
	 */
	uint32_t dac;
	/**
	 * Shortrs controls the sensor reset length.
	 *	- =0: long reset 800ns
	 *	- >0: short reset 380ns
	 */
	uint32_t shortrs;
	/**
	 * GPX offset
	 */
	uint32_t gpx_offset;
	/**
	 * Count of lines for FFT sensors. 12 bit.
	 */
	uint32_t fft_lines;
	/**
	 * Vertical frequency for FFT sensors. 8 bit.
	 */
	uint32_t vfreq;
	/**
	 * Mode for FFT sensors. See enum fft_mode in enum.h for options.
	 */
	uint32_t fft_mode;
	/**
	 * Count of lines which are binned in area mode for FFT sensors. 8 bit.
	 */
	uint32_t lines_binning;
	/**
	 * Number of regions for fft mode range of interest. Min: 1. 16 bit.
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
	/**
	 * Output mode for PCIe board output pin. See enum tor_out in enum.h for options.
	 */
	uint32_t tor;
	/**
	 * ADC operating mode. Only available for specific ADCs, e.g. in camera system 3030. See enum adc_mode in enum.h for opions.
	 */
	uint32_t adc_mode;
	/**
	 * Fixed value for ADC output when adc mode is custom pattern.
	 */
	uint32_t adc_custom_pattern;
	/**
	 * bec in 10 ns
	 */
	uint32_t bec_in_10ns;
	/**
	 * Pause between two measurements when continuous mode is on.
	 */
	uint32_t cont_pause_in_microseconds;
	/**
	 * Determines whether the camera is a high speed infrared camera.
	 *	- =0 no IR
	 *	- >0 IR
	 */
	uint32_t is_hs_ir;
	/**
	 * Determines at which pixel number of one scan the inputs of ioctrl are written to.
	 */
	uint32_t ioctrl_impact_start_pixel;
	/**
	 * This is an array, which sets the width of the ioctrl outputs in 5ns steps.
	 */
	uint32_t ioctrl_output_width_in_5ns[8];
	/**
	 * This is an array, which sets the delay of the ioctrl outputs in 5ns steps.
	 */
	uint32_t ioctrl_output_delay_in_5ns[8];
	/**
	 * Determines the base frequency T0 of the ioctrl pulse generator in 10ns steps.
	 */
	uint32_t ioctrl_T0_period_in_10ns;
	/**
	 * Size of DMA buffer in scans. 1000 is our default. 60 is also working with highspeed (expt=0,02ms). 30 could be with one wrong scan every 10000 scans.
	 */
	uint32_t dma_buffer_size_in_scans;
	/**
	 * Trigger output counter determines how many TO_CNT_OUT are skipped. Use tor_to_cnt_out for setting tor to see TO_CNT_OUT at the output of the PCIe board. Every tocnt+1 TO_CNT_OUT is equal with XCK. Only the lowest 7 bits are used for this setting.
	 */
	uint32_t tocnt;
	/**
	 * Trigger output counter determines how many trigger inputs are skipped before the next measurement is triggered. Every ticnt+1 trigger input the measurement is triggered according to sti_mode. Only the lowest 7 bits are used for this setting.
	 */
	uint32_t ticnt;
	/**
	 * 
	 */
	uint32_t use_ec;
	/**
	 * - =0: Don't write measurement data to disc.
	 * - >0: Write measurement data to disc.
	 */
	uint32_t write_to_disc;
	/**
	 * File path is specifying the path where the measurement data is saved.
	 */
	char file_path[50];
	/**
	 * Specifies how to split files when writing measurement data to disc. See enum split_mode in enum.h for modes. 
	 */
	uint32_t file_split_mode;
};

#endif // STRUCT_H
