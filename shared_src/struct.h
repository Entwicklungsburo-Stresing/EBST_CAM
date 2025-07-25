﻿/*****************************************************************//**
 * @file		struct.h
 * @brief		Settings struct for the ESLSCDLL API.
 * @author		Florian Hahn
 * @date		19.03.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <stdint.h>
#include "enum_settings.h"

/**
 * @brief Maximum number of PCIe cards.
 * 
 * If your mainboard allows you could us up to 5 PCIe cards and measure with all of them simultaneously.
 */
#define MAXPCIECARDS 5
/**
 * @brief Maximum number of cameras per PCIe card.
 * 
 * Multiple cameras can be connected in line and measure simultaneously.
 */
#define MAXCAMCNT 8
/**
 * @brief Maximum for the setting @ref camera_settings.number_of_regions.
 * 
 * The actual maximum is 5, but we use 8 to ensure the correct memory layout.
 */
#define MAX_NUMBER_OF_REGIONS 8
/**
 * @brief Number of DAC channels in the high speed camera.
 */
#define DACCOUNT 8
/**
 * @brief Number of output channels in the IOCTRL.
 */
#define IOCTRL_OUTPUT_COUNT 8

// All settings are uint32_t to ensure the correct memory layout. This is important for the communication with LabVIEW.
// Don't change the order or you will have to change it for LabVIEW in InitMeasurement.vi.

/**
 * @brief Individual settings for each PCIe board.
 */
