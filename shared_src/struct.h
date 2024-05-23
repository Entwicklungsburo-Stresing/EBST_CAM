#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>
#include "../shared_src/globals.h"

#define MAXPCIECARDS 5
#define MAXCAMCNT 8
#define MAX_NUMBER_OF_REGIONS 8
#define DACCOUNT 8
#define IOCTRL_OUTPUT_COUNT 8

// All settings are uint32_t to ensure the correct memory layout. This is important for the communication with LabVIEW.
// Don't change the order or you will have to change it for LabVIEW in InitMeasurement.vi.

// Individual settings for each PCIe board
struct camera_settings
{
	/**
	 * use_software_polling determines which method is used to copy data from DMA to user buffer. Further information about software polling can be found in the manual in chapter 5.4.8.
	 *		* >0: Use software polling. When there is new available data in the DMA buffer, a thread copies the data one scan at a time to the user buffer. Since P222_2 this method is reliable up to about 100kHz. It generates as expected a higher CPU load than the interrupt method. With this option you can get more recent scans from GetCurrentScanNumber(), especially at lower frequencies. For high frequencies > 30kHz this method is not recommended.
	 *		* =0: Use interrupt. Every dma_buffer_size_in_scans/2 scan the interrupt starts a copy process, which copies dma_buffer_size_in_scans/2 scans to the user buffer. 1000 is our default value for dma_buffer_size_in_scans, so interrupt is started every 500 scans.
	 */
	uint32_t use_software_polling;
	/**
	 * Scan trigger input mode determines the signal, on which one readout is started. See enum \ref sti_mode_t in enum_settings.h for options. Further information on trigger signals can be found in the manual in chapter 6.3.
	 */
	uint32_t sti_mode;
	/**
	 * Block trigger input mode determines the signal, on which one block of readouts is started. See enum \ref bti_mode_t in enum_settings.h  for options.  Further information on trigger signals can be found in the manual in chapter 6.3.
	 */
	uint32_t bti_mode;
	/**
	 * Scan timer in microseconds is the time between the start of two readouts. This time is used when sti mode is stimer. Stime is a 28 bit unsigned integer. Further information about the timer can be found in the manual in chapter 6.4.4.
	 *		* min: 1 µs
	 *		* step: 1 µs
	 *		* max: 268,435,455 µs = 268.435455 s
	 */
	uint32_t stime_in_microsec;
	/**
	 * Block timer in microseconds is the time between the start of two blocks of readouts. This time is used when bti mode is btimer. Btime is a 28 bit unsigned integer. Further information about the timer can be found in the manual in chapter 6.4.4.
	 *		* min: 1 µs
	 *		* step: 1 µs
	 *		* max: 268,435,455 µs = 268.435455 s
	 */
	uint32_t btime_in_microsec;
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
	 * Scan trigger slope determines whether positive, negative or both slopes of a trigger are used. See enum \ref sslope_t in enum_settings.h for options. This only applies to external triggers.
	 */
	uint32_t sslope;
	/**
	 * Block trigger slope determines whether positive, negative or both slopes of a trigger are used. See enum \ref bslope_t in enum_settings.h for options. This only applies to external triggers.
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
	 * Trigger mode of the integrator in the camera control box. See enum \ref trigger_mode_t in enum_settings.h for options. Further information about the trigger modes can found in the manual in chapter 7.3.7.	
	 */
	uint32_t trigger_mode_integrator;
	/**
	 * Sensor type should match the sensor type of your camera. See enum \ref sensor_type_t in enum_settings.h for options.
	 */
	uint32_t sensor_type;
	/**
	 * Camera system should match the model number of your camera. See enum \ref camera_system_t in enum_settings.h for options.
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
	 *		* max gain: 4
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
	 * DEPRECATED
	 * Shortrs controlled the sensor reset length. This setting is replaced by sensor_reset_length_in_4_ns.
	 *		* =0: long reset 800ns
	 *		* >0: short reset 380ns
	 */
	uint32_t shortrs;
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
	 * fft_mode controls the operating mode for FFT sensors. The vertical lines of a FFT sensor can either be summed up, read separately or summed up partially. See enum \ref fft_mode_t in enum_settings.h for options. Further information about FFT modes can be found in the manual in chapter 4.5.1.
	 */
	uint32_t fft_mode;
	/**
	 * lines_binning is the count of lines which are summed up in area mode for FFT sensors. When this is 1, every line is read out separately. When it is 2, every two lines are summed up in the sensor and read out as one line, so the count of samples for a complete readout gets divided by two. The same applies for higher values. lines_binning is a 12 bit unsigned integer. Further information about the area mode can be found in the manual in chapter 4.5.1.2.
	 *		* min: 1
	 *		* step: 1
	 *		* default: 1
	 *		* max: 4095
	 */
	uint32_t lines_binning;
	/**
	 * number_of_regions determines in how many regions the sensor gets divided in the FFT mode range of interest. Setting it to 1 would equal the area mode, so the minimum is 2. The size of each region is determined by \ref camera_settings.region_size. Unused regions must be set to 0. Further information about the range of interest mode can be found in the manual in chapter 4.5.1.3.
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
	 * region_size is the size of each region for the region of interest mode for FFT sensors. The sum of all active regions, which is defined by \ref number_of_regions, must equal \ref fft_lines. Inactive regions must be set to 0. region_size is a 32 bit unsigned integer array with the size of 8 but only 8 bit of each element are used. Further information about the range of interest mode can be found in the manual in chapter 4.5.1.3. This is an example for a region_size setting with fft_lines = 70 and number_of_regions = 3. Using this example the sensor will be read out 3 times. The first and the third read out contain the summed up intensity of the upper and the lower 4 lines. The second read out contains the intensity of the summed up 64 lines in between.
	 *		* regions_size[0] = 4
	 *		* regions_size[1] = 64
	 *		* regions_size[2] = 4
	 *		* regions_size[3] = 0
	 *		* regions_size[4] = 0
	 */
	uint32_t region_size[MAX_NUMBER_OF_REGIONS];
	/**
	 * Array for output levels of each digital to analog converter
	 */
	uint32_t dac_output[MAXCAMCNT][DACCOUNT];
	/**
	 * Output mode for PCIe board output pin. See enum \ref tor_out_t in enum_settings.h for options.
	 */
	uint32_t tor;
	/**
	 * ADC operating mode. Only available for specific ADCs, e.g. in camera system 3030. See enum \ref adc_mode_t in enum_settings.h for options.
	 */
	uint32_t adc_mode;
	/**
	 * Fixed value for ADC output when ADC mode is custom pattern.
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
	 * DEPRECATED
	 * This should be set when the camera is a high speed infrared camera. This information was moved to sensor_type.
	 *		* =0 no IR
	 *		* >0 IR
	 */
	uint32_t is_hs_ir;
	/**
	 * IOCTRL impact start pixel is the position in the pixel array where the information of voltage or integrator inputs are written. The number of these inputs can differ, so length of these additional information can differ, too. The setting specifies the first pixel where the information is written, so the information can be read from this one and the following pixels. IOCTRL impact start pixel is a 16 bit unsigned integer. Further information about IOCTRL impact start pixel can be found in the manual in chapter 7.3.
	 *		* min: 0
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
	 * Size of DMA buffer in scans. 1000 is our default. 60 is also working with high speed (exposure time = 0,02ms). 30 could be with one wrong scan every 10000 scans.
	 */
	uint32_t dma_buffer_size_in_scans;
	/**
	 * Trigger output counter determines how many XCK are skipped until the output TO_CNT_OUT shows the XCK signal. Use \ref tor_out_t.tor_to_cnt_out for the setting \ref camera_settings.tor to see TO_CNT_OUT at the output of the PCIe board. Example: tocnt = 2 => skip every first and second XCK, show XCK on the PCIe output on every third XCK. Only the lowest 7 bits are used for this setting.
	 */
	uint32_t tocnt;
	/**
	 * Trigger output counter determines how many trigger inputs are skipped before the next measurement is triggered. Every ticnt+1 trigger input the measurement is triggered according to sti_mode. Only the lowest 7 bits are used for this setting.
	 */
	uint32_t ticnt;
	/**
	 * Sensor_reset_length_in_4_ns controls the length of the reset pulse between two camera readouts for some sensors. This reset can be used, to completely clear the sensor, which is not always the case without this reset for all sensors. Sensor_rese_length_in_4_ns is a 16 bit unsigned integer. Further information about sensor reset can be found in the manual in chapter 4.9.2.
	 *		* min: 0 ns
	 *		* step: 1 * 4 ns = 4 ns
	 *		* default: 100 * 4 ns = 400 ns
	 *		* max: 65535 * 4 ns = 262,140 ns
	 */
	uint32_t sensor_reset_length_in_4_ns;
	/**
	 * Experimental:
	 *		* =0: Don't write measurement data to disc.
	 *		* >0: Write measurement data to disc.
	 */
	uint32_t write_to_disc;
	/**
	 * File path is specifying the path where the measurement data is saved.
	 */
	char file_path[file_path_size];
	/**
	 * DEPRECATED
	 * Not used, because didn't work correctly. Specifies how to split files when writing measurement data to disc. See enum \ref split_mode_t in enum.h for modes. 
	 */
	uint32_t file_split_mode;
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
	 * bnc_out is the output mode for the XCK output of the Camera Control box. See enum \ref bnc_out_t in enum_settings.h for options. Furhter information about bnc out can be found in the manual in chapter 3.3.1.
	 */
	uint32_t bnc_out;
};

