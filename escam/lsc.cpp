/*****************************************************************//**
 * @file   lsc.cpp
 * @copydoc lsc.h
 *********************************************************************/

#include "lsc.h"
#include <sstream>
#include "hooks.h"

Lsc::Lsc()
{
}
Lsc::~Lsc()
{
}

/**
 * @copydoc DLLInitDriver
 */
es_status_codes Lsc::initDriver()
{
	es_status_codes status = DLLInitDriver(&numberOfBoards);
	if (status != es_no_error) return status;
	// The signals mesaureStart and measureDone are emitted in Lsc::startMeasurement, which is a more dependable way of doing it.
	DLLSetBlockStartHook(emitBlockStartSignal);
	DLLSetBlockDoneHook(emitBlockDoneSignal);
	DLLSetAllBlocksDoneHook(emitAllBlocksDoneSignal);
	return status;
}

/**
 * @copydoc DLLExitDriver
 */
es_status_codes Lsc::exitDriver()
{
	return DLLExitDriver();
}

/**
 * @copydoc DLLInitMeasurement
 */
es_status_codes Lsc::initMeasurement(struct measurement_settings settings)
{
	return DLLInitMeasurement(settings);
}

/**
 * @copydoc DLLStartMeasurement_blocking
 */
es_status_codes Lsc::startMeasurement()
{
	// Emitting measureStart and measureDone here is a more dependable way of emitting these signals than using the hooks.
	// Using the hooks, the autotune function missed the measureDone signal in rare cases.
	emit measureStart();
	es_status_codes status = DLLStartMeasurement_blocking();
	emit measureDone();
	return status;
}

/**
 * @copydoc DLLCopyOneSample
 */
es_status_codes Lsc::copyOneSample(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t* pdest)
{
	return DLLCopyOneSample(drvno, sample, block, camera, pdest);
}

