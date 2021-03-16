#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QSettings>
#include "lsc.h"

constexpr auto settingNosPath = "measurement/nos";
constexpr auto settingNosDefault = 1000;
constexpr auto settingNobPath = "measurement/nob";
constexpr auto settingNobDefault = 2;
constexpr auto settingStiPath = "measurement/sti";
constexpr auto settingStiDefault = sti_STimer;
constexpr auto settingBtiPath = "measurement/bti";
constexpr auto settingBtiDefault = bti_BTimer;
constexpr auto settingStimerPath = "measurement/stimer";
constexpr auto settingStimerDefault = 1;
constexpr auto settingBtimerPath = "measurement/btimer";
constexpr auto settingBtimerDefault = 1000;
constexpr auto settingSdatPath = "measurement/sdat";
constexpr auto settingSdatDefault = 0;
constexpr auto settingBdatPath = "measurement/bdat";
constexpr auto settingBdatDefault = 0;
constexpr auto settingSslopePath = "measurement/sslope";
constexpr auto settingSslopeDefault = slope_pos;
constexpr auto settingBslopePath = "measurement/bslope";
constexpr auto settingBslopeDefault = slope_pos;
constexpr auto settingXckdelayPath = "measurement/xckdelay";
constexpr auto settingXckdelayDefault = 0;
constexpr auto settingShutterExpTimePath = "measurement/shutterExpTime";
constexpr auto settingShutterExpTimeDefault = 1;
constexpr auto settingTriggerCcPath = "measurement/triggerCc";
constexpr auto settingTriggerCcDefault = xck;
constexpr auto settingBoardSelPath = "camerasetup/boardsel";
constexpr auto settingBoardSelDefault = 0;
constexpr auto settingSensorTypePath = "camerasetup/sensorType";
constexpr auto settingSensorTypeDefault = PDAsensor;
constexpr auto settingCameraSystemPath = "camerasetup/cameraSystem";
constexpr auto settingCameraSystemDefault = camera_system_3001;
constexpr auto settingCamcntPath = "camerasetup/camcnt";
constexpr auto settingCamcntDefault = 1;
constexpr auto settingPixelPath = "camerasetup/pixelcnt";
constexpr auto settingPixelDefault = 576;
constexpr auto settingMshutPath = "camerasetup/mshut";
constexpr auto settingMshutDefault = false;
constexpr auto settingLedPath = "camerasetup/led";
constexpr auto settingLedDefault = false;
constexpr auto settingGain3010Path = "camerasetup/gain3010";
constexpr auto settingGain3010Default = false;
constexpr auto settingGain3030Path = "camerasetup/gain3030";
constexpr auto settingGain3030Default = 0;
constexpr auto settingCoolingPath = "camerasetup/cooling";
constexpr auto settingCoolingDefault = 0;
constexpr auto settingDacPath = "camerasetup/dac";
constexpr auto settingDacDefault = false;
constexpr auto settingGpxPath = "camerasetup/gpx";
constexpr auto settingGpxDefault = false;
constexpr auto settingGpxOffsetPath = "camerasetup/gpxOffset";
constexpr auto settingGpxOffsetDefault = 0;
constexpr auto settingLinesPath = "fftmode/lines";
constexpr auto settingLinesDefault = 64;
constexpr auto settingVfreqPath = "fftmode/vfreq";
constexpr auto settingVfreqDefault = 0;
constexpr auto settingFftModePath = "fftmode/fftmode";
constexpr auto settingFftModeDefault = full_binning;
constexpr auto settingLinesBinningPath = "fftmode/linesbinning";
constexpr auto settingLinesBinningDefault = 1;
constexpr auto settingNumberOfRegionsPath = "fftmode/numberOfRegions";
constexpr auto settingNumberOfRegionsDefault = 3;
constexpr auto settingRegionSizeEqualPath = "fftmode/regionSizeEqual";
constexpr auto settingRegionSizeEqualDefault = true;
constexpr auto settingRegionSize1Path = "fftmode/regionSize1";
constexpr auto settingRegionSize1Default = 10;
constexpr auto settingRegionSize2Path = "fftmode/regionSize2";
constexpr auto settingRegionSize2Default = 44;
constexpr auto settingRegionSize3Path = "fftmode/regionSize3";
constexpr auto settingRegionSize3Default = 10;
constexpr auto settingRegionSize4Path = "fftmode/regionSize4";
constexpr auto settingRegionSize4Default = 10;
constexpr auto settingRegionSize5Path = "fftmode/regionSize5";
constexpr auto settingRegionSize5Default = 10;
constexpr auto settingRegionSize6Path = "fftmode/regionSize6";
constexpr auto settingRegionSize6Default = 10;
constexpr auto settingRegionSize7Path = "fftmode/regionSize7";
constexpr auto settingRegionSize7Default = 10;
constexpr auto settingRegionSize8Path = "fftmode/regionSize8";
constexpr auto settingRegionSize8Default = 10;
constexpr auto settingSensorOffsetChannel1Path = "sensorOffset/channel1";
constexpr auto settingSensorOffsetChannel1Default = 0;
constexpr auto settingSensorOffsetChannel2Path = "sensorOffset/channel2";
constexpr auto settingSensorOffsetChannel2Default = 0;
constexpr auto settingSensorOffsetChannel3Path = "sensorOffset/channel3";
constexpr auto settingSensorOffsetChannel3Default = 0;
constexpr auto settingSensorOffsetChannel4Path = "sensorOffset/channel4";
constexpr auto settingSensorOffsetChannel4Default = 0;
constexpr auto settingSensorOffsetChannel5Path = "sensorOffset/channel5";
constexpr auto settingSensorOffsetChannel5Default = 0;
constexpr auto settingSensorOffsetChannel6Path = "sensorOffset/channel6";
constexpr auto settingSensorOffsetChannel6Default = 0;
constexpr auto settingSensorOffsetChannel7Path = "sensorOffset/channel7";
constexpr auto settingSensorOffsetChannel7Default = 0;
constexpr auto settingSensorOffsetChannel8Path = "sensorOffset/channel8";
constexpr auto settingSensorOffsetChannel8Default = 0;
constexpr auto settingTorPath = "debug/tor";
constexpr auto settingTorDefault = xck_tor;
constexpr auto settingAdcModePath = "debug/adcMode";
constexpr auto settingAdcModeDefault = normal;
constexpr auto settingAdcCustomValuePath = "debug/adcCustomValue";
constexpr auto settingAdcCustomValueDefault = 0;
constexpr auto settingThemePath = "appearance/theme";
constexpr auto settingThemeDefault = lighttheme;


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
    void on_accepted();
};

#endif // DIALOGSETTINGS_H
