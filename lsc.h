#ifndef LSC_H
#define LSC_H

#include <stdint.h>
#include <QString>

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
    QString driverInstructions = "Run 'sudo insmod lscpcie.ko' and 'sudo chmod 666 /dev/lscpcie0' before running this application";
};

#endif // LSC_H
