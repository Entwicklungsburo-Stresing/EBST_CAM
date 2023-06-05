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
	es_status_codes exitDriver();
	es_status_codes returnFrame(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera, uint32_t length, uint16_t* pdest);
	es_status_codes abortMeasurement();
	es_status_codes resetDSC( uint32_t drvno, uint8_t DSCNumber );
	es_status_codes setDIRDSC( uint32_t drvno, uint8_t DSCNumber, bool dir );
	es_status_codes getDSC( uint32_t drvno, uint8_t DSCNumber, uint32_t* ADSC, uint32_t* LDSC );
	es_status_codes calcTRMS( uint32_t drvno, uint32_t firstSample, uint32_t lastSample, uint32_t TRMS_pixel, uint16_t CAMpos, double *mwf, double *trms );
	std::string driverInstructions;
	std::string _dumpS0Registers(uint32_t drvno);
	std::string _dumpDmaRegisters(uint32_t drvno);
	std::string _dumpTlp(uint32_t drvno);
	std::string _dumpMeasurementSettings();
	std::string _dumpCameraSettings(uint32_t drvno);
	std::string _dumpPciRegisters(uint32_t drvno);
	std::string __AboutDrv(uint32_t drvno);
	std::string __AboutGPX(uint32_t drvno);
	es_status_codes setTorOut(uint32_t drvno, uint8_t torOut);
	es_status_codes dac_setOutput(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint8_t channel, uint16_t output);
	es_status_codes dac_setAllOutputs(uint32_t drvno, uint8_t location, uint8_t cameraPosition, uint32_t* output, bool reorder_channel);
	es_status_codes ioctrl_setT0(uint32_t drvno, uint32_t period_in_10ns);
	es_status_codes ioctrl_setOutput(uint32_t drvno, uint32_t number, uint16_t width_in_5ns, uint16_t delay_in_5ns);
	void getCurrentScanNumber(uint32_t drvno, int64_t* scan, int64_t* block);
	void fillUserBufferWithDummyData(uint32_t drvno);
	bool IsRunning();
	std::string getVerifiedDataDialog(struct verify_data_parameter* vd);
	es_status_codes getCameraStatusOverTemp(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* overTemp);
	es_status_codes getCameraStatusTempGood(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* tempGood);
	es_status_codes getBlockIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* blockIndex);
	es_status_codes getScanIndex(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* scanIndex);
	es_status_codes getS1State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state);
	es_status_codes getS2State(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, bool* state);
	es_status_codes getImpactSignal1(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal);
	es_status_codes getImpactSignal2(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, uint32_t* impactSignal);
	es_status_codes getAllSpecialPixelInformation(uint32_t drvno, uint32_t sample, uint32_t block, uint16_t camera_pos, struct special_pixels* sp);
	void setContinuousMeasurement(bool on);
	void showNewBitmap(uint32_t drvno, uint32_t cur_nob, uint16_t cam, uint16_t pixel, uint32_t nos);
	void start2dViewer(uint32_t drvno, uint32_t cur_nob, uint16_t cam, uint16_t pixel, uint32_t nos);
	void setGammaValue(uint16_t white, uint16_t black);
	uint16_t getGammaWhite();
	uint16_t getGammaBlack();
public slots:
	es_status_codes startMeasurement();
signals:
	void measureStart();
	void measureDone();
	void blockStart();
	void blockDone();
	void allBlocksDone();
private:
	uint8_t _torOut = tor_xck;
	void parseTextToHtml(std::string* str);
};

#endif // LSC_H
