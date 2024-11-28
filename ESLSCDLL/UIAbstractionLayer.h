/*****************************************************************//**
 * @file   UIAbstractionLayer.h
 * @brief  Functions for notifying the UI about the progress of the measurement.
 * 
 * Previously these functions were used by Escam and LabVIEW to notify the UI about the progress of the measurement. Now this functionality is replaced by hooks.h for Escam. LabVIEW still uses these functions.
 * @author Florian Hahn
 * @date   21.01.2021
 *********************************************************************/

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void notifyMeasureStart();
extern void notifyMeasureDone();
extern void notifyBlockStart();
extern void notifyBlockDone();
extern void notifyAllBlocksDone();

#ifdef __cplusplus
}
#endif