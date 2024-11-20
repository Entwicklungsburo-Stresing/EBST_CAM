#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../shared_src/struct.h"
#include "../shared_src/enum_settings.h"
#include "../shared_src/enum_hardware.h"

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

extern uint32_t* virtualCamcnt;
extern uint16_t** userBuffer;
extern uint16_t** userBufferEndPtr;
extern uint16_t** userBufferWritePos;
extern uint16_t** userBufferWritePos_last;
extern uint8_t number_of_boards;
extern bool testModeOn;
extern volatile bool abortMeasurementFlag;
extern volatile uint32_t* numberOfInterrupts;
extern volatile bool continuousMeasurementFlag;
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
extern uint16_t* pcieCardMajorVersion;
extern uint16_t* pcieCardMinorVersion;
typedef void (*hookFunction)();
extern hookFunction measureStartHook;
extern hookFunction measureDoneHook;
extern hookFunction blockStartHook;
extern hookFunction blockDoneHook;
extern hookFunction allBlocksDoneHook;

#ifdef __cplusplus
}
#endif
