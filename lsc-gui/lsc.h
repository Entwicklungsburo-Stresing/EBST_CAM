#ifndef LSC_H
#define LSC_H

#include <stdint.h>
#include <string>
#include <QObject>
#include "../shared_src/enum.h"
#include "../shared_src/struct.h"
#include "../shared_src/es_status_codes.h"

class Lsc : public QObject
{
    Q_OBJECT
public:
    Lsc();
    ~Lsc();
    es_status_codes initDriver();
    es_status_codes initPcieBoard();
	es_status_codes initMeasurement(struct global_settings settings_struct);
    es_status_codes returnFrame(uint32_t board, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length);
    es_status_codes _abortMeasurement();
    std::string driverInstructions;
    std::string dumpS0Registers();
    std::string dumpDmaRegisters();
    std::string dumpTlp();
    void setTorOut(uint8_t torOut);
public slots:
    es_status_codes startMeasurement();
signals:
    void measureStart();
    void measureDone();
    void blockStart();
    void blockDone();
private:
    uint8_t _torOut = xck_tor;
};

#endif // LSC_H