std::string Lsc::_dumpS0Registers(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLDumpS0Registers(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpS0Registers failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpHumanReadableS0Registers(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLDumpHumanReadableS0Registers(drvno, &cstring);
	if (status != es_no_error)
		qCritical("dumpHumanReadable failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpDmaRegisters(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLDumpDmaRegisters(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpDmaRegisters failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpTlp(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLDumpTlpRegisters(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpTlpRegisters failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpMeasurementSettings()
{
	char* cstring;
	es_status_codes status = DLLDumpMeasurementSettings(&cstring);
	if(status != es_no_error)
		qCritical("dumpSettings failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpCameraSettings(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLDumpCameraSettings(drvno, &cstring);
	if (status != es_no_error)
		qCritical("dumpSettings failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpPciRegisters(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLDumpPciRegisters(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpPciRegisters failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::__AboutDrv(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLAboutDrv(drvno, &cstring);
	if(status != es_no_error)
		qCritical("_AboutDrv failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::__AboutGPX(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = DLLAboutGPX(drvno, &cstring);
	if(status != es_no_error)
		qCritical("_AboutGPX failed");
	std::string cppstring = cstring;
	free(cstring);
	parseTextToHtml(&cppstring);
	return cppstring;
}

/**
 * @copydoc DLLSetTORReg
 */
es_status_codes Lsc::setTorOut( uint32_t drvno, uint8_t tor )
{
	return DLLSetTORReg( drvno, tor );
}

/**
 * @copydoc DLLResetDSC
 */
es_status_codes Lsc::resetDSC( uint32_t drvno, uint8_t DSCNumber )
{
	return DLLResetDSC( drvno, DSCNumber );
}

/**
 * @copydoc DLLSetDIRDSC
 */
es_status_codes Lsc::setDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir )
{
	return DLLSetDIRDSC( drvno, DSCNumber, dir );
}

/**
 * @copydoc DLLGetDSC
 */
es_status_codes Lsc::getDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC )
{
	return DLLGetDSC( drvno, DSCNumber, ADSC, LDSC);
}

/**
 * @copydoc DLLCalcTrms
 */
es_status_codes Lsc::calcTRMS( uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms )
{
	return DLLCalcTrms( drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf, trms );
}

/**
 * @copydoc DLLAbortMeasurement
 */
es_status_codes Lsc::abortMeasurement()
{
	return DLLAbortMeasurement();
}

void Lsc::parseTextToHtml(std::string* str)
{
	// insert <table><tr><td> at the beginning
	str->insert(0, "<table><tr><td>");
	// replace all \t characters with </td><td>
	size_t start_pos = 0;
	std::string from = "\t";
	std::string to = "</td><td>";
	while ((start_pos = str->find(from, start_pos)) != std::string::npos)
	{
		str->replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	// replace all \n characters with </td></tr><tr><td>
	start_pos = 0;
	from = "\n";
	to = "</td></tr><tr><td>";
	while ((start_pos = str->find(from, start_pos)) != std::string::npos)
	{
		str->replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	// insert </td></tr></table> at the end
	str->append("</td></tr></table>");
}

/**
 * @copydoc DLLDAC8568_setOutput
 */
es_status_codes Lsc::dac_setOutput(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output)
{
	return DLLDAC8568_setOutput(drvno, location, cameraPosition, channel, output);
}

/**
 * @copydoc DLLDAC8568_setAllOutputs
 */
es_status_codes Lsc::dac_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, bool reorder_channels)
{
	return DLLDAC8568_setAllOutputs(drvno, location, cameraPosition, output, reorder_channels);
}

/**
 * @copydoc DLLIOCtrl_setT0
 */
es_status_codes Lsc::ioctrl_setT0(uint32_t drvno, uint32_t period_in_10ns)
{
	return DLLIOCtrl_setT0(drvno, period_in_10ns);
}

/**
 * @copydoc DLLIOCtrl_setOutput
 */
es_status_codes Lsc::ioctrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	return DLLIOCtrl_setOutput(drvno, number, width_in_5ns, delay_in_5ns);
}

/**
 * @copydoc DLLGetCurrentScanNumber
 */
void Lsc::getCurrentScanNumber(uint32_t drvno, int64_t* sample, int64_t* block)
{
	return DLLGetCurrentScanNumber(drvno, sample, block);
}

/**
 * @copydoc DLLFillUserBufferWithDummyData
 */
void Lsc::fillUserBufferWithDummyData()
{
	DLLFillUserBufferWithDummyData();
	emit measureStart();
	emit blockStart();
	emit blockDone();
	emit allBlocksDone();
	emit measureDone();
	return;
}

/**
 * @copydoc DLLGetIsRunning
 */
bool Lsc::IsRunning()
{
	return (bool)DLLGetIsRunning();
}

std::string Lsc::getVerifiedDataDialog(struct verify_data_parameter* vd)
{
	char* cstring;
	DLLGetVerifiedDataDialog(vd, &cstring);
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

/**
 * @copydoc DLLGetCameraStatusOverTemp
 */
es_status_codes Lsc::getCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp)
{
	return DLLGetCameraStatusOverTemp(drvno, sample, block, camera_pos,(uint8_t*) overTemp);
}

/**
 * @copydoc DLLGetCameraStatusTempGood
 */
es_status_codes Lsc::getCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood)
{
	return DLLGetCameraStatusTempGood(drvno, sample, block, camera_pos, (uint8_t*)tempGood);
}

/**
 * @copydoc DLLGetBlockIndex
 */
es_status_codes Lsc::getBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex)
{
	return DLLGetBlockIndex(drvno, sample, block, camera_pos, blockIndex);
}

/**
 * @copydoc DLLGetScanIndex
 */
es_status_codes Lsc::getScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex)
{
	return DLLGetScanIndex(drvno, sample, block, camera_pos, scanIndex);
}

/**
 * @copydoc DLLGetS1State
 */
es_status_codes Lsc::getS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return DLLGetS1State(drvno, sample, block, camera_pos, (uint8_t*)state);
}

/**
 * @copydoc DLLGetS2State
 */
es_status_codes Lsc::getS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return DLLGetS2State(drvno, sample, block, camera_pos, (uint8_t*)state);
}

