#include "lsc.h"
#include <sstream>
#include "../shared_src/Board.h"

Lsc::Lsc()
{
    driverInstructions = "On Linux: Run 'sudo insmod lscpcie.ko' and 'sudo chmod 666 /dev/lscpcie0' before running this application";
}
Lsc::~Lsc()
{
    ExitDriver(1);
}

/**
 * @brief Inits PCIe board driver.
 * @return es_status_codes:
 *      - es_no_error
 *      - es_driver_init_failed
 */
es_status_codes Lsc::initDriver()
{
    return InitDriver();
}

/**
 * @brief Inits PCIe board.
 * @return es_status_codes:
 *      - es_no_error
 *      - es_open_device_failed
 *      - es_getting_dma_buffer_failed
 *      - es_unknown_error
 *		- es_parameter_out_of_range
 *		- es_invalid_driver_number
 *		- es_getting_device_info_failed
 */
es_status_codes Lsc::initPcieBoard()
{
    return InitBoard();
}

/**
 * @brief Init Measurement.
 * 
 * @return es_status_codes:
 *		- es_invalid_driver_number
 *		- es_invalid_driver_handle
 *		- es_no_error
 *		- es_register_write_failed
 *		- es_register_read_failed
 *		- es_parameter_out_of_range
 *		- es_allocating_memory_failed
 *		- es_not_enough_ram
 *		- es_getting_dma_buffer_failed
 *		- es_enabling_interrupts_failed
 *		- es_camera_not_found
 */
es_status_codes Lsc::initMeasurement()
{
    return InitMeasurement();
}

es_status_codes Lsc::startMeasurement()
{
    return StartMeasurement();
}

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

es_status_codes Lsc::setTorOut( uint32_t drvno, uint8_t torOut )
{
	return SetTORReg( drvno, torOut );
}

es_status_codes Lsc::resetDSC( uint32_t drvno, uint8_t DSCNumber )
{
	return resetDSC( drvno, DSCNumber );
}

es_status_codes Lsc::setDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir )
{
	return setDIRDSC( drvno, DSCNumber, dir );
}


es_status_codes Lsc::getDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC )
{
	return getDSC( drvno, DSCNumber, ADSC, LDSC);
}

es_status_codes Lsc::abortMeasurement(uint32_t drvno)
{
    return AbortMeasurement( drvno );
}

void Lsc::parseTextToHtml(std::string* str)
{
    str->insert(0, "<table><tr><td>");
    size_t start_pos = 0;
    std::string from = "\t";
    std::string to = "</td><td>";
    while ((start_pos = str->find(from, start_pos)) != std::string::npos)
    {
        str->replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    start_pos = 0;
    from = "\n";
    to = "</td></tr><tr><td>";
    while ((start_pos = str->find(from, start_pos)) != std::string::npos)
    {
        str->replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    str->append("</td></tr></table>");
}

es_status_codes Lsc::dac_setAllOutputs(uint32_t drvno, uint32_t* output, bool isIr)
{
	return DAC_setAllOutputs(drvno, output, isIr);
}