// In this struct are settings, that are the same for all PCIe boards.
struct measurement_settings
{
	/**
	 * Select boards with bits. 1 for using this board, 0 for not using this board.
	 *
	 * - bit 0: board 0
	 * - bit 1: board 1
	 * - bit 2: board 2
	 * - bit 3: board 3
	 * - bit 4: board 4
	 */
	uint32_t board_sel;
	/**
	 * Number of samples. 32 bit. Min: 2, max: max of uint32
	 */
	uint32_t nos;
	/**
	 * Number of blocks. 32 bit. Min: 1, max: max of uint32
	 */
	uint32_t nob;
	/**
	 * Continuous mode switch. The continuous mode repeats automatically the measurement cycle until it is stopped. One cycle consists of number of samples * number of blocks readouts. The data is not stored permanently. Each cycle is overwriting the data from the previous cycle. The data of a specific sample/block is always at the same memory address. That means for example scan 100 in block 2 from the first measurement cycle will be overwritten by scan 100 in block 2 in the second measurement cycle. The time gap between two cycles is done softwarewise and is beeing controlled by the parameter cont_pause_in_microseconds. So the start of the next cycle is not strictly linked to your trigger, which means when triggering fast, triggers could be missed.
	 *	- >0 on
	 *	- =0 off
	 */
	uint32_t contiuous_measurement;
	/**
	 * Pause between two measurement cycles when continuous mode is on. See description of the parameter contiuous_measurement for more information about the continuous mode.
	 */
	uint32_t cont_pause_in_microseconds;
	/**
	 * This is an array of structs for individual settings for each PCIe board.
	 */
	struct camera_settings camera_settings[MAXPCIECARDS];
};

// In this struct are settings, that are the same for all PCIe boards.
struct measurement_settings_matlab
{
	/**
	 * \copydoc measurement_settings.board_sel
	 */
	uint32_t board_sel;
	/**
	 * \copydoc measurement_settings.nos
	 */
	uint32_t nos;
	/**
	 * \copydoc measurement_settings.nob
	 */
	uint32_t nob;
	/**
	 * \copydoc measurement_settings.contiuous_measurement
	 */
	uint32_t contiuous_measurement;
	/**
	 * \copydoc measurement_settings.cont_pause_in_microseconds
	 */
	uint32_t cont_pause_in_microseconds;
};

#endif // STRUCT_H
