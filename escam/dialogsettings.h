/*****************************************************************//**
 * @file   dialogsettings.h
 * @brief  Dialog for setting the measurement settings.
 * 
 * @author Florian Hahn
 * @date   03.02.2021
 *********************************************************************/

#pragma once

#include <QDialog>
#include <QSettings>
#include "lsc.h"
#include "../shared_src/default_settings.h"

// The order is the same as in dialogsettings.ui and camerasettingswidget.ui
// measurement settings
constexpr auto settingBoardSelPath = "boardSel";
constexpr auto settingBoard0Path = "board0";
constexpr auto settingBoard1Path = "board1";
constexpr auto settingBoard2Path = "board2";
constexpr auto settingBoard3Path = "board3";
constexpr auto settingBoard4Path = "board4";
constexpr auto settingNosPath = "nos";
constexpr auto settingNobPath = "nob";
constexpr auto settingContinuousPauseInMicrosecondsPath = "contPauseInMicroseconds";
constexpr auto settingThemePath = "theme";
constexpr auto settingThemeIndexPath = "themeIndex";
constexpr auto settingSettingsLevelPath = "settingsLevel";
constexpr auto settingColorSchemePath = "colorScheme";
// camera settings
// measurement
constexpr auto settingStiPath = "sti";
constexpr auto settingBtiPath = "bti";
constexpr auto settingSslopePath = "sslope";
constexpr auto settingBslopePath = "bslope";
constexpr auto settingStimePath = "stimer";
constexpr auto settingStimeResolutionModePath = "stimeResolutionMode";
constexpr auto settingBtime_in_microseconds_Path = "btimer";
constexpr auto settingSdat_in_10nsPath = "sdat";
constexpr auto settingBdat_in_10nsPath = "bdat";
constexpr auto settingShutterSecIn10nsPath = "shutterSecIn10ns";
constexpr auto settingShutterBecIn10nsPath = "shutterBecIn10ns";
constexpr auto settingSticntPath = "sticnt";
constexpr auto settingBticntPath = "bticnt";
constexpr auto settingTocntPath = "tocnt";
constexpr auto settingTriggerModeIntegratorPath = "triggerModeIntegrator";
constexpr auto settingXckdelayIn10nsPath = "xckdelay_in_10ns";
constexpr auto settingS1S2ReadDelayIn10nsPath = "S1S2ReadDelayIn10ns";
// camera setup
constexpr auto settingCameraSystemPath = "cameraSystem";
constexpr auto settingSensorTypePath = "sensorType";
constexpr auto settingIsFftLegacyPath = "isFftLegacy";
constexpr auto settingCamcntPath = "camcnt";
constexpr auto settingPixelPath = "pixelcnt";
constexpr auto settingLedPath = "led";
constexpr auto settingSensorGainPath = "sensorGain";
constexpr auto settingAdcGainPath = "adcGain";
constexpr auto settingIsCooledCameraLegacyModePath = "isCooledCameraLegacyMode";
constexpr auto settingCoolingPath = "cooling";
constexpr auto settingGpxOffsetPath = "gpxOffset";
constexpr auto settingIOCtrlImpactStartPixelPath = "IOCtrlImpactStartPixel";
constexpr auto settingsUseSoftwarePollingPath = "use_software_polling";
constexpr auto settingSensorResetOrHsirEcPath = "sensorResetOrHsirEc";
constexpr auto settingChannelSelectPath = "channelSelect";
constexpr auto settingShiftS1S2ToNextScanPath = "shiftS1S2ToNextScan";
// FFT mode
constexpr auto settingLinesPath = "lines";
constexpr auto settingVfreqPath = "vfreq";
constexpr auto settingFftModePath = "fftmode";
constexpr auto settingLinesBinningPath = "linesbinning";
constexpr auto settingNumberOfRegionsPath = "numberOfRegions";
constexpr auto settingRegionSizeEqualPath = "regionSizeEqual";
constexpr auto settingRegionSize1Path = "regionSize1";
constexpr auto settingRegionSize2Path = "regionSize2";
constexpr auto settingRegionSize3Path = "regionSize3";
constexpr auto settingRegionSize4Path = "regionSize4";
constexpr auto settingRegionSize5Path = "regionSize5";
constexpr auto settingRegionSize6Path = "regionSize6";
constexpr auto settingRegionSize7Path = "regionSize7";
constexpr auto settingRegionSize8Path = "regionSize8";
// other
constexpr auto settingMonitorPath = "monitor";
constexpr auto settingTorPath = "tor";
constexpr auto settingAdcModePath = "adcMode";
constexpr auto settingAdcCustomValuePath = "adcCustomValue";
constexpr auto settingWriteDataToDiscPath = "writeDataToDisc";
constexpr auto settingFilePathPath = "filePath";
constexpr auto settingManipulateDataModePath = "manipulateDataMode";
constexpr auto settingManipulateDataCustomFactorPath = "manipulateDataCustomFactor";
constexpr auto settingEcLegacyModePath = "ecLegacyMode";
// DAC
constexpr auto settingDacCameraChannel1Path = "dacCameraChannel1Pos";
constexpr auto settingDacCameraChannel2Path = "dacCameraChannel2Pos";
constexpr auto settingDacCameraChannel3Path = "dacCameraChannel3Pos";
constexpr auto settingDacCameraChannel4Path = "dacCameraChannel4Pos";
constexpr auto settingDacCameraChannel5Path = "dacCameraChannel5Pos";
constexpr auto settingDacCameraChannel6Path = "dacCameraChannel6Pos";
constexpr auto settingDacCameraChannel7Path = "dacCameraChannel7Pos";
constexpr auto settingDacCameraChannel8Path = "dacCameraChannel8Pos";
constexpr auto settingDacCameraChannelBaseDir = "dacCameraChannel";
constexpr auto settingDacPcieChannel1Path = "dacPcieChannel1";
constexpr auto settingDacPcieChannel2Path = "dacPcieChannel2";
constexpr auto settingDacPcieChannel3Path = "dacPcieChannel3";
constexpr auto settingDacPcieChannel4Path = "dacPcieChannel4";
constexpr auto settingDacPcieChannel5Path = "dacPcieChannel5";
constexpr auto settingDacPcieChannel6Path = "dacPcieChannel6";
constexpr auto settingDacPcieChannel7Path = "dacPcieChannel7";
constexpr auto settingDacPcieChannel8Path = "dacPcieChannel8";
// IO control
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
// chart
constexpr auto settingAxesMirrorXPath = "AxesMirrorXPath";
constexpr auto settingShowCrosshairPath = "showCrosshair";
constexpr auto settingSoftwareVersionPath = "softwareVersion";
constexpr auto settingShowCameraBaseDir = "showcamera";
// Shutter
constexpr auto settingShutter1Path = "shutter1";
constexpr auto settingShutter2Path = "shutter2";
constexpr auto settingShutter3Path = "shutter3";
constexpr auto settingShutter4Path = "shutter4";
constexpr auto settingShutterMshutPath = "shutterMshut";

QT_BEGIN_NAMESPACE
namespace Ui { class DialogSettings; }
QT_END_NAMESPACE

class DialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSettings(QWidget *parent = nullptr);
	~DialogSettings();
	Ui::DialogSettings* ui;
signals:
	void settings_saved();
	void defaults_loaded();
	void initializingDone();
private:
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
