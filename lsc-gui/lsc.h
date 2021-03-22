#ifndef LSC_H
#define LSC_H

#include <stdint.h>
#include <string>
#include <QObject>
#include "shared_src/enum.h"
#include "shared_src/struct.h"

class Lsc : public QObject
{
    Q_OBJECT
public:
    Lsc();
    ~Lsc();
    es_status_codes initDriver();
    es_status_codes initPcieBoard();
    void initMeasurement(struct global_settings* settings_struct);
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
