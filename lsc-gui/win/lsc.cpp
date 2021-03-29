#include "../lsc.h"
#include "shared_src/board.h"

Lsc::Lsc()
{
    driverInstructions = "";
}
Lsc::~Lsc()
{

}

es_status_codes Lsc::initDriver()
{
    return CCDDrvInit();
}

es_status_codes Lsc::initPcieBoard()
{
    return InitBoard(1);
}

es_status_codes Lsc::initMeasurement(struct global_settings* settings_struct)
{
    return InitMeasurement(settings_struct);
}

es_status_codes Lsc::startMeasurement(uint8_t boardsel)
{
    return StartMeasurement(boardsel);
}

es_status_codes Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    return ReturnFrame(board, sample, block, camera, pdest, length);
}

std::string Lsc::dumpS0Registers()
{
    return NULL;
}

std::string Lsc::dumpDmaRegisters()
{
    return NULL;
}

std::string Lsc::dumpTlp()
{
    return NULL;
}

void Lsc::setTorOut(uint8_t torOut)
{
    return;
}
