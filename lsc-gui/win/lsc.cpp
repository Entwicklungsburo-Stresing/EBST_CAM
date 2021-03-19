#include "../lsc.h"
#include "shared_src/board.h"

Lsc::Lsc()
{
    driverInstructions = "";
}
Lsc::~Lsc()
{

}

int Lsc::initDriver()
{
    return CCDDrvInit();
}

int Lsc::initPcieBoard()
{
    return InitBoard(1);
}

void Lsc::initMeasurement(struct global_settings* settings_struct)
{
   //old InitMeasurement(1, camcnt, pixel, xckdelay);
    InitMeasurement(settings_struct);
    return;
}

void Lsc::startMeasurement()
{
    StartMess(1);
    return;
}

void Lsc::returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length)
{

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
