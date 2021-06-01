#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define MANUAL_OVERRIDE_TLP false
#define LEGACY_202_14_TLPCNT false
#define _FORCETLPS128 true	//only use payload size 128byte
#define DMA_64BIT_EN false
#define S0_SPACE_OFFSET 0x80
#define INTR_EN true
#define HWDREQ_EN true // enables hardware start of DMA by XCK h->l slope
#define MAXPCIECARDS 5
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

extern uint32_t* aPIXEL;
extern uint32_t* aCAMCNT;
extern bool* useSWTrig;
extern uint16_t** userBuffer;
extern uint32_t BOARD_SEL;
extern uint8_t number_of_boards;
extern uint32_t Nob;
extern uint32_t* Nospb;
extern bool abortMeasurementFlag;
extern uint32_t numberOfInterrupts;

#ifdef __cplusplus
}
#endif
#endif // GLOBALS_H
