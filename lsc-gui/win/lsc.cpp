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

void Lsc::initMeasurement(uint32_t camcnt, uint32_t pixel, uint32_t xckdelay)
{
    InitMeasurement(1, camcnt, pixel, xckdelay);
    return;
}

void Lsc::startMeasurement()
{
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