struct camera_settings
{
	/**
	 * use_software_polling determines which method is used to copy data from DMA to user buffer.
	 *		* >0: Use software polling. When there is new available data in the DMA buffer, a thread copies the data one scan at a time to the user buffer. Since the PCIe card firmware version P222_2 this method is reliable up to about 100kHz. It generates as expected a higher CPU load than the interrupt method. With this option you can get more recent scans from @ref DLLGetCurrentScanNumber, especially at lower frequencies.
	 *		* =0: Use interrupt. Every @ref camera_settings.dma_buffer_size_in_scans /2 scan the interrupt starts a copy process, which copies dma_buffer_size_in_scans/2 scans to the user buffer. 1000 is our default value for @ref camera_settings.dma_buffer_size_in_scans, so the interrupt is triggered every 500 scans.
	 * 
	 * Further information about software polling can be found in the manual in chapter 5.4.8.
	 */
	uint32_t use_software_polling;
	/**
	 * Scan trigger input mode determines the signal, on which one readout is started. See enum @ref sti_mode_t in enum_settings.h for options. Further information on trigger signals can be found in the manual in chapter 6.3.
	 */
	uint32_t sti_mode;
	/**
	 * Block trigger input mode determines the signal, on which one block of readouts is started. See enum @ref bti_mode_t in enum_settings.h  for options.  Further information on trigger signals can be found in the manual in chapter 6.3.
	 */
	uint32_t bti_mode;
	/**
	 * Stime is the time between the start of two readouts. This time is used when @ref camera_settings.sti_mode is set to @ref sti_mode_t.sti_STimer. The resolution of this timer depends on the setting @ref camera_settings.timer_resolution_mode. Stime is a 28 bit unsigned integer. Further information about the timer can be found in the manual in chapter 6.4.4.
	 */
	uint32_t stime;
	/**
	 * Block time is the time between the start of two blocks of readouts. This time is used when \ref camera_settings.bti_mode is set to \ref  bti_mode_t.bti_BTimer. The resolution of this timer depends on the setting @ref camera_settings.timer_resolution_mode. Btime is a 28 bit unsigned integer. Further information about the timer can be found in the manual in chapter 6.4.4.
	 */
	uint32_t btime;
	/**
	 * Scan delay after trigger in 10 ns steps is the delay time between the trigger starting a scan, which is determined by sti_mode and the actual start of the scan. SDAT is a 31 bit unsigned integer. Further information about sdat can be found in the manual in chapter 2.9.2 and 6.2.4.10.
	 *		* disable: 0
	 *		* min: 1 * 10 ns = 10 ns
	 *		* step: 10 ns
	 *		* max: 2,147,483,647 * 10 ns = 21,474,836,470 ns = 21.474836470 s
	 */
	uint32_t sdat_in_10ns;
	/**
	 * Block delay after trigger in 10 ns steps. This is the delay between the trigger starting a block, which is determined by bti_mode and the actual start of the block. BDAT is a 31 bit unsigned integer. Further information about bdat can be found in the manual in chapter 2.9.2 and 6.2.6.2.
	 *		* disable: 0
	 *		* min: 1 * 10 ns = 10 ns
	 *		* step: 10 ns
	 *		* max: 2,147,483,647 * 10 ns = 21,474,836,470 ns = 21.474836470 s
	 */
	uint32_t bdat_in_10ns;
	/**
	 * Scan trigger slope determines whether positive, negative or both slopes of a trigger are used. See enum @ref sslope_t in enum_settings.h for options. This only applies to external triggers.
	 */
	uint32_t sslope;
	/**
	 * Block trigger slope determines whether positive, negative or both slopes of a trigger are used. See enum @ref bslope_t in enum_settings.h for options. This only applies to external triggers.
	 */
	uint32_t bslope;
	/**
	 * LEGACY
	 * XCK delay in 10 ns steps was the time between the high slope of XCK and the actual start of the camera read out. This is done by delaying VON relative to XCK. Since P222_09 cameras are not designed to interact with the VON signal anymore. For the following camera versions xckdelay can be used:
	 *		* 205.X
	 *		* 215.6 and older
	 *		* 206.X
	 *		* 216.X
	 * For newer camera versions VCLKs are generated inside the camera and the delay between integrator and XCK can be achieved by using the analog delay of the camera control. Further information about xckdelay can be found in the manual in chapter 6.2.5.12.
	 * 31 bit. xckdelay = 500ns + xckdelay_in_10ns * 10ns
	 *		* disable: 0
	 *		* min 1: 500 ns + 1 * 10ns = 510 ns
	 *		* max 2,147,483,647: 500 ns + 2,147,483,647 * 10 ns = 21,474,836,970 ns = 21.474836970 s
	 */
	uint32_t xckdelay_in_10ns;
	/**
	 * Scan exposure control in 10 ns steps adds a delay time between the trigger and the start of the camera read out (start of XCK). Some sensors use this time for a electronic exposure control. Also mechanical shutters can be controlled with this setting. The SEC time is added between the end of SDAT and the start of the camera read out. SEC is a 32 bit unsigned integer. Further information about SEC can be found in the manual in chapter 4.9.1 and 6.2.4.11.
	 *		* disable: 0
	 *		* min: 1 * 10 ns = 10 ns
	 *		* step: 10 ns
	 *		* max: 4,294,967,295 * 10 ns = 42,949,672,950 ns
	 */
	uint32_t sec_in_10ns;
	/**
	 * Trigger mode of the integrator in the camera control box. See enum @ref trigger_mode_t in enum_settings.h for options. Further information about the trigger modes can found in the manual in chapter 7.3.7.	
	 */
	uint32_t trigger_mode_integrator;
	/**
	 * Sensor type should match the sensor type of your camera. See enum @ref sensor_type_t in enum_settings.h for options.
	 */
	uint32_t sensor_type;
	/**
	 * Camera system should match the model number of your camera. See enum @ref camera_system_t in enum_settings.h for options.
	 */
	uint32_t camera_system;
	/**
	 * Camcnt is the number of cameras which are connected to one PCIe board. This could be multiple cameras connected via a chain to one PCIe board or multiple channel on one camera control box. 0 is a valid input for operating a PCIe card without any camera connected and only use its special inputs. Camcnt is a 4 bit unsigned integer.
	 *		* Min: 0
	 *		* Max: 15
	 */
	uint32_t camcnt;
	/**
	 * Pixel is the number of pixels in one sensor. Only 64*n are allowed. Pixel is a 16 bit unsigned integer. Typical values are: 576, 1024, 1088, 2112.
	 *		* min: 64
	 *		* step: 64
	 *		* max: 8256
	 */
	uint32_t pixel;
	/**
	 * Is fft legacy is a special legacy mode for operation with older FFT cameras. For the following camera versions fft legacy should be turned on:
	 *		* 205.X
	 *		* 215.6 and older
	 *		* 208.X
	 *		* 218.3 and older
	 *		* 210.X
	 *		* 211.2 and older
	 *		* 206.X
	 *		* 216.X
	 * If the PCIe card version is 222.8 or older or 202.X, fft legacy should always be turned on, if you are using a FFT sensor. FFT legacy is available since 222.10.
	 *		* =0: off (default)
	 *		* >0: fft legacy mode on
	 */
	uint32_t is_fft_legacy;
	/**
	 * LED off turns the LEDs in the camera off.
	 *		* =0 LED on
	 *		* >0 LED off
	 */
	uint32_t led_off;
	/**
	 * Sensor gain is controlling the internal gain function of some infrared sensors. For the some sensors gain can be switched on or off, others sensor got multiple levels of gain. Further information about sensor gain can be found in the manual in chapters 3.3.1, 3.4.1.1, 4.5.2, 8.4.4, 8.7.3.
	 *	* camera system 3001/3010:
	 *		* gain off: 0
	 *		* gain on: 1
	 *	* camera system 3030:
	 *		* gain off: 0
	 *		* step: 1
	 *		* max gain: 3
	 */
	uint32_t sensor_gain;
	/**
	 * ADC gain is controlling the gain function of the ADC in 3030 high speed cameras. ADC gain is a 8 bit unsigned integer. Further information about adc_gain can be found in the manual in chapter 3.4.2.1. 
	 *		* min: 0
	 *		* step: 1
	 *		* default: 6
	 *		* max: 12
	 */
	uint32_t adc_gain;
	/**
	 * Temperature level is controlling the target temperature for the temperature regulation in cooled cameras. The cooling is done by a Peltier element which can generate a temperature difference of 40 °C. That means the target temperature can only be reached, if the ambient temperature is not higher than target + 40 °C. The regulation is optimized for -20 °C to 0 °C. The precision is 1 K at these levels. On other levels the error can rise up to 10 K. If the target temperature is reached, the LED TG (Temperature Good) will light up green. temp_level is a 3 bit unsigned integer. Further information about temp_level can be found in the manual in chapter 3.5.
	 *		* 0: cooling off
	 *		* 1: 0 °C
	 *		* 2: -10 °C
	 *		* 3: -20 °C
	 *		* 4: -25 °C
	 *		* 5: -30 °C
	 *		* 6: -40 °C
	 *		* 7: min °C
	 */
	uint32_t temp_level;
	/**
	 * Block trigger input counter determines how many block trigger inputs are skipped before the next block start is triggered. Every bticnt+1 trigger input the next block is triggered according to @ref camera_settings.bti_mode. bticnt is a 7 bit unsigned integer. Introduced in PCIe board version 222.12.
	 *		* min: 0
	 *		* step: 1
	 *		* max: 127
	 */
	uint32_t bticnt;
	/**
	 * GPX offset is controlling the output offset of the TDC-GPX IC on the TDC add on board for the PCIe card. The TDC-GPX IC can be used to measure the time delay between one start and two stop signals in picosecond resolution. Adding an offset can improve the accuracy of the TDC-GPX IC. GPX offset is a 18 bit unsigned integer. Further information about gpx_offset can be found in the manual in chapter 10.2. 
	 *		* min: 0
	 *		* step: 1
	 *		* default: 1000
	 *		* max: 262,143
	 */
	uint32_t gpx_offset;
	/**
	 * fft_lines is the count of vertical lines for FFT sensors. This setting should match your sensor. You can find this information in the manual. fft_lines is a 12 bit unsigned integer. Further information about FFT sensors can be found in the manual in chapters 1.4 and 11.3.3.
	 *		* min: 1
	 *		* step: 1
	 *		* default: 64
	 *		* max: 4095
	 */
	uint32_t fft_lines;
	/**
	 * vfreq controls the vertical clock frequency for FFT sensors. Different sensors are capable of different vertical clock speeds, so this setting should match your sensor. You can find this information in the manual. vfreq is the period of the vertical clock, so a higher vfreq means a lower frequency. vfreq is 8 bit unsigned integer. Further information about vfreq and FFT sensors can be found in the manual in chapters 1.4, 3.3.5 and 11.3.3.
	 *		* min: 1 * 256 ns = 256 ns => 3.9 MHz
	 *		* step: 1 * 256 ns = 256 ns
	 *		* max: 255 * 256 ns = 65,280 ns => 15.3 kHz
	 */
	uint32_t vfreq;
	/**
	 * fft_mode controls the operating mode for FFT sensors. The vertical lines of a FFT sensor can either be summed up, read separately or summed up partially. See enum @ref fft_mode_t in enum_settings.h for options. Further information about FFT modes can be found in the manual in chapter 4.5.1.
	 */
	uint32_t fft_mode;
	/**
	 * DEPRECATED
	 * lines_binning is the count of lines which are summed up in area mode for FFT sensors. When this is 1, every line is read out separately. When it is 2, every two lines are summed up in the sensor and read out as one line, so the count of samples for a complete readout gets divided by two. The same applies for higher values. lines_binning is a 12 bit unsigned integer. Further information about the area mode can be found in the manual in chapter 4.5.1.2.
	 *		* min: 1
	 *		* step: 1
	 *		* default: 1
	 *		* max: 4095
	 */
	uint32_t lines_binning;
	/**
	 * number_of_regions determines in how many regions the sensor gets divided when @ref camera_settings.fft_mode is set to @ref fft_mode_t.partial_binning. Setting it to 1 would equal the area mode, so the minimum is 2. The size of each region is determined by @ref camera_settings.region_size. Unused regions must be set to 0. Further information about the range of interest mode can be found in the manual in chapter 4.5.1.3.
	 *		* min: 2
	 *		* step: 1
	 *		* max: 5
	 */
	uint32_t number_of_regions;
	/**
	 * S1 S2 read delay in 10 ns controls the delay between the trigger and the moment, when the status of the S1 and S2 inputs are read. When the delay exceeds XCK, S1 and S2 are never read. S1 S2 read delay is a 32 bit unsigned integer. Further information information about S1 S2 read delay can be found in the manual in chapter 6.2.5.13.
	 *		* min: 0 ns
	 *		* step: 10 ns
	 *		* max: 4,294,967,295 * 10 ns = 42,949,672,950 ns
	 */
	uint32_t s1s2_read_delay_in_10ns;
	/**
	 * region_size is the size of each region for the region of interest mode for FFT sensors. The sum of all active regions, which is defined by @ref number_of_regions, must equal @ref fft_lines. Inactive regions must be set to 0. region_size is a 32 bit unsigned integer array with the size of 8 but only 8 bit of each element are used. Further information about the range of interest mode can be found in the manual in chapter 4.5.1.3. This is an example for a region_size setting with fft_lines = 70 and number_of_regions = 3. Using this example the sensor will be read out 3 times. The first and the third read out contain the summed up intensity of the upper and the lower 4 lines. The second read out contains the intensity of the summed up 64 lines in between.
	 *		* region_size[0] = 4
	 *		* region_size[1] = 64
	 *		* region_size[2] = 4
	 *		* region_size[3] = 0
	 *		* region_size[4] = 0
	 */
	uint32_t region_size[MAX_NUMBER_OF_REGIONS];
	/**
	 * Array for output levels of each digital to analog converter
	 */
	uint32_t dac_output[MAXCAMCNT][DACCOUNT];
	/**
	 * Output mode for PCIe board output pin. See enum @ref tor_out_t in enum_settings.h for options. Further information about tor can be found in the manual in chapter 6.2.4.12.
	 */
	uint32_t tor;
	/**
	 * ADC mode controls the operating mode of the ADC. This option is intended for debugging purpose and only available for specific ADCs, e.g. in camera system 3030. See enum @ref adc_mode_t in enum_settings.h for options.
	 */
	uint32_t adc_mode;
	/**
	 * Adc custom pattern is the constant output value of all 8 ADC channels if @ref camera_settings.adc_mode is set to @ref adc_mode_t.custom_pattern. This is a 14 bit unsigned integer.
	 *		* min: 0
	 *		* step: 1
	 *		* max: 16383
	 */
	uint32_t adc_custom_pattern;
	/**
	 * Block exposure control in 10 ns steps adds a delay time between the block trigger and the start start of the block. Mechanical shutters can be controlled with this setting. The BEC time is added between the end of BDAT and the start of the block. BEC is a 32 bit unsigned integer. Further information about BEC can be found in the manual in chapter 4.9.1 and 6.2.6.3.
	 *		* disable: 0
	 *		* min: 1 * 10 ns = 10 ns
	 *		* step: 10 ns
	 *		* max: 4,294,967,295 * 10 ns = 42,949,672,950 ns
	 */
	uint32_t bec_in_10ns;
	/**
	 * Channel select controls which channel of a camera control box is used for the camera readout. This feature is implemented in the camera version P230_6 and newer. See enum @ref channel_select_t in enum_settings.h for options.
	 */
	uint32_t channel_select;
	/**
	 * IOCTRL impact start pixel is the position in the pixel array where the information of voltage or integrator inputs are written. The number of these inputs can differ, so length of these additional information can differ, too. The setting specifies the first pixel where the information is written, so the information can be read from this one and the following pixels. IOCTRL impact start pixel is a 16 bit unsigned integer. Further information about IOCTRL impact start pixel can be found in the manual in chapter 7.3.
	 *		* min: 23
	 *		* step: 1
	 *		* default: 1078
	 *		* max: 65535
	 */
	uint32_t ioctrl_impact_start_pixel;
	/**
	 * This is an array, which sets the width of the IOCTRL outputs in 5ns steps.
	 */
	uint32_t ioctrl_output_width_in_5ns[IOCTRL_OUTPUT_COUNT];
	/**
	 * This is an array, which sets the delay of the IOCTRL outputs in 5ns steps.
	 */
	uint32_t ioctrl_output_delay_in_5ns[IOCTRL_OUTPUT_COUNT];
	/**
	 * Determines the base frequency T0 of the IOCTRL pulse generator in 10ns steps.
	 */
	uint32_t ioctrl_T0_period_in_10ns;
	/**
	 * Size of the DMA buffer in scans. The default is 1000. This setting controls how often the interrupt is triggered to copy data from the DMA buffer to the user buffer. This setting only has an effect when @ref camera_settings.use_software_polling is turned off. A lower number means more interrupts in a shorter time and so more recent data available. Which data is available is indicated by @ref DLLGetCurrentScanNumber. Too many interrupts in a too short time can lead to errors. 60 is working with high speed (exposure time = 0,02ms). When this setting is 30, there could a wrong scan every 10000 scans.
	 */
	uint32_t dma_buffer_size_in_scans;
	/**
	 * Trigger output counter determines how many XCK are skipped until the output TO_CNT_OUT shows the XCK signal. Use @ref tor_out_t.tor_to_cnt_out for the setting @ref camera_settings.tor to see TO_CNT_OUT at the output of the PCIe board. Example: tocnt = 2 => skip every first and second XCK, show XCK on the PCIe output on every third XCK. tocnt is a 7 bit unsigned integer. Further information about tocnt can be found in the manual in chapter 6.2.4.12.
	 *		* min: 0 (TO_CNT_OUT = XCK)
	 *		* step: 1
	 *		* max: 127
	 */
	uint32_t tocnt;
	/**
	 * Scan trigger input counter determines how many scan trigger inputs are skipped before the next measurement is triggered. Every sticnt+1 trigger input the measurement is triggered according to @ref camera_settings.sti_mode. sticnt is a 7 bit unsigned integer. Further information about sticnt can be found in the manual in chapter 6.2.4.12.
	 *		* min: 0
	 *		* step: 1
	 *		* max: 127
	 */
	uint32_t sticnt;
	/**
	 * sensor_reset_or_hsir_ec either controls the length of the reset pulse between two camera readouts or the exposure time of the high speed infrared sensor. The purpose of this setting depends on @ref camera_settings.sensor_type. sensor_reset_or_hsir_ec is a 16 bit unsigned integer.
	 * 
	 * Sensor reset for HSVIS:
	 * This reset can be used, to completely clear the sensor. Further information about sensor reset can be found in the manual in chapter 4.9.2.
	 *		* min: 0 ns
	 *		* step: 1 * 4 ns = 4 ns
	 *		* default: 100 * 4 ns = 400 ns
	 *		* max: 65535 * 4 ns = 262,140 ns
	 * 
	 * Exposure control for HSIR:
	 * When sensor_type is HSIR this setting controls the exposure time of the sensor. The exposure time cannot be shorter than the minimum. If the value is smaller than the minimum the exposure time will be the minumum. If the value exceeds the repetition rate of the measurement, there will be samples with a zero line.
	 *		* min: 134 * 160 ns = 21,440 ns
	 *		* step: 1 * 160 ns = 160 ns
	 *		* default: 140 * 160 ns = 22,400 ns
	 *		* max: 65535 * 160 ns = 10,485,600 ns
	 */
	uint32_t sensor_reset_or_hsir_ec;
	/**
	 * Write to disc is an experimental feature for writing the measurement data on the fly to the disc. The data format is binary. It is the same data layout as the data is stored in RAM during the measurement. Additionally there is a file header at the beginning of the file. The path to the target file is given by @ref camera_settings.file_path. This feature is only available on Windows. In most cases the resulting file should be correct, but data layout errors has been observed. This is the reason why the feature is marked as experimental.
	 *		* =0: Don't write measurement data to disc.
	 *		* >0: Write measurement data to disc.
	 */
	uint32_t write_to_disc;
	/**
	 * File path is specifying the path where the measurement data is saved, when @ref camera_settings.write_to_disc is activated. File path is a char array with the size 256, so the maximum path length is 256 characters.
	 *		* example value: C:/Users/XY/
	 */
	char file_path[file_path_size];
	/**
	 * When shift S1S2 to next scan is on, the input states of S1 and S2, which are sampled in scan n, are displayed in pixel 2 in scan n+1. This option is useful for the sensor type HSIR, because this camera displays the data sampled at trigger n in scan n+1. To match the actual states of S1 and S2 to the sensor data activate this option. This feature is supported since PCIe board version P222_17.
	 *		* =0: off
	 *		* =1: on
	 */
	uint32_t shift_s1s2_to_next_scan;
	/**
	 * Is cooled camera legacy is a special mode for operating older cooled cameras. If on, a bit in the PCIe board is set to react correctly to the cooled status messages from the camera. The following camera versions need to be run in legacy mode:
	 *		* 208.X
	 *		* 218.1 and older
	 * 
	 *		* =0 off
	 *		* >0 cooled camera legacy mode on
	 */
	uint32_t is_cooled_camera_legacy_mode;
	/**
	 * monitor is the output mode for the monitor output of the camera. See enum @ref monitor_t in enum_settings.h for options. Further information about monitor can be found in the manual in chapter 3.3.1.
	 */
	uint32_t monitor;
	/**
	 * With manipulate_data_mode you can activate / deactivate a built in data manipulation during the measurement. This is potentially used to linearize the sensor data for specific sensors. See @ref manipulate_data_mode_t in enum_settings.h for options.
	 */
	uint32_t manipulate_data_mode;
	/**
	 * manipulate_data_custom_factor is used when @ref camera_settings.manipulate_data_mode is set to @ref manipulate_data_mode_t.manipulate_data_mode_custom_factor. This factor is multiplied with the data of each pixel.
	 */
	double manipulate_data_custom_factor;
	/**
	 * ec_legacy_mode is a special mode for operating older high speed cameras, camera system 3030. The following camera versions need to be run in legacy mode:
	 *		* 209.12 and older
	 * 
	 *		* = 0 off
	 *		* > 0 ec legacy mode on
	 */
	uint32_t ec_legacy_mode;
	/**
	 * timer_resolution_mode determines the resolution of the timer controlled by @ref camera_settings.stime and @ref camera_settings.btime. See @ref timer_resolution_t in enum_settings.h for options.
	 */
	uint32_t timer_resolution_mode;
};

