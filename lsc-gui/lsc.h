#ifndef LSC_H
#define LSC_H

#include <stdint.h>
#include <string>
#include <QObject>

enum tor_out
{
    xck_tor = 0,
    rego = 1,
    von = 2,
    dma_act = 3,
    asls = 4,
    stimer = 5,
    btimer = 6,
    isr_act = 7,
    s1 = 8,
    s2 = 9,
    bon = 10,
    measureon = 11,
    sdat = 12,
    bdat = 13,
    sshut = 14,
    bshut = 15
};

class Lsc : public QObject
{
    Q_OBJECT
public:
    Lsc();
    ~Lsc();
    int initDriver();
    int initPcieBoard();
    void initMeasurement(uint32_t camcnt, uint32_t pixel, uint32_t xckdelay);
    void startMeasurement();
    void returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length);
    std::string driverInstructions;
    std::string dumpS0Registers();
    std::string dumpDmaRegisters();
    std::string dumpTlp();
    void setTorOut(uint8_t torOut);
signals:
    void measureStart();
    void measureDone();
    void blockStart();
    void blockDone();
private:
    uint8_t _torOut = xck_tor;
};

#endif // LSC_H
