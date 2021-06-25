#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QSettings>
#include "lsc.h"
#include "../shared_src/default_settings.h"

constexpr auto settingNosPath = "measurement/nos";
constexpr auto settingNobPath = "measurement/nob";
constexpr auto settingStiPath = "measurement/sti";
constexpr auto settingBtiPath = "measurement/bti";
constexpr auto settingStime_in_microseconds_Path = "measurement/stimer";
constexpr auto settingBtime_in_microseconds_Path = "measurement/btimer";
constexpr auto settingSdat_in_100nsPath = "measurement/sdat";
constexpr auto settingBdat_in_100nsPath = "measurement/bdat";
constexpr auto settingSslopePath = "measurement/sslope";
constexpr auto settingBslopePath = "measurement/bslope";
constexpr auto settingXckdelayPath = "measurement/xckdelay";
constexpr auto settingShutterExpTimeIn100nsPath = "measurement/shutterExpTimeIn100ns";
constexpr auto settingTriggerCcPath = "measurement/triggerCc";
constexpr auto settingBoardSelPath = "camerasetup/boardsel";
constexpr auto settingSensorTypePath = "camerasetup/sensorType";
constexpr auto settingCameraSystemPath = "camerasetup/cameraSystem";
constexpr auto settingCamcntPath = "camerasetup/camcnt";
constexpr auto settingPixelPath = "camerasetup/pixelcnt";
constexpr auto settingMshutPath = "camerasetup/mshut";
constexpr auto settingLedPath = "camerasetup/led";
constexpr auto settingGain3010Path = "camerasetup/gain3010";
constexpr auto settingGain3030Path = "camerasetup/gain3030";
constexpr auto settingCoolingPath = "camerasetup/cooling";
constexpr auto settingDacPath = "camerasetup/dac";
constexpr auto settingGpxPath = "camerasetup/gpx";
constexpr auto settingGpxOffsetPath = "camerasetup/gpxOffset";
constexpr auto settingLinesPath = "fftmode/lines";
constexpr auto settingVfreqPath = "fftmode/vfreq";
constexpr auto settingFftModePath = "fftmode/fftmode";
constexpr auto settingLinesBinningPath = "fftmode/linesbinning";
constexpr auto settingNumberOfRegionsPath = "fftmode/numberOfRegions";
constexpr auto settingRegionSizeEqualPath = "fftmode/regionSizeEqual";
constexpr auto settingRegionSize1Path = "fftmode/regionSize1";
constexpr auto settingRegionSize2Path = "fftmode/regionSize2";
constexpr auto settingRegionSize3Path = "fftmode/regionSize3";
constexpr auto settingRegionSize4Path = "fftmode/regionSize4";
constexpr auto settingRegionSize5Path = "fftmode/regionSize5";
constexpr auto settingRegionSize6Path = "fftmode/regionSize6";
constexpr auto settingRegionSize7Path = "fftmode/regionSize7";
constexpr auto settingRegionSize8Path = "fftmode/regionSize8";
constexpr auto settingSensorOffsetChannel1Path = "sensorOffset/channel1";
constexpr auto settingSensorOffsetChannel2Path = "sensorOffset/channel2";
constexpr auto settingSensorOffsetChannel3Path = "sensorOffset/channel3";
constexpr auto settingSensorOffsetChannel4Path = "sensorOffset/channel4";
constexpr auto settingSensorOffsetChannel5Path = "sensorOffset/channel5";
constexpr auto settingSensorOffsetChannel6Path = "sensorOffset/channel6";
constexpr auto settingSensorOffsetChannel7Path = "sensorOffset/channel7";
constexpr auto settingSensorOffsetChannel8Path = "sensorOffset/channel8";
constexpr auto settingTorPath = "debug/tor";
constexpr auto settingAdcModePath = "debug/adcMode";
constexpr auto settingAdcCustomValuePath = "debug/adcCustomValue";

//GUI Settings
constexpr auto settingThemePath = "appearance/theme";

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QSettings* settings, QWidget *parent = nullptr);
    ~DialogSettings();
signals:
    void settings_saved();
private:
    Ui::DialogSettings *ui;
    QSettings* _settings;
private slots:
    void loadDefaults();
    void on_accepted();
    void on_comboBoxSti_currentIndexChanged(int index);
    void on_comboBoxBti_currentIndexChanged(int index);
    void on_comboBoxSensorType_currentIndexChanged(int index);
    void on_checkBoxUseDac_stateChanged(int arg1);
    void on_comboBoxCameraSystem_currentIndexChanged(int index);
    void on_checkBoxMshut_stateChanged(int arg1);
    void on_checkBoxRegionsEqual_stateChanged(int arg1);
    void on_pushButtonDefault_clicked();
};

#endif // DIALOGSETTINGS_H
