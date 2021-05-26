#include "globals.h"

uint32_t tmp_aPIXEL[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
uint32_t* aPIXEL = tmp_aPIXEL;
uint32_t tmp_aCAMCNT[MAXPCIECARDS] = { 1, 1, 1, 1, 1 };	// cameras parallel
uint32_t* aCAMCNT = tmp_aCAMCNT;	// cameras parallel
bool useSWTrig_temp = false;
bool* useSWTrig = &useSWTrig_temp;
uint16_t* temp_userBuffer[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
uint16_t** userBuffer= temp_userBuffer;
uint32_t BOARD_SEL = 1;
uint32_t numberOfInterrupts = 0;
uint8_t number_of_boards = 0;
uint32_t Nob = 1;
uint32_t tmp_Nosbp = 1000;
uint32_t* Nospb = &tmp_Nosbp;
bool abortMeasurementFlag = false;
