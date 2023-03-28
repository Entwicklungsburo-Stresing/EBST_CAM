#include "lsc.h"
#include <sstream>
#include "../shared_src/Board.h"

Lsc::Lsc()
{
	driverInstructions = "Check if driver is loaded correctly.";
}
Lsc::~Lsc()
{
}

/**
 * \copydoc InitDriver
 */
es_status_codes Lsc::initDriver()
{
	return InitDriver();
}

/**
 * \copydoc InitBoard
 */
es_status_codes Lsc::initPcieBoard()
{
	return InitBoard();
}

/**
 * \copydoc ExitDriver
 */
es_status_codes Lsc::exitDriver()
{
	return ExitDriver();
}

/**
 * \copydoc InitMeasurement
 */
es_status_codes Lsc::initMeasurement()
{
	return InitMeasurement();
}

/**
 * \copydoc StartMeasurement
 */
es_status_codes Lsc::startMeasurement()
{
	return StartMeasurement();
}

/**
 * \copydoc ReturnFrame
 */
es_status_codes Lsc::returnFrame(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
	return ReturnFrame(drvno, sample, block, camera, pdest, length);
}

std::string Lsc::_dumpS0Registers(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpS0Registers(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpS0Registers failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpDmaRegisters(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpDmaRegisters(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpDmaRegisters failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpTlp(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpTlpRegisters(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpTlpRegisters failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpMeasurementSettings()
{
	char* cstring;
	es_status_codes status = dumpMeasurementSettings(&cstring);
	if(status != es_no_error)
		qCritical("dumpSettings failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpCameraSettings(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpCameraSettings(drvno, &cstring);
	if (status != es_no_error)
		qCritical("dumpSettings failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::_dumpPciRegisters(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpPciRegisters(drvno, &cstring);
	if(status != es_no_error)
		qCritical("dumpPciRegisters failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::__AboutDrv(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = _AboutDrv(drvno, &cstring);
	if(status != es_no_error)
		qCritical("_AboutDrv failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::__AboutGPX(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = _AboutGPX(drvno, &cstring);
	if(status != es_no_error)
		qCritical("_AboutGPX failed");
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

/**
 * \copydoc SetTORReg
 */
es_status_codes Lsc::setTorOut( uint32_t drvno, uint8_t torOut )
{
	return SetTORReg( drvno, torOut );
}

/**
 * \copydoc ResetDSC
 */
es_status_codes Lsc::resetDSC( uint32_t drvno, uint8_t DSCNumber )
{
	return ResetDSC( drvno, DSCNumber );
}

/**
 * \copydoc SetDIRDSC
 */
es_status_codes Lsc::setDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir )
{
	return SetDIRDSC( drvno, DSCNumber, dir );
}

/**
 * \copydoc GetDSC
 */
es_status_codes Lsc::getDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC )
{
	return GetDSC( drvno, DSCNumber, ADSC, LDSC);
}

/**
 * \copydoc CalcTrms
 */
es_status_codes Lsc::calcTRMS( uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms )
{
	return CalcTrms( drvno, firstSample, lastSample, TRMS_pixel, CAMpos, mwf, trms );
}

/**
 * \copydoc AbortMeasurement
 */
es_status_codes Lsc::abortMeasurement()
{
	return AbortMeasurement();
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
 * \copydoc DAC8568_setOutput
 */
es_status_codes Lsc::dac_setOutput(uint32_t drvno, uint8_t location, uint8_t channel, uint16_t output)
{
	return DAC8568_setOutput(drvno, location, channel, output);
}

/**
 * \copydoc DAC8568_setAllOutputs
 */
es_status_codes Lsc::dac_setAllOutputs(uint32_t drvno, uint8_t location, uint32_t* output, bool reorder_channels)
{
	return DAC8568_setAllOutputs(drvno, location, output, reorder_channels);
}

/**
 * \copydoc IOCtrl_setT0
 */
es_status_codes Lsc::ioctrl_setT0(uint32_t drvno, uint32_t period_in_10ns)
{
	return IOCtrl_setT0(drvno, period_in_10ns);
}

/**
 * \copydoc IOCtrl_setOutput
 */
es_status_codes Lsc::ioctrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	return IOCtrl_setOutput(drvno, number, width_in_5ns, delay_in_5ns);
}


void Lsc::getCurrentScanNumber(uint32_t drvno, int64_t* scan, int64_t* block)
{
	return GetCurrentScanNumber(drvno, scan, block);
}

void Lsc::fillUserBufferWithDummyData(uint32_t drvno)
{
	FillUserBufferWithDummyData(drvno);
	emit measureStart();
	emit blockStart();
	emit blockDone();
	emit measureDone();
	return;
}

bool Lsc::IsRunning()
{
	return isRunning;
}

std::string Lsc::getVerifiedDataDialog(struct verify_data_parameter* vd)
{
	char* cstring;
	GetVerifiedDataDialog(vd, &cstring);
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

/**
 * \copydoc GetCameraStatusOverTemp
 */
es_status_codes Lsc::getCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp)
{
	return GetCameraStatusOverTemp(drvno, sample, block, camera_pos, overTemp);
}

/**
 * \copydoc GetCameraStatusTempGood
 */
es_status_codes Lsc::getCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood)
{
	return GetCameraStatusTempGood(drvno, sample, block, camera_pos, tempGood);
}

/**
 * \copydoc GetBlockIndex
 */
es_status_codes Lsc::getBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex)
{
	return GetBlockIndex(drvno, sample, block, camera_pos, blockIndex);
}

/**
 * \copydoc GetScanIndex
 */
es_status_codes Lsc::getScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex)
{
	return GetScanIndex(drvno, sample, block, camera_pos, scanIndex);
}

/**
 * \copydoc GetS1State
 */
es_status_codes Lsc::getS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return GetS1State(drvno, sample, block, camera_pos, state);
}

/**
 * \copydoc GetS2State
 */
es_status_codes Lsc::getS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state)
{
	return GetS2State(drvno, sample, block, camera_pos, state);
}

/**
 * \copydoc GetImpactSignal1
 */
es_status_codes Lsc::getImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return GetImpactSignal1(drvno, sample, block, camera_pos, impactSignal);
}

/**
 * \copydoc GetImpactSignal2
 */
es_status_codes Lsc::getImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal)
{
	return GetImpactSignal2(drvno, sample, block, camera_pos, impactSignal);
}

/**
 * \copydoc GetAllSpecialPixelInformation
 */
es_status_codes Lsc::getAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp)
{
	return GetAllSpecialPixelInformation(drvno, sample, block, camera_pos, sp);
}

/**
 * \copydoc SetContinuousMeasurement
 */
void Lsc::setContinuousMeasurement(bool on)
{
	return SetContinuousMeasurement(on);
}