/**
 * @copydoc DLLGetImpactSignal1
 */
es_status_codes Lsc::getImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return DLLGetImpactSignal1(drvno, sample, block, camera_pos, impactSignal);
}

/**
 * @copydoc DLLGetImpactSignal2
 */
es_status_codes Lsc::getImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return DLLGetImpactSignal2(drvno, sample, block, camera_pos, impactSignal);
}

/**
 * @copydoc DLLGetAllSpecialPixelInformation
 */
es_status_codes Lsc::getAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	return DLLGetAllSpecialPixelInformation(drvno, sample, block, camera_pos, sp);
}

/**
 * @copydoc DLLSetContinuousMeasurement
 */
void Lsc::setContinuousMeasurement(bool on)
{
	return DLLSetContinuousMeasurement(on);
}

/**
 * @copydoc DLLShowNewBitmap
 */
void Lsc::showNewBitmap(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos)
{
	return DLLShowNewBitmap(drvno, block, camera, pixel, nos);
}

/**
 * @copydoc DLLStart2dViewer
 */
void Lsc::start2dViewer(uint32_t drvno, uint32_t block, uint16_t camera, uint16_t pixel, uint32_t nos)
{
	return DLLStart2dViewer(drvno, block, camera, pixel, nos);
}

/**
 * @copydoc DLLSetGammaValue
 */
void Lsc::setGammaValue(uint16_t white, uint16_t black)
{
	return DLLSetGammaValue(white, black);
}

/**
 * @copydoc GetGammaWhite
 */
uint16_t Lsc::getGammaWhite()
{
	return DLLGetGammaWhite();
}

/**
 * @copydoc DLLGetGammaBlack
 */
uint16_t Lsc::getGammaBlack()
{
	return DLLGetGammaBlack();
}

/**
 * @copydoc DLLReadScanFrequencyBit
 */
es_status_codes Lsc::readScanFrequencyBit(uint32_t drvno, bool* scanFrequencyTooHigh)
{
	return DLLReadScanFrequencyBit(drvno,(uint8_t*) scanFrequencyTooHigh);
}

/**
 * @copydoc DLLResetScanFrequencyBit
 */
es_status_codes Lsc::resetScanFrequencyBit(uint32_t drvno)
{
	return DLLResetScanFrequencyBit(drvno);
}

/**
 * @copydoc DLLReadBlockFrequencyBit
 */
es_status_codes Lsc::readBlockFrequencyBit(uint32_t drvno, bool* blockFrequencyTooHigh)
{
	return DLLReadBlockFrequencyBit(drvno,(uint8_t*) blockFrequencyTooHigh);
}

/**
 * @copydoc DLLResetBlockFrequencyBit
 */
es_status_codes Lsc::resetBlockFrequencyBit(uint32_t drvno)
{
	return DLLResetBlockFrequencyBit(drvno);
}

/**
 * @copydoc DLLCheckFifoValid
 */
es_status_codes Lsc::checkFifoValid(uint32_t drvno, bool* valid)
{
	return DLLCheckFifoValid(drvno, (uint8_t*)valid);
}

/**
 * @copydoc DLLCheckFifoOverflow
 */
es_status_codes Lsc::checkFifoOverflow(uint32_t drvno, bool* overflow)
{
	return DLLCheckFifoOverflow(drvno, (uint8_t*)overflow);
}

/**
 * @copydoc DLLCheckFifoEmpty
 */
es_status_codes Lsc::checkFifoEmpty(uint32_t drvno, bool* empty)
{
	return DLLCheckFifoEmpty(drvno, (uint8_t*)empty);
}

/**
 * @copydoc DLLCheckFifoFull
 */
es_status_codes Lsc::checkFifoFull(uint32_t drvno, bool* full)
{
	return DLLCheckFifoFull(drvno, (uint8_t*)full);
}

