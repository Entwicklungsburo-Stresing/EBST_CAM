﻿/*****************************************************************//**
 * @file   lsc.cpp
 * @copydoc lsc.h
 *********************************************************************/

#include "lsc.h"
#include <sstream>
#include "hooks.h"

Lsc::Lsc()
{
	driverInstructions = "Check if driver is loaded correctly.";
}
Lsc::~Lsc()
{
}

/**
 * @copydoc InitDriver
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
 * @copydoc ExitDriver
 */
es_status_codes Lsc::exitDriver()
{
	return DLLExitDriver();
}

/**
 * @copydoc InitMeasurement
 */
es_status_codes Lsc::initMeasurement(struct measurement_settings settings)
{
	return DLLInitMeasurement(settings);
}

/**
 * @copydoc StartMeasurement
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
 * @copydoc CopyOneSample
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
 * @copydoc SetTORReg
 */
es_status_codes Lsc::setTorOut( uint32_t drvno, uint8_t torOut )
{
	return DLLSetTORReg( drvno, torOut );
}

/**
 * @copydoc ResetDSC
 */
es_status_codes Lsc::resetDSC( uint32_t drvno, uint8_t DSCNumber )
{
	return DLLResetDSC( drvno, DSCNumber );
}

/**
 * @copydoc SetDIRDSC
 */
es_status_codes Lsc::setDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir )
{
	return DLLSetDIRDSC( drvno, DSCNumber, dir );
}

/**
 * @copydoc GetDSC
 */
es_status_codes Lsc::getDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC )
{
	return DLLGetDSC( drvno, DSCNumber, ADSC, LDSC);
}

/**
 * @copydoc CalcTrms
 */
es_status_codes Lsc::calcTRMS( uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms )
{
	return DLLCalcTrms( drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf, trms );
}

/**
 * @copydoc SetAbortMeasurementFlag
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
 * @copydoc DAC8568_setOutput
 */
es_status_codes Lsc::dac_setOutput(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output)
{
	return DLLDAC8568_setOutput(drvno, location, cameraPosition, channel, output);
}

/**
 * @copydoc DAC8568_setAllOutputs
 */
es_status_codes Lsc::dac_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, bool reorder_channels)
{
	return DLLDAC8568_setAllOutputs(drvno, location, cameraPosition, output, reorder_channels);
}

/**
 * @copydoc CamIOCtrl_setT0
 */
es_status_codes Lsc::ioctrl_setT0(uint32_t drvno, uint32_t period_in_10ns)
{
	return DLLIOCtrl_setT0(drvno, period_in_10ns);
}

/**
 * @copydoc CamIOCtrl_setOutput
 */
es_status_codes Lsc::ioctrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	return DLLIOCtrl_setOutput(drvno, number, width_in_5ns, delay_in_5ns);
}

/**
 * @copydoc GetCurrentScanNumber
 */
void Lsc::getCurrentScanNumber(uint32_t drvno, int64_t* scan, int64_t* block)
{
	return DLLGetCurrentScanNumber(drvno, scan, block);
}

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
 * @copydoc GetCameraStatusOverTemp
 */
es_status_codes Lsc::getCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp)
{
	return DLLGetCameraStatusOverTemp(drvno, sample, block, camera_pos,(uint8_t*) overTemp);
}

/**
 * @copydoc GetCameraStatusTempGood
 */
es_status_codes Lsc::getCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood)
{
	return DLLGetCameraStatusTempGood(drvno, sample, block, camera_pos, (uint8_t*)tempGood);
}

/**
 * @copydoc GetBlockIndex
 */
es_status_codes Lsc::getBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex)
{
	return DLLGetBlockIndex(drvno, sample, block, camera_pos, blockIndex);
}

/**
 * @copydoc GetScanIndex
 */
es_status_codes Lsc::getScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex)
{
	return DLLGetScanIndex(drvno, sample, block, camera_pos, scanIndex);
}

/**
 * @copydoc GetS1State
 */
es_status_codes Lsc::getS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return DLLGetS1State(drvno, sample, block, camera_pos, (uint8_t*)state);
}

/**
 * @copydoc GetS2State
 */
es_status_codes Lsc::getS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return DLLGetS2State(drvno, sample, block, camera_pos, (uint8_t*)state);
}

/**
 * @copydoc GetImpactSignal1
 */
es_status_codes Lsc::getImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return DLLGetImpactSignal1(drvno, sample, block, camera_pos, impactSignal);
}

/**
 * @copydoc GetImpactSignal2
 */
es_status_codes Lsc::getImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return DLLGetImpactSignal2(drvno, sample, block, camera_pos, impactSignal);
}

/**
 * @copydoc GetAllSpecialPixelInformation
 */
es_status_codes Lsc::getAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	return DLLGetAllSpecialPixelInformation(drvno, sample, block, camera_pos, sp);
}

/**
 * @copydoc SetContinuousMeasurement
 */
void Lsc::setContinuousMeasurement(bool on)
{
	return DLLSetContinuousMeasurement(on);
}

/**
 * @copydoc ShowNewBitmap
 */
void Lsc::showNewBitmap(uint32_t drvno, uint32_t cur_nob, uint16_t cam, uint16_t pixel, uint32_t nos)
{
	return DLLShowNewBitmap(drvno, cur_nob, cam, pixel, nos);
}