/**
 * @brief In this struct are settings, that are the same for all PCIe boards.
 */
struct measurement_settings
{
	/**
	 * board_sel controls which boards are used for the measurement. When multiple boards are selected the measurement is started for all boards at the same time. The exact trigger moment is controlled separately by each board. Every board has its own camera settings, which are set in the struct member @ref measurement_settings.camera_settings. Select between 1 and 5 boards. This variable works bitwise. Bit 0 controls board 0: 1 for using this board, 0 for not using this board.
	 *		* bit 0: board 0
	 *		* bit 1: board 1
	 *		* bit 2: board 2
	 *		* bit 3: board 3
	 *		* bit 4: board 4
	 */
	uint32_t board_sel;
	/**
	 * nos is the number of samples. One sample is one readout of the camera. One readout is triggered on each sample trigger which is controlled by @ref camera_settings.sti_mode. nos is a 32 bit unsigned integer. Further information about samples and blocks can be found in the manual in chapter 6.4.1.
	 *		* min: 2
	 *		* step: 1
	 *		* max: 4,294,967,295
	 */
	uint32_t nos;
	/**
	 * nob is the number of blocks. One block contains nos readouts and is triggered on each block trigger which is controlled by @ref camera_settings.bti_mode. nob is a 32 bit unsigned integer. Further information about samples and blocks can be found in the manual in chapter 6.4.1.
	 *		* min: 1
	 *		* step: 1
	 *		* max: 4,294,967,295
	 */
	uint32_t nob;
	/**
	 * Continuous mode switch. The continuous mode repeats automatically the measurement cycle until it is stopped. One cycle consists of number of samples * number of blocks readouts. The data is not stored permanently. Each cycle is overwriting the data from the previous cycle. The data of a specific sample/block is always at the same memory address. That means for example scan 100 in block 2 from the first measurement cycle will be overwritten by scan 100 in block 2 in the second measurement cycle. The time gap between two cycles is done software wise and is beeing controlled by the parameter @ref measurement_settings.cont_pause_in_microseconds. So the start of the next cycle is not strictly linked to your trigger, which means when triggering fast, triggers could be missed.
	 *		* >0 on
	 *		* =0 off
	 */
	uint32_t continuous_measurement;
	/**
	 * cont_pause_in_microseconds is the pause between two measurement cycles when continuous mode is on. See description of the parameter continuous_measurement for more information about the continuous mode. cont_pause_in_microseconds is a 32 bit unsigned integer.
	 *		* min: 0 µs
	 *		* step: 1 µs
	 *		* max: 4,294,967,295 µs
	 */
	uint32_t cont_pause_in_microseconds;
	/**
	 * This is an array of structs for individual settings for each PCIe board.
	 */
	struct camera_settings camera_settings[MAXPCIECARDS];
};

