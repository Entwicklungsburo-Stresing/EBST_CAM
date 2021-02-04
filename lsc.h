#ifndef LSC_H
#define LSC_H

#include <stdint.h>
#include <string>

class Lsc
{
public:
    Lsc();
    ~Lsc();
    int initDriver();
    int initPcieBoard();
    void initMeasurement();
    void startMeasurement();
    void returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length);
    std::string driverInstructions;
    std::string dumpS0Registers();
    std::string dumpDmaRegisters();
    std::string dumpTlp();
};

#endif // LSC_H
