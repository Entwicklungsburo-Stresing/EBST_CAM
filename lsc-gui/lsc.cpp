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
 */
es_status_codes Lsc::initPcieBoard(uint32_t drvno)
{
    return InitBoard(drvno);
}

/**
 * @brief Init Measurement.
 * @param settings_struct
 * @return
 */
es_status_codes Lsc::initMeasurement()
{
    return InitMeasurement();
}

es_status_codes Lsc::startMeasurement()
{
    return StartMeasurement();
}

es_status_codes Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    return ReturnFrame(board, sample, block, camera, pdest, length);
}

std::string Lsc::_dumpS0Registers(uint32_t drvno)
{
    char* cstring;
    es_status_codes status = dumpS0Registers(drvno, &cstring);
    std::string cppstring = cstring;
    return cppstring;
}

std::string Lsc::_dumpDmaRegisters(uint32_t drvno)
{
    char* cstring;
    es_status_codes status = dumpDmaRegisters(drvno, &cstring);
    std::string cppstring = cstring;
    return cppstring;
}

std::string Lsc::_dumpTlp(uint32_t drvno)
{
    char* cstring;
    es_status_codes status = dumpTlpRegisters(drvno, &cstring);
    std::string cppstring = cstring;
    return cppstring;
}

std::string Lsc::_dumpGlobalSettings()
{
    char* cstring;
    es_status_codes status = dumpSettings(&cstring);
    std::string cppstring = cstring;
    return cppstring;
}

es_status_codes Lsc::setTorOut(uint32_t drvno, uint8_t torOut)
{
    return SetTORReg(drvno, torOut);
}

es_status_codes Lsc::abortMeasurement(uint32_t drvno)
{
    return AbortMeasurement( drvno );
}
