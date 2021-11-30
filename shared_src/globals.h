#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "struct.h"

#define MANUAL_OVERRIDE_TLP false
#define LEGACY_202_14_TLPCNT false
#define _FORCETLPS128 true	//only use payload size 128byte
#define DMA_64BIT_EN false
#define S0_SPACE_OFFSET 0x80
#define INTR_EN true
#define HWDREQ_EN true // enables hardware start of DMA by XCK h->l slope
#define DMA_BUFFER_SIZE_IN_SCANS 1000//60 is also working with highspeed (expt=0,02ms) //30 could be with one wrong scan every 10000 scans
#define DMA_BUFFER_PARTS 2
#define DMA_DMASPERINTR DMA_BUFFER_SIZE_IN_SCANS / DMA_BUFFER_PARTS  // alle halben buffer ein intr um hi/lo part zu kopieren deshalb
/**
 * @brief DMA_CONTIGBUF: DMA buffer type switch.
 * 
 * true: DMA buffer is set by driver (data must be copied afterwards to user space).
 * false: DMA buffer is set by application (pointer must be passed to SetupPCIE_DMA).
 */
#define DMA_CONTIGBUF true
/**
 * \brief Determines which method is used to copy data from DMA to user buffer.
 * 
 * true: Use Software Polling. When there is new available data in the DMA buffer, a thread copies the data one scan at a time to the user buffer. This method is reliable up to about 3kHz.
 * false: Use Interrupt. Every 500th scan the interrupt starts a copy process, which copies 500 scans to the user buffer.
 */
#define USE_SOFTWARE_POLLING true

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

#ifdef __cplusplus
}
#endif
#endif // GLOBALS_H
