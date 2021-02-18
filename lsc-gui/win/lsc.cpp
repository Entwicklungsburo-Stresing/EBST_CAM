#include "../lsc.h"

Lsc::Lsc()
{
    driverInstructions = "";
}
Lsc::~Lsc()
{

}

int Lsc::initDriver()
{
    return 1;
}

int Lsc::initPcieBoard()
{
    return 0;
}

void Lsc::initMeasurement()
{

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
