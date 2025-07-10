/*****************************************************************//**
 * @file		dialogsettings.h
 * @brief		Dialog for setting the measurement settings.
 * @author		Florian Hahn
 * @date		03.02.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include <QSettings>
#include "lsc.h"
#include "../shared_src/default_settings.h"

// The order is the same as in dialogsettings.ui and camerasettingswidget.ui
// measurement settings
constexpr auto settingBoardSelPath = "board_sel";
constexpr auto settingBoard0Path = "board0";
constexpr auto settingBoard1Path = "board1";
constexpr auto settingBoard2Path = "board2";
constexpr auto settingBoard3Path = "board3";
constexpr auto settingBoard4Path = "board4";
constexpr auto settingNosPath = "nos";
constexpr auto settingNobPath = "nob";
constexpr auto settingContinuousPauseInMicrosecondsPath = "cont_pause_in_microseconds";
constexpr auto settingThemePath = "theme";
constexpr auto settingThemeIndexPath = "themeIndex";
constexpr auto settingSettingsLevelPath = "settingsLevel";
constexpr auto settingColorSchemePath = "colorScheme";
// camera settings
// measurement
constexpr auto settingStiPath = "sti_mode";
constexpr auto settingBtiPath = "bti_mode";
constexpr auto settingSslopePath = "sslope";
constexpr auto settingBslopePath = "bslope";
constexpr auto settingStimePath = "stime";
constexpr auto settingTimerResolutionModePath = "timer_resolution_mode";
constexpr auto settingBtimePath = "btime";
constexpr auto settingSdat_in_10nsPath = "sdat_in_10ns";
constexpr auto settingBdat_in_10nsPath = "bdat_in_10ns";
constexpr auto settingShutterSecIn10nsPath = "sec_in_10ns";
constexpr auto settingShutterBecIn10nsPath = "bec_in_10ns";
constexpr auto settingSticntPath = "sticnt";
constexpr auto settingBticntPath = "bticnt";
constexpr auto settingTocntPath = "tocnt";
constexpr auto settingTriggerModeIntegratorPath = "trigger_mode_integrator";
constexpr auto settingXckdelayIn10nsPath = "xckdelay_in_10ns";
constexpr auto settingS1S2ReadDelayIn10nsPath = "s1s2_read_delay_in_10ns";
// camera setup
constexpr auto settingCameraSystemPath = "camera_system";
constexpr auto settingSensorTypePath = "sensor_type";
constexpr auto settingIsFftLegacyPath = "is_fft_legacy";
constexpr auto settingCamcntPath = "camcnt";
constexpr auto settingPixelPath = "pixel";
constexpr auto settingLedPath = "led_off";
constexpr auto settingSensorGainPath = "sensor_gain";
constexpr auto settingAdcGainPath = "adc_gain";
constexpr auto settingIsCooledCameraLegacyModePath = "is_cooled_camera_legacy_mode";
constexpr auto settingCoolingPath = "temp_level";
constexpr auto settingGpxOffsetPath = "gpx_offset";
constexpr auto settingIOCtrlImpactStartPixelPath = "ioctrl_impact_start_pixel";
constexpr auto settingsUseSoftwarePollingPath = "use_software_polling";
constexpr auto settingSensorResetOrHsirEcPath = "sensor_reset_or_hsir_ec";
constexpr auto settingChannelSelectPath = "channel_select";
constexpr auto settingShiftS1S2ToNextScanPath = "shift_s1s2_to_next_scan";
// FFT mode
constexpr auto settingLinesPath = "fft_lines";
constexpr auto settingVfreqPath = "vfreq";
constexpr auto settingFftModePath = "fft_mode";
constexpr auto settingLinesBinningPath = "lines_binning";
constexpr auto settingNumberOfRegionsPath = "number_of_regions";
constexpr auto settingRegionSizeEqualPath = "regionSizeEqual";
constexpr auto settingRegionSize1Path = "region_size1";
constexpr auto settingRegionSize2Path = "region_size2";
constexpr auto settingRegionSize3Path = "region_size3";
constexpr auto settingRegionSize4Path = "region_size4";
constexpr auto settingRegionSize5Path = "region_size5";
constexpr auto settingRegionSize6Path = "region_size6";
constexpr auto settingRegionSize7Path = "region_size7";
constexpr auto settingRegionSize8Path = "region_size8";
// other
constexpr auto settingMonitorPath = "monitor";
constexpr auto settingTorPath = "tor";
constexpr auto settingAdcModePath = "adc_mode";
constexpr auto settingAdcCustomValuePath = "adc_custom_pattern";
constexpr auto settingWriteDataToDiscPath = "write_to_disc";
constexpr auto settingFilePathPath = "file_path";
constexpr auto settingManipulateDataModePath = "manipulate_data_mode";
constexpr auto settingManipulateDataCustomFactorPath = "manipulate_data_custom_factor";
constexpr auto settingEcLegacyModePath = "ec_legacy_mode";
// DAC
constexpr auto settingDacCameraChannel1Path = "dac_output1Pos";
constexpr auto settingDacCameraChannel2Path = "dac_output2Pos";
constexpr auto settingDacCameraChannel3Path = "dac_output3Pos";
constexpr auto settingDacCameraChannel4Path = "dac_output4Pos";
constexpr auto settingDacCameraChannel5Path = "dac_output5Pos";
constexpr auto settingDacCameraChannel6Path = "dac_output6Pos";
constexpr auto settingDacCameraChannel7Path = "dac_output7Pos";
constexpr auto settingDacCameraChannel8Path = "dac_output8Pos";
constexpr auto settingDacCameraChannelBaseDir = "dac_output";
constexpr auto settingDacPcieChannel1Path = "dacPcieChannel1";
constexpr auto settingDacPcieChannel2Path = "dacPcieChannel2";
constexpr auto settingDacPcieChannel3Path = "dacPcieChannel3";
constexpr auto settingDacPcieChannel4Path = "dacPcieChannel4";
constexpr auto settingDacPcieChannel5Path = "dacPcieChannel5";
constexpr auto settingDacPcieChannel6Path = "dacPcieChannel6";
constexpr auto settingDacPcieChannel7Path = "dacPcieChannel7";
constexpr auto settingDacPcieChannel8Path = "dacPcieChannel8";
// IO control
constexpr auto settingIOCtrlOutput1DelayIn5nsPath = "ioctrl_output_delay_in_5ns_1";
constexpr auto settingIOCtrlOutput1WidthIn5nsPath = "ioctrl_output_width_in_5ns_1";
constexpr auto settingIOCtrlOutput2DelayIn5nsPath = "ioctrl_output_delay_in_5ns_2";
constexpr auto settingIOCtrlOutput2WidthIn5nsPath = "ioctrl_output_width_in_5ns_2";
constexpr auto settingIOCtrlOutput3DelayIn5nsPath = "ioctrl_output_delay_in_5ns_3";
constexpr auto settingIOCtrlOutput3WidthIn5nsPath = "ioctrl_output_width_in_5ns_3";
constexpr auto settingIOCtrlOutput4DelayIn5nsPath = "ioctrl_output_delay_in_5ns_4";
constexpr auto settingIOCtrlOutput4WidthIn5nsPath = "ioctrl_output_width_in_5ns_4";
constexpr auto settingIOCtrlOutput5DelayIn5nsPath = "ioctrl_output_delay_in_5ns_5";
constexpr auto settingIOCtrlOutput5WidthIn5nsPath = "ioctrl_output_width_in_5ns_5";
constexpr auto settingIOCtrlOutput6DelayIn5nsPath = "ioctrl_output_delay_in_5ns_6";
constexpr auto settingIOCtrlOutput6WidthIn5nsPath = "ioctrl_output_width_in_5ns_6";
constexpr auto settingIOCtrlOutput7WidthIn5nsPath = "ioctrl_output_delay_in_5ns_7";
constexpr auto settingIOCtrlOutput7DelayIn5nsPath = "ioctrl_output_width_in_5ns_7";
constexpr auto settingIOCtrlT0PeriodIn10nsPath = "ioctrl_T0_period_in_10ns";
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
