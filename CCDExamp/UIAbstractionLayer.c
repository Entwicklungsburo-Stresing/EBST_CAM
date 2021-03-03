//UI abstraction layer for CCDExample
#include "shared_src/UIAbstractionLayer.h"
#include "CCDExamp.h"

void notifyMeasureStart( UINT32 drv )
{

	return;
}

void notifyMeasureDone( UINT32 drv )
{
	double mwf = 0.0; //unused
	CalcTrms( DRV, 0, *Nospb, TRMSpix, 0, &mwf, &TRMSval_global[0] );
	if (CAMCNT > 1) CalcTrms( DRV, 0, *Nospb, TRMSpix, 1, &mwf, &TRMSval_global[1] );
	UpdateDisplay();
	return;
}

void notifyBlockStart( UINT32 drv )
{

	return;
}

void notifyBlockDone( UINT32 drv )
{
	UpdateDisplay();
	return;
}