/**
 * @brief In this struct are settings, that are the same for all PCIe boards.
 * 
 * It is the same as @ref measurement_settings but doesn't contain @ref camera_settings. This is for compatibility reasons for Matlab, because structs in Matlab are not able to contain structs.
 */
struct measurement_settings_matlab
{
	/**
	 * @copydoc measurement_settings.board_sel
	 */
	uint32_t board_sel;
	/**
	 * @copydoc measurement_settings.nos
	 */
	uint32_t nos;
	/**
	 * @copydoc measurement_settings.nob
	 */
	uint32_t nob;
	/**
	 * @copydoc measurement_settings.continuous_measurement
	 */
	uint32_t continuous_measurement;
	/**
	 * @copydoc measurement_settings.cont_pause_in_microseconds
	 */
	uint32_t cont_pause_in_microseconds;
};

struct file_header
{
	uint32_t software_version_major;
	uint32_t software_version_pcie;
	uint32_t software_version_minor;
	uint32_t number_of_boards;
	uint32_t board_sel;
	uint32_t drvno;
	uint32_t pixel;
	uint32_t nos;
	uint32_t nob;
	uint32_t camcnt;
	uint64_t measurement_cnt;
	char timestamp[file_timestamp_size];
};

struct special_pixels
{
	uint32_t overTemp;
	uint32_t tempGood;
	uint32_t blockIndex;
	uint32_t scanIndex;
	uint32_t scanIndex2;
	uint32_t s1State;
	uint32_t s2State;
	uint32_t impactSignal1;
	uint32_t impactSignal2;
	uint32_t cameraSystem3001;
	uint32_t cameraSystem3010;
	uint32_t cameraSystem3030;
	uint32_t fpgaVerMajor;
	uint32_t fpgaVerMinor;
};

struct verify_data_parameter
{
	/**
	 * Path and filename to the file.
	 */
	char filename_full[file_filename_full_size];
	/**
	 * Count of samples found in the file.
	 */
	uint32_t sample_cnt;
	/**
	 * Count of blocks found in the file.
	 */
	uint32_t block_cnt;
	/**
	 * Count of measurements found in the file.
	 */
	uint64_t measurement_cnt;
	/**
	 * File header of the file.
	 */
	struct file_header fh;
	/**
	 * Counted errors, while checking the sample and block counter bits in the data. When error_cnt is 0, the data is perfectly as expected.
	 */
	uint32_t error_cnt;
	/**
	 * Counter of last read sample in file
	 */
	uint32_t last_sample;
	/**
	 * Counter of last read block in file
	 */
	uint32_t last_block;
	uint32_t last_sample_before_error;
	uint32_t last_block_before_error;
	uint64_t last_measurement_before_error;
};