/**
 * @copydoc DLLFindCam
 */
es_status_codes Lsc::findCam(uint32_t drvno)
{
	return DLLFindCam(drvno);
}

/**
 * @copydoc DLLSaveMeasurementDataToFile.
 */
es_status_codes Lsc::saveMeasurementDataToFile(const char* filename)
{
	return DLLSaveMeasurementDataToFile(filename);
}

/**
 * @copydoc DLLImportMeasurementDataFromFile
 */
es_status_codes Lsc::importMeasurementDataFromFile(const char* fileName)
{
	return DLLImportMeasurementDataFromFile(fileName);
}

/**
 * @copydoc DLLWaitForMeasureDone
 */
es_status_codes Lsc::waitForMeasureDone()
{
	return DLLWaitForMeasureDone();
}

/**
 * @copydoc DLLGetXckLength
 */
es_status_codes Lsc::getXckLength(uint32_t drvno, uint32_t* xckLengthIn10ns)
{
	return DLLGetXckLength(drvno, xckLengthIn10ns);
}

/**
 * @copydoc DLLGetXckPeriod
 */
es_status_codes Lsc::getXckPeriod(uint32_t drvno, uint32_t* xckPeriodIn10ns)
{
	return DLLGetXckPeriod(drvno, xckPeriodIn10ns);
}

/**
 * @copydoc DLLGetBonLength
 */
es_status_codes Lsc::getBonLength(uint32_t drvno, uint32_t* bonLengthIn10ns)
{
	return DLLGetBonLength(drvno, bonLengthIn10ns);
}

/**
 * @copydoc DLLGetBonPeriod
 */
es_status_codes Lsc::getBonPeriod(uint32_t drvno, uint32_t* bonPeriodIn10ns)
{
	return DLLGetBonPeriod(drvno, bonPeriodIn10ns);
}

/**
 * @copydoc GetBlockOn
 */
es_status_codes Lsc::getBlockOn(uint32_t drvno, bool* blockOn)
{
	return DLLGetBlockOn(drvno,(uint8_t*)blockOn);
}

/**
 * @copydoc DLLGetScanTriggerDetected
 */
es_status_codes Lsc::getScanTriggerDetected(uint32_t drvno, bool* detected)
{
	return DLLGetScanTriggerDetected(drvno, (uint8_t*)detected);
}

/**
 * @copydoc DLLGetBlockTriggerDetected
 */
es_status_codes Lsc::getBlockTriggerDetected(uint32_t drvno, bool* detected)
{
	return DLLGetBlockTriggerDetected(drvno, (uint8_t*)detected);
}

/**
 * @copydoc DLLResetScanTriggerDetected
 */
es_status_codes Lsc::resetScanTriggerDetected(uint32_t drvno)
{
	return DLLResetScanTriggerDetected(drvno);
}

/**
 * @copydoc DLLResetBlockTriggerDetected
 */
es_status_codes Lsc::resetBlockTriggerDetected(uint32_t drvno)
{
	return DLLResetBlockTriggerDetected(drvno);
}

/**
 * @copydoc DLLGetVirtualCamcnt
 */
uint32_t Lsc::getVirtualCamcnt(uint32_t drvno)
{
	return DLLGetVirtualCamcnt(drvno);
}

/**
 * @copydoc DLLGetTestModeOn
 */
bool Lsc::getTestModeOn()
{
	return DLLGetTestModeOn();
}

/**
 * @copydoc DLLOpenShutter
 */
es_status_codes Lsc::openShutter(uint32_t drvno)
{
	return DLLOpenShutter(drvno);
}

/**
 * @copydoc DLLCloseShutter
 */
es_status_codes Lsc::closeShutter(uint32_t drvno)
{
	return DLLCloseShutter(drvno);
}

/**
 * @copydoc DLLSetShutterStates
 */
es_status_codes Lsc::setShutterStates(uint32_t drvno, uint16_t shutter_states)
{
	return DLLSetShutterStates(drvno, shutter_states);
}
