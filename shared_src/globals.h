#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "struct.h"
#include "enum.h"

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

struct global_vars
{
	uint16_t** userBuffer;
	void* hDev;
	uint32_t* aPIXEL;
	uint32_t* aCAMCNT;
	uint32_t* Nospb;
	uint32_t* Nob;
	bool* useSWTrig;
};

struct file_specs
{
	uint32_t drvno;
	uint32_t measurement_cnt;
	uint32_t block_cnt;
	char path[file_path_size];
	uint32_t split_mode;
	char timestamp[file_timestamp_size];
};

struct file_header
{
	uint32_t drvno;
	uint32_t pixel;
	uint32_t nos;
	uint32_t nob;
	uint32_t camcnt;
	uint32_t measurement_cnt;
	uint32_t block_cnt;
	char timestamp[file_timestamp_size];
	char filename_full[file_filename_full_size];
	uint32_t split_mode;
};

extern uint32_t* aPIXEL;
extern uint32_t* aCAMCNT;
extern bool* useSWTrig;
extern uint16_t** userBuffer;
extern uint16_t** userBufferWritePos;
extern uint32_t BOARD_SEL;
extern uint8_t number_of_boards;
extern uint32_t* Nob;
extern uint32_t* Nospb;
extern bool abortMeasurementFlag;
extern uint32_t numberOfInterrupts;
extern bool continiousMeasurementFlag;
extern uint32_t continiousPauseInMicroseconds;
extern struct global_settings settings_struct;
extern bool isRunning;
extern int64_t scanCounterTotal;


#ifdef __cplusplus
}
#endif
#endif // GLOBALS_H
