#include "lsc.h"
#include <sstream>
#include "../shared_src/Board.h"

Lsc::Lsc()
{
    driverInstructions = "Check if driver is loaded correctly.";
}
Lsc::~Lsc()
{
    switch (number_of_boards)
    {
    default:
    case 1:
        ExitDriver(1);
        break;
    case 2:
        ExitDriver(3);
        break;
    }
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
    std::string cppstring = cstring;
    parseTextToHtml(&cppstring);
    return cppstring;
}

std::string Lsc::_dumpDmaRegisters(uint32_t drvno)
{
    char* cstring;
    es_status_codes status = dumpDmaRegisters(drvno, &cstring);
    std::string cppstring = cstring;
    parseTextToHtml(&cppstring);
    return cppstring;
}

std::string Lsc::_dumpTlp(uint32_t drvno)
{
    char* cstring;
    es_status_codes status = dumpTlpRegisters(drvno, &cstring);
    std::string cppstring = cstring;
    parseTextToHtml(&cppstring);
    return cppstring;
}

std::string Lsc::_dumpGlobalSettings()
{
    char* cstring;
    es_status_codes status = dumpSettings(&cstring);
    std::string cppstring = cstring;
    parseTextToHtml(&cppstring);
    return cppstring;
}

std::string Lsc::_dumpPciRegisters(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = dumpPciRegisters(drvno, &cstring);
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::__AboutDrv(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = _AboutDrv(drvno, &cstring);
	std::string cppstring = cstring;
	parseTextToHtml(&cppstring);
	return cppstring;
}

std::string Lsc::__AboutGPX(uint32_t drvno)
{
	char* cstring;
	es_status_codes status = _AboutGPX(drvno, &cstring);
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
es_status_codes Lsc::abortMeasurement(uint32_t drvno)
{
    return AbortMeasurement( drvno );
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
 * \copydoc DAC_setAllOutputs
 */
es_status_codes Lsc::dac_setAllOutputs(uint32_t drvno, uint32_t* output, bool isIr)
{
	return DAC_setAllOutputs(drvno, output, isIr);
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
