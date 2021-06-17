//UI abstraction layer for CCDExample
#include "shared_src/UIAbstractionLayer.h"
#include "CCDExamp.h"

void notifyMeasureStart( uint32_t drv )
{

	return;
}

void notifyMeasureDone( uint32_t drv )
{
	double mwf = 0.0; //unused
	CalcTrms( DRV, 0, *Nospb, TRMSpix, 0, &mwf, &TRMSval_global[0] );
	if (CAMCNT > 1) CalcTrms( DRV, 0, *Nospb, TRMSpix, 1, &mwf, &TRMSval_global[1] );
	enableAllControls();
	UpdateDisplay();
	return;
}

void notifyBlockStart( uint32_t drv )
{

	return;
}

void notifyBlockDone( uint32_t drv )
{
	return;
}