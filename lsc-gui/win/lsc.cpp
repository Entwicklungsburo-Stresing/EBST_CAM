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

    SetGlobalVariables(1, 1, 576, 0);
    return InitBoard(1);
}

void Lsc::initMeasurement(struct global_settings* settings_struct)
{
   //old InitMeasurement(1, camcnt, pixel, xckdelay);
    InitMeasurement(settings_struct);
    return;
}

void Lsc::startMeasurement(uint8_t boardsel)
{
    StartMess(boardsel);
    return;
}

void Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{
    ReturnFrame(board, sample, block, camera, pdest, length);
    return;
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
