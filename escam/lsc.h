#ifndef LSC_H
#define LSC_H

#include <stdint.h>
#include <string>
#include <QObject>
#include "../shared_src/enum.h"
#include "../shared_src/struct.h"
#include "../shared_src/es_status_codes.h"
#include "../shared_src/globals.h"

class Lsc : public QObject
{
    Q_OBJECT
public:
    Lsc();
    ~Lsc();
    es_status_codes initDriver();
    es_status_codes initPcieBoard();
	es_status_codes initMeasurement();
    es_status_codes returnFrame(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint16_t *pdest, uint32_t length);
	es_status_codes abortMeasurement( uint32_t drvno );
	es_status_codes resetDSC( uint32_t drvno, uint8_t DSCNumber );
	es_status_codes setDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir );
	es_status_codes getDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC );
	es_status_codes calcTRMS( uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms );
    std::string driverInstructions;
    std::string _dumpS0Registers(uint32_t drvno);
    std::string _dumpDmaRegisters(uint32_t drvno);
    std::string _dumpTlp(uint32_t drvno);
    std::string _dumpGlobalSettings();
	std::string _dumpPciRegisters(uint32_t drvno);
    es_status_codes setTorOut(uint32_t drvno, uint8_t torOut);
	es_status_codes dac_setAllOutputs(uint32_t drvno, uint32_t* output, bool isIr);
	es_status_codes ioctrl_setT0(uint32_t drvno, uint32_t period_in_10ns);
	es_status_codes ioctrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns);
public slots:
    es_status_codes startMeasurement();
signals:
    void measureStart();
    void measureDone();
    void blockStart();
    void blockDone();
private:
    uint8_t _torOut = xck_tor;
    void parseTextToHtml(std::string* str);
};

#endif // LSC_H