/**
 * @copydoc Start2dViewer
 */
void Lsc::start2dViewer(uint32_t drvno, uint32_t cur_nob, uint16_t cam, uint16_t pixel, uint32_t nos)
{
	return DLLStart2dViewer(drvno, cur_nob, cam, pixel, nos);
}

/**
 * @copydoc SetGammaValue
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
 * @copydoc GetGammaBlack
 */
uint16_t Lsc::getGammaBlack()
{
	return DLLGetGammaBlack();
}

/**
 * @copydoc ReadScanFrequencyBit
 */
es_status_codes Lsc::readScanFrequencyBit(uint32_t drvno, bool* scanFrequencyTooHigh)
{
	return DLLReadScanFrequencyBit(drvno,(uint8_t*) scanFrequencyTooHigh);
}

/**
 * @copydoc ResetScanFrequencyBit
 */
es_status_codes Lsc::resetScanFrequencyBit(uint32_t drvno)
{
	return DLLResetScanFrequencyBit(drvno);
}

/**
 * @copydoc ReadBlockFrequencyBit
 */
es_status_codes Lsc::readBlockFrequencyBit(uint32_t drvno, bool* blockFrequencyTooHigh)
{
	return DLLReadBlockFrequencyBit(drvno,(uint8_t*) blockFrequencyTooHigh);
}

/**
 * @copydoc ResetBlockFrequencyBit
 */
es_status_codes Lsc::resetBlockFrequencyBit(uint32_t drvno)
{
	return DLLResetBlockFrequencyBit(drvno);
}

/**
 * @copydoc CheckFifoValid
 */
es_status_codes Lsc::checkFifoValid(uint32_t drvno, bool* valid)
{
	return DLLCheckFifoValid(drvno, (uint8_t*)valid);
}

es_status_codes Lsc::checkFifoOverflow(uint32_t drvno, bool* overflow)
{
	return DLLCheckFifoOverflow(drvno, (uint8_t*)overflow);
}

es_status_codes Lsc::checkFifoEmpty(uint32_t drvno, bool* empty)
{
	return DLLCheckFifoEmpty(drvno, (uint8_t*)empty);
}

es_status_codes Lsc::checkFifoFull(uint32_t drvno, bool* full)
{
	return DLLCheckFifoFull(drvno, (uint8_t*)full);
}

/**
 * @copydoc FindCam
 */
es_status_codes Lsc::findCam(uint32_t drvno)
{
	return DLLFindCam(drvno);
}

/**
 * @copydoc SaveMeasurementDataToFile.
 */
es_status_codes Lsc::SaveMeasurementDataToFile(const char* filename)
{
	return DLLSaveMeasurementDataToFile(filename);
}

es_status_codes Lsc::importMeasurementDataFromFile(const char* fileName)
{
	return DLLImportMeasurementDataFromFile(fileName);
}


es_status_codes Lsc::waitForMeasureDone()
{
	return DLLWaitForMeasureDone();
}

es_status_codes Lsc::getXckLength(uint32_t drvno, uint32_t* xckLengthIn10ns)
{
	return DLLGetXckLength(drvno, xckLengthIn10ns);
}

es_status_codes Lsc::getXckPeriod(uint32_t drvno, uint32_t* xckPeriodIn10ns)
{
	return DLLGetXckPeriod(drvno, xckPeriodIn10ns);
}

es_status_codes Lsc::getBonLength(uint32_t drvno, uint32_t* bonLengthIn10ns)
{
	return DLLGetBonLength(drvno, bonLengthIn10ns);
}

es_status_codes Lsc::getBonPeriod(uint32_t drvno, uint32_t* bonPeriodIns10ns)
{
	return DLLGetBonPeriod(drvno, bonPeriodIns10ns);
}

es_status_codes Lsc::getBlockOn(uint32_t drvno, bool* block_on)
{
	return DLLGetBlockOn(drvno,(uint8_t*) block_on);
}

es_status_codes Lsc::getScanTriggerDetected(uint32_t drvno, bool* detected)
{
	return DLLGetScanTriggerDetected(drvno, (uint8_t*)detected);
}

es_status_codes Lsc::getBlockTriggerDetected(uint32_t drvno, bool* detected)
{
	return DLLGetBlockTriggerDetected(drvno, (uint8_t*)detected);
}

es_status_codes Lsc::resetScanTriggerDetected(uint32_t drvno)
{
	return DLLResetScanTriggerDetected(drvno);
}

es_status_codes Lsc::resetBlockTriggerDetected(uint32_t drvno)
{
	return DLLResetBlockTriggerDetected(drvno);
}

uint32_t Lsc::getVirtualCamcnt(uint32_t drvno)
{
	return DLLGetVirtualCamcnt(drvno);
}

bool Lsc::getTestModeOn()
{
	return DLLGetTestModeOn();
}

es_status_codes Lsc::openShutter(uint32_t drvno)
{
	return DLLOpenShutter(drvno);
}

es_status_codes Lsc::closeShutter(uint32_t drvno)
{
	return DLLCloseShutter(drvno);
}

es_status_codes Lsc::setShutterStates(uint32_t drvno, uint16_t shutter_states)
{
	return DLLSetShutterStates(drvno, shutter_states);
}
