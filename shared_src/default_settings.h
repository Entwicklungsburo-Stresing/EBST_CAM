/*****************************************************************//**
 * @file		default_settings.h
 * @brief		Default settings for initializing the settings struct and the settings dialog.
 * @author		Florian Hahn
 * @date		24.06.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include "../shared_src/enum_settings.h"

#define settingsUseSoftwarePollingDefault			false
#define settingNosDefault							1000
#define settingNobDefault							1
#define settingStiDefault							sti_STimer
#define settingBtiDefault							bti_BTimer
#define settingStime_Default						1000
#define settingBtimeDefault							1000000
#define settingSdat_in_10nsDefault					0
#define settingBdat_in_10nsDefault					0
#define settingSslopeDefault						sslope_pos
#define settingBslopeDefault						bslope_pos
#define settingXckdelayIn10nsDefault				0
#define settingShutterSecIn10nsDefault				0
#define settingShutterBecIn10nsDefault				0
#define settingTriggerModeIntegratorDefault			xck
#define settingBoardSelDefault						1
#define settingBoard0Default						1
#define settingBoard1Default						0
#define settingBoard2Default						0
#define settingBoard3Default						0
#define settingBoard4Default						0
#define settingSensorTypeDefault					sensor_type_pda
#define settingIsFftlegacyDefault					0
#define settingCameraSystemDefault					camera_system_3001
#define settingCamcntDefault						1
#define settingPixelDefault							1088
#define settingLedDefault							false
#define settingSensorGainDefault					0
#define settingAdcGainDefault						0
#define settingCoolingDefault						0
#define settingGpxOffsetDefault						1000
#define settingLinesDefault							64
#define settingVfreqDefault							7
#define settingFftModeDefault						full_binning
#define settingLinesBinningDefault					1
#define settingNumberOfRegionsDefault				3
#define settingS1S2ReadDelayIn10nsDefault			0
#define settingRegionSizeEqualDefault				true
#define settingRegionSize1Default					10
#define settingRegionSize2Default					44
#define settingRegionSize3Default					10
#define settingRegionSize4Default					0
#define settingRegionSize5Default					0
#define settingRegionSize6Default					0
#define settingRegionSize7Default					0
#define settingRegionSize8Default					0
#define settingDacCameraDefault						55000
#define settingDacPcieDefault						20000
#define settingTorDefault							tor_xck
#define settingMonitorDefault						monitor_xck
#define settingAdcModeDefault						normal
#define settingAdcCustomValueDefault				0
#define settingThemeIndexDefault					0
#define settingThemeDefault							"windows11"
#define settingSettingsLevelDefault					settings_level_guided
#define settingBecIn10nsDefault						0
#define settingShowCameraDefault					1
#define settingIOCtrlImpactStartPixelDefault		1078
#define settingIOCtrlOutput1WidthIn5nsDefault		50
#define settingIOCtrlOutput2WidthIn5nsDefault		50
#define settingIOCtrlOutput3WidthIn5nsDefault		50
#define settingIOCtrlOutput4WidthIn5nsDefault		50
#define settingIOCtrlOutput5WidthIn5nsDefault		50
#define settingIOCtrlOutput6WidthIn5nsDefault		50
#define settingIOCtrlOutput7WidthIn5nsDefault		50
#define settingIOCtrlOutput1DelayIn5nsDefault		0
#define settingIOCtrlOutput2DelayIn5nsDefault		100
#define settingIOCtrlOutput3DelayIn5nsDefault		200
#define settingIOCtrlOutput4DelayIn5nsDefault		300
#define settingIOCtrlOutput5DelayIn5nsDefault		400
#define settingIOCtrlOutput6DelayIn5nsDefault		500
#define settingIOCtrlOutput7DelayIn5nsDefault		600
#define settingIOCtrlT0PeriodIn10nsDefault			1000
#define settingContinuousMeasurementDefault			0
#define settingContinuousPausInMicrosecondsDefault	0
#define settingDmaBufferSizeInScansDefault			1000
#define settingSticntDefault						0
#define settingBticntDefault						0
#define settingTocntDefault							0
#define settingSensorResetOrHsIrDefault				100
#define settingWriteToDiscDefault					0
#define settingFilePathDefault						""
#define settingShiftS1S2ToNextScanDefault			false
#define settingIsCooledCameraLegacyModeDefault		false
#define settingAxesMirrorXPathDefault				false
#define settingShowCrosshairDefault					false
#define settingChannelSelectDefault					channel_select_A_B
#define settingColorSchemeDefault					0
#define settingManipulateDataModeDefault			manipulate_data_mode_none
#define settingManipulateDataCustomFactorDefault	1
#define settingEcLegacyModeDefault					false
#define settingTimerResolutionModeDefault			timer_resolution_1us
