/*****************************************************************//**
 * @file   globals.c
 * @copydoc globals.h
 *********************************************************************/

#include "globals.h"
#include "../shared_src/default_settings.h"

uint32_t tmp_virtualCamcnt[MAXPCIECARDS] = { 1, 1, 1, 1, 1 };
/**
 * virtualCamcnt is either equal to the setting @ref camera_settings.camcnt or 1 if this setting is 0.
 */
uint32_t* virtualCamcnt = tmp_virtualCamcnt;
uint16_t* temp_userBuffer[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
uint16_t** userBuffer= temp_userBuffer;
uint16_t* temp_userBufferEndPtr[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
uint16_t** userBufferEndPtr = temp_userBufferEndPtr;
uint16_t* temp_userBufferWritePos[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
uint16_t** userBufferWritePos = temp_userBufferWritePos;
uint16_t* temp_userBufferWritePos_last[MAXPCIECARDS] = { NULL, NULL, NULL, NULL, NULL };
uint16_t** userBufferWritePos_last = temp_userBufferWritePos_last;
uint16_t temp_pcieCardMajorVersion[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
uint16_t* pcieCardMajorVersion = temp_pcieCardMajorVersion;
uint16_t temp_pcieCardMinorVersion[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
uint16_t* pcieCardMinorVersion = temp_pcieCardMinorVersion;
volatile bool temp_timerOn[MAXPCIECARDS] = { false, false, false, false, false };
volatile bool* timerOn = temp_timerOn;
uint32_t temp_numberOfInterrupts[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
volatile uint32_t* numberOfInterrupts = temp_numberOfInterrupts;
uint8_t number_of_boards = 0;
bool testModeOn = false;
volatile bool abortMeasurementFlag = false;
volatile bool continuousMeasurementFlag = false;
bool isRunning = false;
int64_t temp_scanCounterTotal[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
int64_t* scanCounterTotal = temp_scanCounterTotal;
uint64_t measurement_cnt = 0;
char start_timestamp[file_timestamp_size];
size_t temp_data_available[MAXPCIECARDS] = { 0, 0, 0, 0, 0 };
volatile size_t* data_available = temp_data_available;
volatile bool tmp_allInterruptsDone[MAXPCIECARDS] = { true, true, true, true, true };
volatile bool* allInterruptsDone = tmp_allInterruptsDone;
hookFunction measureStartHook = NULL;
hookFunction measureDoneHook = NULL;
hookFunctionUint32 blockStartHook = NULL;
hookFunctionUint32 blockDoneHook = NULL;
hookFunctionUint64 allBlocksDoneHook = NULL;

const struct camera_settings camera_settings_default =
{
	// The order is the same as in struct.h
	.use_software_polling = settingsUseSoftwarePollingDefault,
	.sti_mode = settingStiDefault,
	.bti_mode = settingBtiDefault,
	.stime = settingStime_Default,
	.btime = settingBtimeDefault,
	.sdat_in_10ns = settingSdat_in_10nsDefault,
	.bdat_in_10ns = settingSdat_in_10nsDefault,
	.sslope = settingSslopeDefault,
	.bslope = settingBslopeDefault,
	.xckdelay_in_10ns = settingXckdelayIn10nsDefault,
	.sec_in_10ns = settingShutterSecIn10nsDefault,
	.trigger_mode_integrator = settingTriggerModeIntegratorDefault,
	.sensor_type = settingSensorTypeDefault,
	.camera_system = settingCameraSystemDefault,
	.camcnt = settingCamcntDefault,
	.pixel = settingPixelDefault,
	.is_fft_legacy = settingIsFftlegacyDefault,
	.led_off = settingLedDefault,
	.sensor_gain = settingSensorGainDefault,
	.adc_gain = settingAdcGainDefault,
	.temp_level = settingCoolingDefault,
	.bticnt = settingBticntDefault,
	.gpx_offset = settingGpxOffsetDefault,
	.fft_lines = settingLinesDefault,
	.vfreq = settingVfreqDefault,
	.fft_mode = settingFftModeDefault,
	.lines_binning = settingLinesBinningDefault,
	.number_of_regions = settingNumberOfRegionsDefault,
	.s1s2_read_delay_in_10ns = settingS1S2ReadDelayIn10nsDefault,
	.region_size[0] = settingRegionSize1Default,
	.region_size[1] = settingRegionSize2Default,
	.region_size[2] = settingRegionSize3Default,
	.region_size[3] = settingRegionSize4Default,
	.region_size[4] = settingRegionSize5Default,
	.region_size[5] = settingRegionSize6Default,
	.region_size[6] = settingRegionSize7Default,
	.region_size[7] = settingRegionSize8Default,
	.dac_output[0][0] = settingDacCameraDefault,
	.dac_output[0][1] = settingDacCameraDefault,
	.dac_output[0][2] = settingDacCameraDefault,
	.dac_output[0][3] = settingDacCameraDefault,
	.dac_output[0][4] = settingDacCameraDefault,
	.dac_output[0][5] = settingDacCameraDefault,
	.dac_output[0][6] = settingDacCameraDefault,
	.dac_output[0][7] = settingDacCameraDefault,
	.dac_output[1][0] = settingDacCameraDefault,
	.dac_output[1][1] = settingDacCameraDefault,
	.dac_output[1][2] = settingDacCameraDefault,
	.dac_output[1][3] = settingDacCameraDefault,
	.dac_output[1][4] = settingDacCameraDefault,
	.dac_output[1][5] = settingDacCameraDefault,
	.dac_output[1][6] = settingDacCameraDefault,
	.dac_output[1][7] = settingDacCameraDefault,
	.dac_output[2][0] = settingDacCameraDefault,
	.dac_output[2][1] = settingDacCameraDefault,
	.dac_output[2][2] = settingDacCameraDefault,
	.dac_output[2][3] = settingDacCameraDefault,
	.dac_output[2][4] = settingDacCameraDefault,
	.dac_output[2][5] = settingDacCameraDefault,
	.dac_output[2][6] = settingDacCameraDefault,
	.dac_output[2][7] = settingDacCameraDefault,
	.dac_output[3][0] = settingDacCameraDefault,
	.dac_output[3][1] = settingDacCameraDefault,
	.dac_output[3][2] = settingDacCameraDefault,
	.dac_output[3][3] = settingDacCameraDefault,
	.dac_output[3][4] = settingDacCameraDefault,
	.dac_output[3][5] = settingDacCameraDefault,
	.dac_output[3][6] = settingDacCameraDefault,
	.dac_output[3][7] = settingDacCameraDefault,
	.dac_output[4][0] = settingDacCameraDefault,
	.dac_output[4][1] = settingDacCameraDefault,
	.dac_output[4][2] = settingDacCameraDefault,
	.dac_output[4][3] = settingDacCameraDefault,
	.dac_output[4][4] = settingDacCameraDefault,
	.dac_output[4][5] = settingDacCameraDefault,
	.dac_output[4][6] = settingDacCameraDefault,
	.dac_output[4][7] = settingDacCameraDefault,
	.dac_output[5][0] = settingDacCameraDefault,
	.dac_output[5][1] = settingDacCameraDefault,
	.dac_output[5][2] = settingDacCameraDefault,
	.dac_output[5][3] = settingDacCameraDefault,
	.dac_output[5][4] = settingDacCameraDefault,
	.dac_output[5][5] = settingDacCameraDefault,
	.dac_output[5][6] = settingDacCameraDefault,
	.dac_output[5][7] = settingDacCameraDefault,
	.dac_output[6][0] = settingDacCameraDefault,
	.dac_output[6][1] = settingDacCameraDefault,
	.dac_output[6][2] = settingDacCameraDefault,
	.dac_output[6][3] = settingDacCameraDefault,
	.dac_output[6][4] = settingDacCameraDefault,
	.dac_output[6][5] = settingDacCameraDefault,
	.dac_output[6][6] = settingDacCameraDefault,
	.dac_output[6][7] = settingDacCameraDefault,
	.dac_output[7][0] = settingDacCameraDefault,
	.dac_output[7][1] = settingDacCameraDefault,
	.dac_output[7][2] = settingDacCameraDefault,
	.dac_output[7][3] = settingDacCameraDefault,
	.dac_output[7][4] = settingDacCameraDefault,
	.dac_output[7][5] = settingDacCameraDefault,
	.dac_output[7][6] = settingDacCameraDefault,
	.dac_output[7][7] = settingDacCameraDefault,
	.tor = settingTorDefault,
	.adc_mode = settingAdcModeDefault,
	.adc_custom_pattern = settingAdcCustomValueDefault,
	.bec_in_10ns = settingBecIn10nsDefault,
	.channel_select = settingChannelSelectDefault,
	.ioctrl_impact_start_pixel = settingIOCtrlImpactStartPixelDefault,
	.ioctrl_output_width_in_5ns[0] = settingIOCtrlOutput1WidthIn5nsDefault,
	.ioctrl_output_width_in_5ns[1] = settingIOCtrlOutput2WidthIn5nsDefault,
	.ioctrl_output_width_in_5ns[2] = settingIOCtrlOutput3WidthIn5nsDefault,
	.ioctrl_output_width_in_5ns[3] = settingIOCtrlOutput4WidthIn5nsDefault,
	.ioctrl_output_width_in_5ns[4] = settingIOCtrlOutput5WidthIn5nsDefault,
	.ioctrl_output_width_in_5ns[5] = settingIOCtrlOutput6WidthIn5nsDefault,
	.ioctrl_output_width_in_5ns[6] = settingIOCtrlOutput7WidthIn5nsDefault,
	.ioctrl_output_delay_in_5ns[0] = settingIOCtrlOutput1DelayIn5nsDefault,
	.ioctrl_output_delay_in_5ns[1] = settingIOCtrlOutput2DelayIn5nsDefault,
	.ioctrl_output_delay_in_5ns[2] = settingIOCtrlOutput3DelayIn5nsDefault,
	.ioctrl_output_delay_in_5ns[3] = settingIOCtrlOutput4DelayIn5nsDefault,
	.ioctrl_output_delay_in_5ns[4] = settingIOCtrlOutput5DelayIn5nsDefault,
	.ioctrl_output_delay_in_5ns[5] = settingIOCtrlOutput6DelayIn5nsDefault,
	.ioctrl_output_delay_in_5ns[6] = settingIOCtrlOutput7DelayIn5nsDefault,
	.ioctrl_T0_period_in_10ns = settingIOCtrlT0PeriodIn10nsDefault,
	.dma_buffer_size_in_scans = settingDmaBufferSizeInScansDefault,
	.tocnt = settingTocntDefault,
	.sticnt = settingSticntDefault,
	.sensor_reset_or_hsir_ec = settingSensorResetOrHsIrDefault,
	.write_to_disc = settingWriteToDiscDefault,
	.file_path = settingFilePathDefault,
	.shift_s1s2_to_next_scan = settingShiftS1S2ToNextScanDefault,
	.is_cooled_camera_legacy_mode = settingIsCooledCameraLegacyModeDefault,
	.monitor = settingMonitorDefault,
	.manipulate_data_mode = settingManipulateDataModeDefault,
	.manipulate_data_custom_factor = settingManipulateDataCustomFactorDefault,
	.ec_legacy_mode = settingEcLegacyModeDefault,
	.timer_resolution_mode = settingTimerResolutionModeDefault,
};

struct measurement_settings settings_struct =
{
	.board_sel						= settingBoardSelDefault,
	.nos							= settingNosDefault,
	.nob							= settingNobDefault,
	.continuous_measurement			= settingContinuousMeasurementDefault,
	.cont_pause_in_microseconds		= settingContinuousPausInMicrosecondsDefault,
};
