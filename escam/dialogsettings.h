#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QSettings>
#include "lsc.h"
#include "../shared_src/default_settings.h"

//measurement
constexpr auto settingNosPath = "nos";
constexpr auto settingNobPath = "nob";
constexpr auto settingStiPath = "sti";
constexpr auto settingBtiPath = "bti";
constexpr auto settingStime_in_microseconds_Path = "stimer";
constexpr auto settingBtime_in_microseconds_Path = "btimer";
constexpr auto settingSdat_in_10nsPath = "sdat";
constexpr auto settingBdat_in_10nsPath = "bdat";
constexpr auto settingSslopePath = "sslope";
constexpr auto settingBslopePath = "bslope";
constexpr auto settingXckdelayIn10nsPath = "xckdelay_in_10ns";
constexpr auto settingShutterSecIn10nsPath = "shutterSecIn10ns";
constexpr auto settingShutterBecIn10nsPath = "shutterBecIn10ns";
constexpr auto settingTriggerCcPath = "triggerCc";
constexpr auto settingContiniousPauseInMicrosecondsPath = "contPauseInMicroseconds";
//camera setup
constexpr auto settingBoardSelPath = "boardSel";
constexpr auto settingBoard0Path = "board0";
constexpr auto settingBoard1Path = "board1";
constexpr auto settingBoard2Path = "board2";
constexpr auto settingBoard3Path = "board3";
constexpr auto settingBoard4Path = "board4";
constexpr auto settingSensorTypePath = "sensorType";
constexpr auto settingCameraSystemPath = "cameraSystem";
constexpr auto settingCamcntPath = "camcnt";
constexpr auto settingPixelPath = "pixelcnt";
constexpr auto settingMshutPath = "mshut";
constexpr auto settingLedPath = "led";
constexpr auto settingSensorGainPath = "sensorGain";
constexpr auto settingAdcGainPath = "adcGain";
constexpr auto settingCoolingPath = "cooling";
constexpr auto settingGpxOffsetPath = "gpxOffset";
constexpr auto settingIsIrPath = "isIr";
constexpr auto settingIOCtrlImpactStartPixelPath = "IOCtrlImpactStartPixel";
constexpr auto settingShortrsPath = "shortrs";
constexpr auto settingIsCooledCamPath = "isCooledCam";
//FFT mode
constexpr auto settingLinesPath = "lines";
constexpr auto settingVfreqPath = "vfreq";
constexpr auto settingFftModePath = "fftmode";
constexpr auto settingLinesBinningPath = "linesbinning";
constexpr auto settingNumberOfRegionsPath = "numberOfRegions";
constexpr auto settingRegionSizeEqualPath = "regionSizeEqual";
constexpr auto settingKeepPath = "keep";
constexpr auto settingRegionSize1Path = "regionSize1";
constexpr auto settingRegionSize2Path = "regionSize2";
constexpr auto settingRegionSize3Path = "regionSize3";
constexpr auto settingRegionSize4Path = "regionSize4";
constexpr auto settingRegionSize5Path = "regionSize5";
constexpr auto settingRegionSize6Path = "regionSize6";
constexpr auto settingRegionSize7Path = "regionSize7";
constexpr auto settingRegionSize8Path = "regionSize8";
//Export data
constexpr auto settingWriteDataToDiscPath = "writeDataToDisc";
constexpr auto settingSplitModePath = "splitMode";
constexpr auto settingFilePathPath = "filePath";
//DAC
constexpr auto settingDacCameraChannel1Path = "dacCameraChannel1";
constexpr auto settingDacCameraChannel2Path = "dacCameraChannel2";
constexpr auto settingDacCameraChannel3Path = "dacCameraChannel3";
constexpr auto settingDacCameraChannel4Path = "dacCameraChannel4";
constexpr auto settingDacCameraChannel5Path = "dacCameraChannel5";
constexpr auto settingDacCameraChannel6Path = "dacCameraChannel6";
constexpr auto settingDacCameraChannel7Path = "dacCameraChannel7";
constexpr auto settingDacCameraChannel8Path = "dacCameraChannel8";
constexpr auto settingDacPcieChannel1Path = "dacPcieChannel1";
constexpr auto settingDacPcieChannel2Path = "dacPcieChannel2";
constexpr auto settingDacPcieChannel3Path = "dacPcieChannel3";
constexpr auto settingDacPcieChannel4Path = "dacPcieChannel4";
constexpr auto settingDacPcieChannel5Path = "dacPcieChannel5";
constexpr auto settingDacPcieChannel6Path = "dacPcieChannel6";
constexpr auto settingDacPcieChannel7Path = "dacPcieChannel7";
constexpr auto settingDacPcieChannel8Path = "dacPcieChannel8";

//debug
constexpr auto settingTorPath = "tor";
constexpr auto settingAdcModePath = "adcMode";
constexpr auto settingAdcCustomValuePath = "adcCustomValue";
//appearance
constexpr auto settingThemePath = "theme";
constexpr auto settingSettingsLevelPath = "settingsLevel";
constexpr auto settingShowCameraBaseDir = "showcamera";
//io control
constexpr auto settingIOCtrlOutput1DelayIn5nsPath = "Output1DelayIn5ns";
constexpr auto settingIOCtrlOutput1WidthIn5nsPath = "Output1WidthIn5ns";
constexpr auto settingIOCtrlOutput2DelayIn5nsPath = "Output2DelayIn5ns";
constexpr auto settingIOCtrlOutput2WidthIn5nsPath = "Output2WidthIn5ns";
constexpr auto settingIOCtrlOutput3DelayIn5nsPath = "Output3DelayIn5ns";
constexpr auto settingIOCtrlOutput3WidthIn5nsPath = "Output3WidthIn5ns";
constexpr auto settingIOCtrlOutput4DelayIn5nsPath = "Output4DelayIn5ns";
constexpr auto settingIOCtrlOutput4WidthIn5nsPath = "Output4WidthIn5ns";
constexpr auto settingIOCtrlOutput5DelayIn5nsPath = "Output5DelayIn5ns";
constexpr auto settingIOCtrlOutput5WidthIn5nsPath = "Output5WidthIn5ns";
constexpr auto settingIOCtrlOutput6DelayIn5nsPath = "Output6DelayIn5ns";
constexpr auto settingIOCtrlOutput6WidthIn5nsPath = "Output6WidthIn5ns";
constexpr auto settingIOCtrlOutput7WidthIn5nsPath = "Output7DelayIn5ns";
constexpr auto settingIOCtrlOutput7DelayIn5nsPath = "Output7WidthIn5ns";
constexpr auto settingIOCtrlT0PeriodIn10nsPath = "T0PeriodIn10ns";
//software
constexpr auto settingsUseSoftwarePollingPath = "use_software_polling";

namespace Ui
{
	class DialogSettings;
}

class DialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSettings(QWidget *parent = nullptr);
	~DialogSettings();
signals:
	void settings_saved();
	void defaults_loaded();
	void initializingDone();
private:
	Ui::DialogSettings *ui;
	QSettings settings;
private slots:
	void loadDefaults();
	void on_accepted();
	void on_pushButtonDefault_clicked();
	void on_comboBoxSettingsLevel_currentIndexChanged(int index);
	void on_checkBoxBoard0_stateChanged(int state);
	void on_checkBoxBoard1_stateChanged(int state);
	void on_checkBoxBoard2_stateChanged(int state);
	void on_checkBoxBoard3_stateChanged(int state);
	void on_checkBoxBoard4_stateChanged(int state);
};

#endif // DIALOGSETTINGS_H
