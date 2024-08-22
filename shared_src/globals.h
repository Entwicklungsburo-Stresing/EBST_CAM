#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum file_specifications_t
{
	file_path_size = 256,
	file_timestamp_size = 64,
	file_filename_full_size = 256
};

#include "struct.h"
#include "enum_settings.h"
#include "enum_hardware.h"

#define MANUAL_OVERRIDE_TLP false
#define LEGACY_202_14_TLPCNT false
#define FORCETLPS128 true	//only use payload size 128byte
#define DMA_64BIT_EN false
#define S0_SPACE_OFFSET 0x80
#define HWDREQ_EN true // enables hardware start of DMA by XCK h->l slope
#define DMA_BUFFER_PARTS 2
/**
 * @brief DMA_CONTIGBUF: DMA buffer type switch.
 * 
 * true: DMA buffer is set by driver (data must be copied afterwards to user space).
 * false: DMA buffer is set by application (pointer must be passed to SetupPCIE_DMA).
 */
#define DMA_CONTIGBUF true

struct file_header
{
	uint32_t drvno;
	uint32_t pixel;
	uint32_t nos;
	uint32_t nob;
	uint32_t camcnt;
	uint64_t measurement_cnt;
	char timestamp[file_timestamp_size];
	char filename_full[file_filename_full_size];
	uint32_t split_mode;
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

extern uint32_t* aCAMCNT;
extern uint16_t** userBuffer;
extern uint16_t** userBufferEndPtr;
extern uint16_t** userBufferWritePos;
extern uint16_t** userBufferWritePos_last;
extern uint8_t number_of_boards;
extern volatile bool abortMeasurementFlag;
extern volatile uint32_t* numberOfInterrupts;
extern volatile bool continuousMeasurementFlag;
extern uint32_t continuousPauseInMicroseconds;
extern struct measurement_settings settings_struct;
extern const struct camera_settings camera_settings_default;
extern bool isRunning;
extern bool wasRunning;
extern int64_t* scanCounterTotal;
extern uint64_t measurement_cnt;
extern char start_timestamp[file_timestamp_size];
extern volatile size_t* data_available;
extern volatile bool* timerOn;
extern volatile bool* allInterruptsDone;

#ifdef __cplusplus
}
#endif
#endif // GLOBALS_H
