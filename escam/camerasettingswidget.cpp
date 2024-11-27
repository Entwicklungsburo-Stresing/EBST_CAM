#include "camerasettingswidget.h"
#include "dialogsettings.h"
#include "ui_dialogsettings.h"

CameraSettingsWidget::CameraSettingsWidget(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::CameraSettingsWidgetClass())
{
	ui->setupUi(this);
#if __linux__
	// disable option software polling on Linux
	ui->checkBoxUseSoftwarePolling->setChecked(false);
	ui->checkBoxUseSoftwarePolling->setEnabled(false);
#endif
}

CameraSettingsWidget::~CameraSettingsWidget()
{}

void CameraSettingsWidget::on_accepted()
{
	// Here the settings on the UI are saved to the system
	// The order is the same as in the widget camerasettingswidget.ui and dialogsettings.h
	// Measurement
	settings.setValue(settingStiPath, ui->comboBoxSti->currentIndex());
	settings.setValue(settingBtiPath, ui->comboBoxBti->currentIndex());
	settings.setValue(settingSslopePath, ui->comboBoxSslope->currentIndex());
	settings.setValue(settingBslopePath, ui->comboBoxBslope->currentIndex());
	settings.setValue(settingStime_in_microseconds_Path, ui->doubleSpinBoxSTime_in_us->value());
	settings.setValue(settingBtime_in_microseconds_Path, ui->doubleSpinBoxBTimer_in_us->value());
	settings.setValue(settingSdat_in_10nsPath, ui->doubleSpinBoxSdatIn10ns->value());
	settings.setValue(settingBdat_in_10nsPath, ui->doubleSpinBoxBdatIn10ns->value());
	settings.setValue(settingShutterSecIn10nsPath, ui->doubleSpinBoxSecIn10ns->value());
	settings.setValue(settingShutterBecIn10nsPath, ui->doubleSpinBoxBecIn10ns->value());
	settings.setValue(settingSticntPath, ui->spinBoxSticnt->value());
	settings.setValue(settingBticntPath, ui->spinBoxBticnt->value());
	settings.setValue(settingTocntPath, ui->spinBoxTocnt->value());
	settings.setValue(settingTriggerModeIntegratorPath, ui->comboBoxTriggerModeIntegrator->currentIndex());
	settings.setValue(settingXckdelayIn10nsPath, ui->doubleSpinBoxXckdelayIn10ns->value());
	settings.setValue(settingS1S2ReadDelayIn10nsPath, ui->doubleSpinBoxS1S2ReadDelayIn10ns->value());
	//Camera setup
	settings.setValue(settingCameraSystemPath, ui->comboBoxCameraSystem->currentIndex());
	settings.setValue(settingSensorTypePath, ui->comboBoxSensorType->currentIndex());
	settings.setValue(settingIsFftLegacyPath, ui->checkBoxIsFftLegacy->isChecked());
	settings.setValue(settingCamcntPath, ui->spinBoxCamcnt->value());
	settings.setValue(settingPixelPath, ui->spinBoxPixel->value());
	settings.setValue(settingLedPath, ui->checkBoxLed->isChecked());
	settings.setValue(settingSensorGainPath, ui->spinBoxSensorGain->value());
	settings.setValue(settingAdcGainPath, ui->spinBoxAdcGain->value());
	settings.setValue(settingIsCooledCameraLegacyModePath, ui->checkBoxIsCooledCameraLegacyMode->isChecked());
	settings.setValue(settingCoolingPath, ui->comboBoxCamCool->currentIndex());
	settings.setValue(settingGpxOffsetPath, ui->spinBoxGpxOffset->value());
	settings.setValue(settingIOCtrlImpactStartPixelPath, ui->spinBoxIOCtrlImpactStartPixel->value());
	settings.setValue(settingsUseSoftwarePollingPath, ui->checkBoxUseSoftwarePolling->isChecked());
	if(settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble() == sensor_type_hsvis)
		settings.setValue(settingSensorResetOrHsirEcPath, ui->spinBoxSensorResetOrHsirEcIn1ns->value() / 4);
	else
		settings.setValue(settingSensorResetOrHsirEcPath, ui->spinBoxSensorResetOrHsirEcIn1ns->value() / 160);
	settings.setValue(settingChannelSelectPath, ui->comboBoxChannelSelect->currentIndex());
	settings.setValue(settingShiftS1S2ToNextScanPath, ui->checkBoxShiftS1S2ToNextScan->isChecked());
	//FFT mode
	settings.setValue(settingLinesPath, ui->spinBoxLines->value());
	settings.setValue(settingVfreqPath, ui->spinBoxVfreq->value());
	settings.setValue(settingFftModePath, ui->comboBoxFftMode->currentIndex());
	settings.setValue(settingLinesBinningPath, ui->spinBoxLinesBinning->value());
	settings.setValue(settingNumberOfRegionsPath, ui->spinBoxNumberOfRegions->value());
	settings.setValue(settingRegionSizeEqualPath, ui->checkBoxRegionsEqual->isChecked());
	settings.setValue(settingRegionSize1Path, ui->spinBoxRegion1->value());
	settings.setValue(settingRegionSize2Path, ui->spinBoxRegion2->value());
	settings.setValue(settingRegionSize3Path, ui->spinBoxRegion3->value());
	settings.setValue(settingRegionSize4Path, ui->spinBoxRegion4->value());
	settings.setValue(settingRegionSize5Path, ui->spinBoxRegion5->value());
	//Other
	settings.setValue(settingMonitorPath, ui->comboBoxMonitor->currentIndex());
	settings.setValue(settingTorPath, ui->comboBoxOutput->currentIndex());
	settings.setValue(settingAdcModePath, ui->comboBoxAdcMode->currentIndex());
	settings.setValue(settingAdcCustomValuePath, ui->spinBoxAdcCustom->value());
	settings.setValue(settingWriteDataToDiscPath, ui->checkBoxWriteDataToDisc->isChecked());
	settings.setValue(settingFilePathPath, ui->plainTextEditFilePath->toPlainText());
	return;
}

void CameraSettingsWidget::on_comboBoxSti_currentIndexChanged(int index)
{
	bool enabled = true;
	switch (_settings_level)
	{
	case settings_level_guided:
		enabled = false;
		break;
	default:
	case settings_level_free:
		enabled = true;
		break;
	}
	switch (index)
	{
	case sti_STimer:
		ui->doubleSpinBoxSTime_in_us->setEnabled(true);
		break;
	default:
		ui->doubleSpinBoxSTime_in_us->setEnabled(enabled);
	}
	switch (index)
	{
	case sti_I:
	case sti_S1:
	case sti_S2:
	case sti_S2_enable_I:
		ui->comboBoxSslope->setEnabled(true);
		break;
	default:
		ui->comboBoxSslope->setEnabled(enabled);
	}
}

void CameraSettingsWidget::on_comboBoxBti_currentIndexChanged(int index)
{
	bool enabled = true;
	switch (_settings_level)
	{
	case settings_level_guided:
		enabled = false;
		break;
	default:
	case settings_level_free:
		enabled = true;
		break;
	}
	switch (index)
	{
	case bti_BTimer:
		ui->doubleSpinBoxBTimer_in_us->setEnabled(true);
		ui->comboBoxBslope->setEnabled(enabled);
		break;
	default:
		ui->doubleSpinBoxBTimer_in_us->setEnabled(enabled);
		ui->comboBoxBslope->setEnabled(true);
	}
}


void CameraSettingsWidget::on_comboBoxSensorType_currentIndexChanged(int index)
{
	bool enabled = false;
	if (index == sensor_type_fft)
	{
		enabled = true;
	}
	switch (_settings_level)
	{
	case settings_level_guided:
		break;
	default:
	case settings_level_free:
		enabled = true;
		break;
	}
	ui->tabWidget->setTabEnabled(2, enabled);
	ui->checkBoxIsFftLegacy->setEnabled(enabled);
	if(!enabled)
		ui->comboBoxFftMode->setCurrentIndex(full_binning);
}

void CameraSettingsWidget::on_comboBoxCameraSystem_currentIndexChanged(int index)
{
	bool enabled = true;
	QVariant enabled_item(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	QVariant disabled_item(0);
	switch (_settings_level)
	{
	case settings_level_guided:
		enabled = false;
		break;
	default:
	case settings_level_free:
		enabled = true;
		disabled_item = enabled_item;
		break;
	}
	switch (index)
	{
	case camera_system_3001:
		ui->spinBoxAdcGain->setEnabled(enabled);
		ui->comboBoxAdcMode->setEnabled(enabled);
		ui->spinBoxAdcCustom->setEnabled(enabled);
		// disable some sensor types
		ui->comboBoxSensorType->setItemData(sensor_type_pda, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_ir, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_fft, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_cmos, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_hsvis, disabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_hsir, disabled_item, Qt::UserRole - 1);
		if(ui->comboBoxSensorType->currentIndex() >= sensor_type_hsvis)
			ui->comboBoxSensorType->setCurrentIndex(sensor_type_pda);
		break;
	case camera_system_3010:
		ui->spinBoxAdcGain->setEnabled(enabled);
		ui->comboBoxAdcMode->setEnabled(true);
		ui->spinBoxAdcCustom->setEnabled(true);
		ui->comboBoxSensorType->setItemData(sensor_type_pda, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_ir, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_fft, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_cmos, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_hsvis, disabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_hsir, disabled_item, Qt::UserRole - 1);
		if (ui->comboBoxSensorType->currentIndex() >= sensor_type_hsvis)
			ui->comboBoxSensorType->setCurrentIndex(sensor_type_pda);
		break;
	case camera_system_3030:
		ui->spinBoxAdcGain->setEnabled(true);
		ui->comboBoxAdcMode->setEnabled(true);
		ui->spinBoxAdcCustom->setEnabled(true);
		ui->comboBoxSensorType->setItemData(sensor_type_pda, disabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_ir, disabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_fft, disabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_cmos, disabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_hsvis, enabled_item, Qt::UserRole - 1);
		ui->comboBoxSensorType->setItemData(sensor_type_hsir, enabled_item, Qt::UserRole - 1);
		if (ui->comboBoxSensorType->currentIndex() <= sensor_type_cmos)
			ui->comboBoxSensorType->setCurrentIndex(sensor_type_hsvis);
		break;
	}
}

void CameraSettingsWidget::on_checkBoxRegionsEqual_stateChanged(int arg1)
{
	(void)arg1;
	on_spinBoxNumberOfRegions_valueChanged(ui->spinBoxNumberOfRegions->value());
}

void CameraSettingsWidget::on_spinBoxNumberOfRegions_valueChanged(int value)
{
	if (_settings_level == settings_level_free)
	{
		ui->spinBoxRegion1->setEnabled(true);
		ui->spinBoxRegion2->setEnabled(true);
		ui->spinBoxRegion3->setEnabled(true);
		ui->spinBoxRegion4->setEnabled(true);
		ui->spinBoxRegion5->setEnabled(true);
	}
	else
	{
		ui->spinBoxRegion1->setEnabled(false);
		ui->spinBoxRegion2->setEnabled(false);
		ui->spinBoxRegion3->setEnabled(false);
		ui->spinBoxRegion4->setEnabled(false);
		ui->spinBoxRegion5->setEnabled(false);
		if (ui->comboBoxFftMode->currentIndex() == partial_binning)
		{
			if (ds)
			{
				ds->ui->doubleSpinBoxNos->setValue(value);
			}
			if (!(ui->checkBoxRegionsEqual->checkState()))
			{
				switch (value)
				{
				case 5:
					ui->spinBoxRegion5->setEnabled(true);
					[[fallthrough]];
				case 4:
					ui->spinBoxRegion4->setEnabled(true);
					[[fallthrough]];
				case 3:
					ui->spinBoxRegion3->setEnabled(true);
					[[fallthrough]];
				case 2:
					ui->spinBoxRegion2->setEnabled(true);
					[[fallthrough]];
				case 1:
					ui->spinBoxRegion1->setEnabled(true);
				}
				switch (value)
				{
				case 1:
					ui->spinBoxRegion2->setValue(0);
					[[fallthrough]];
				case 2:
					ui->spinBoxRegion3->setValue(0);
					[[fallthrough]];
				case 3:
					ui->spinBoxRegion4->setValue(0);
					[[fallthrough]];
				case 4:
					ui->spinBoxRegion5->setValue(0);
				}
			}
			else
			{
				int regionSize = ui->spinBoxLines->value() / value;
				int remainder = ui->spinBoxLines->value() - (regionSize * (value - 1));
				switch (value)
				{
				case 1:
					ui->spinBoxRegion1->setValue(remainder);
					ui->spinBoxRegion2->setValue(0);
					ui->spinBoxRegion3->setValue(0);
					ui->spinBoxRegion4->setValue(0);
					ui->spinBoxRegion5->setValue(0);
					break;
				case 2:
					ui->spinBoxRegion1->setValue(regionSize);
					ui->spinBoxRegion2->setValue(remainder);
					ui->spinBoxRegion3->setValue(0);
					ui->spinBoxRegion4->setValue(0);
					ui->spinBoxRegion5->setValue(0);
					break;
				case 3:
					ui->spinBoxRegion1->setValue(regionSize);
					ui->spinBoxRegion2->setValue(regionSize);
					ui->spinBoxRegion3->setValue(remainder);
					ui->spinBoxRegion4->setValue(0);
					ui->spinBoxRegion5->setValue(0);
					break;
				case 4:
					ui->spinBoxRegion1->setValue(regionSize);
					ui->spinBoxRegion2->setValue(regionSize);
					ui->spinBoxRegion3->setValue(regionSize);
					ui->spinBoxRegion4->setValue(remainder);
					ui->spinBoxRegion5->setValue(0);
					break;
				case 5:
					ui->spinBoxRegion1->setValue(regionSize);
					ui->spinBoxRegion2->setValue(regionSize);
					ui->spinBoxRegion3->setValue(regionSize);
					ui->spinBoxRegion4->setValue(regionSize);
					ui->spinBoxRegion5->setValue(remainder);
					break;
				}
			}
		}
	}
	return;
}

void CameraSettingsWidget::loadDefaults()
{
	//measurement
	ui->comboBoxSti->setCurrentIndex(settingStiDefault);
	ui->comboBoxBti->setCurrentIndex(settingBtiDefault);
	ui->comboBoxSslope->setCurrentIndex(settingSslopeDefault);
	ui->comboBoxBslope->setCurrentIndex(settingBslopeDefault);
	ui->doubleSpinBoxSTime_in_us->setValue(settingStime_in_microseconds_Default);
	ui->doubleSpinBoxBTimer_in_us->setValue(settingBtime_in_microseconds_Default);
	ui->doubleSpinBoxSdatIn10ns->setValue(settingSdat_in_10nsDefault);
	ui->doubleSpinBoxBdatIn10ns->setValue(settingBdat_in_10nsDefault);
	ui->doubleSpinBoxSecIn10ns->setValue(settingShutterSecIn10nsDefault);
	ui->doubleSpinBoxBecIn10ns->setValue(settingShutterBecIn10nsDefault);
	ui->spinBoxBticnt->setValue(settingBticntDefault);
	ui->spinBoxSticnt->setValue(settingSticntDefault);
	ui->spinBoxTocnt->setValue(settingTocntDefault);
	ui->comboBoxTriggerModeIntegrator->setCurrentIndex(settingTriggerModeIntegratorDefault);
	ui->doubleSpinBoxXckdelayIn10ns->setValue(settingXckdelayIn10nsDefault);
	ui->doubleSpinBoxS1S2ReadDelayIn10ns->setValue(settingS1S2ReadDelayIn10nsDefault);
	//camera setup
	ui->comboBoxCameraSystem->setCurrentIndex(settingCameraSystemDefault);
	ui->comboBoxCameraSystem->currentIndexChanged(settingCameraSystemDefault);
	ui->comboBoxSensorType->setCurrentIndex(settingSensorTypeDefault);
	ui->comboBoxSensorType->currentIndexChanged(settingSensorTypeDefault);
	ui->spinBoxCamcnt->setValue(settingCamcntDefault);
	ui->spinBoxPixel->setValue(settingPixelDefault);
	ui->checkBoxLed->setChecked(settingLedDefault);
	ui->spinBoxSensorGain->setValue(settingSensorGainDefault);
	ui->spinBoxAdcGain->setValue(settingAdcGainDefault);
	ui->checkBoxIsCooledCameraLegacyMode->setChecked(settingIsCooledCameraLegacyModeDefault);
	ui->comboBoxCamCool->setCurrentIndex(settingCoolingDefault);
	ui->spinBoxGpxOffset->setValue(settingGpxOffsetDefault);
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settingIOCtrlImpactStartPixelDefault);
	ui->checkBoxUseSoftwarePolling->setChecked(settingsUseSoftwarePollingDefault);
	ui->spinBoxSensorResetOrHsirEcIn1ns->setValue(settingSensorResetOrHsIrDefault * 4);
	ui->comboBoxChannelSelect->setCurrentIndex(settingChannelSelectDefault);
	ui->checkBoxShiftS1S2ToNextScan->setChecked(settingShiftS1S2ToNextScanDefault);
	//FFT mode
	ui->spinBoxLines->setValue(settingLinesDefault);
	ui->spinBoxVfreq->setValue(settingVfreqDefault);
	ui->comboBoxFftMode->setCurrentIndex(settingFftModeDefault);
	ui->spinBoxLinesBinning->setValue(settingLinesBinningDefault);
	ui->spinBoxNumberOfRegions->setValue(settingNumberOfRegionsDefault);
	ui->checkBoxRegionsEqual->setChecked(settingRegionSizeEqualDefault);
	ui->spinBoxRegion1->setValue(settingRegionSize1Default);
	ui->spinBoxRegion2->setValue(settingRegionSize2Default);
	ui->spinBoxRegion3->setValue(settingRegionSize3Default);
	ui->spinBoxRegion4->setValue(settingRegionSize4Default);
	ui->spinBoxRegion5->setValue(settingRegionSize5Default);
	//other
	ui->comboBoxMonitor->setCurrentIndex(settingMonitorDefault);
	ui->comboBoxOutput->setCurrentIndex(settingTorDefault);
	ui->comboBoxAdcMode->setCurrentIndex(settingAdcModeDefault);
	ui->spinBoxAdcCustom->setValue(settingAdcCustomValueDefault);
	ui->checkBoxWriteDataToDisc->setChecked(settingWriteToDiscDefault);
	ui->plainTextEditFilePath->setPlainText(QDir::currentPath());
	return;
}

// only allow values n * 64
void CameraSettingsWidget::on_spinBoxPixel_valueChanged(int arg1)
{
	int newPixelValue = 0;
	if (arg1 % 64 > 32)
		newPixelValue = arg1 + 64 - arg1 % 64;
	else
		newPixelValue = arg1 - arg1 % 64;
	ui->spinBoxPixel->setValue(newPixelValue);
}

void CameraSettingsWidget::on_comboBoxFftMode_currentIndexChanged(int index)
{
	bool enabled = true;
	switch (_settings_level)
	{
	case settings_level_guided:
		enabled = false;
		break;
	default:
	case settings_level_free:
		enabled = true;
		break;
	}
	switch (index)
	{
	case full_binning:
		ui->spinBoxLinesBinning->setEnabled(enabled);
		ui->spinBoxNumberOfRegions->setEnabled(enabled);
		ui->checkBoxRegionsEqual->setEnabled(enabled);
		ui->comboBoxSti->setEnabled(true);
		if (ds)
		{
			ds->ui->doubleSpinBoxNos->setEnabled(true);
		}
		break;
	case partial_binning:
		ui->spinBoxLinesBinning->setEnabled(enabled);
		ui->spinBoxNumberOfRegions->setEnabled(true);
		ui->checkBoxRegionsEqual->setEnabled(true);
		ui->comboBoxSti->setEnabled(enabled);
		ui->comboBoxSti->setCurrentIndex(sti_ASL);
		if (ds)
		{
			if(!enabled)
				ds->ui->doubleSpinBoxNos->setValue(ui->spinBoxNumberOfRegions->value());
			ds->ui->doubleSpinBoxNos->setEnabled(enabled);
		}
		break;
	case area_mode:
		ui->spinBoxLinesBinning->setEnabled(true);
		ui->spinBoxNumberOfRegions->setEnabled(enabled);
		ui->checkBoxRegionsEqual->setEnabled(enabled);
		ui->comboBoxSti->setEnabled(enabled);
		ui->comboBoxSti->setCurrentIndex(sti_ASL);
		if (ds)
		{
			if (!enabled)
				ds->ui->doubleSpinBoxNos->setValue(ui->spinBoxLines->value());
			ds->ui->doubleSpinBoxNos->setEnabled(enabled);
		}
		break;
	}
	on_spinBoxNumberOfRegions_valueChanged(ui->spinBoxNumberOfRegions->value());
}

void CameraSettingsWidget::on_pushButtonFilePath_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), nullptr, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty()) ui->plainTextEditFilePath->setPlainText(dir);
	return;
}

void CameraSettingsWidget::on_checkBoxWriteDataToDisc_stateChanged(int arg1)
{
	bool enabled = true;
	switch (_settings_level)
	{
	case settings_level_guided:
		enabled = arg1;
		break;
	default:
	case settings_level_free:
		enabled = true;
		break;
	}
	ui->plainTextEditFilePath->setEnabled(enabled);
	ui->pushButtonFilePath->setEnabled(enabled);
	return;
}

void CameraSettingsWidget::changeSettingsLevel(int settings_level)
{
	_settings_level = settings_level;
	//run all slots to apply visible and enabled changes
	on_comboBoxSti_currentIndexChanged(ui->comboBoxSti->currentIndex());
	on_comboBoxBti_currentIndexChanged(ui->comboBoxBti->currentIndex());
	on_comboBoxSensorType_currentIndexChanged(ui->comboBoxSensorType->currentIndex());
	on_comboBoxCameraSystem_currentIndexChanged(ui->comboBoxCameraSystem->currentIndex());
	on_checkBoxRegionsEqual_stateChanged(ui->checkBoxRegionsEqual->checkState());
	on_comboBoxFftMode_currentIndexChanged(ui->comboBoxFftMode->currentIndex());
	on_checkBoxWriteDataToDisc_stateChanged(ui->checkBoxWriteDataToDisc->checkState());
	return;
}

void CameraSettingsWidget::initializeWidget()
{
	settings.beginGroup("board" + QString::number(drvno));

	// Here the saved settings on the system are applied to the UI.
	// For some settings there are two calls, to trigger the according slot for graying out options. I don't know why this is necessary, but without it the slots are not triggered.
	// The order is the same as in the widget camerasettingswidget.ui and dialogsettings.h
	//Measurement
	ui->comboBoxSti->setCurrentIndex(settings.value(settingStiPath, settingStiDefault).toDouble());
	ui->comboBoxBti->setCurrentIndex(settings.value(settingBtiPath, settingBtiDefault).toDouble());
	ui->comboBoxSslope->setCurrentIndex(settings.value(settingSslopePath, settingSslopeDefault).toDouble());
	ui->comboBoxBslope->setCurrentIndex(settings.value(settingBslopePath, settingBslopeDefault).toDouble());
	ui->doubleSpinBoxSTime_in_us->setValue(settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble());
	ui->doubleSpinBoxBTimer_in_us->setValue(settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble());
	ui->doubleSpinBoxSdatIn10ns->setValue(settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
	ui->doubleSpinBoxBdatIn10ns->setValue(settings.value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
	ui->doubleSpinBoxSecIn10ns->setValue(settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toDouble());
	ui->doubleSpinBoxBecIn10ns->setValue(settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toDouble());
	ui->spinBoxSticnt->setValue(settings.value(settingSticntPath, settingSticntDefault).toDouble());
	ui->spinBoxBticnt->setValue(settings.value(settingBticntPath, settingBticntDefault).toDouble());
	ui->spinBoxTocnt->setValue(settings.value(settingTocntPath, settingTocntDefault).toDouble());
	ui->comboBoxTriggerModeIntegrator->setCurrentIndex(settings.value(settingTriggerModeIntegratorPath, settingTriggerModeIntegratorDefault).toDouble());
	ui->doubleSpinBoxXckdelayIn10ns->setValue(settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toDouble());
	ui->doubleSpinBoxS1S2ReadDelayIn10ns->setValue(settings.value(settingS1S2ReadDelayIn10nsPath, settingS1S2ReadDelayIn10nsDefault).toDouble());
	//Camera setup
	ui->comboBoxCameraSystem->setCurrentIndex(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble());
	ui->comboBoxCameraSystem->currentIndexChanged(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble());
	ui->comboBoxSensorType->setCurrentIndex(settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble());
	ui->comboBoxSensorType->currentIndexChanged(settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble());
	ui->checkBoxIsFftLegacy->setChecked(settings.value(settingIsFftLegacyPath, settingIsFftlegacyDefault).toBool());
	ui->spinBoxCamcnt->setValue(settings.value(settingCamcntPath, settingCamcntDefault).toDouble());
	ui->spinBoxPixel->setValue(settings.value(settingPixelPath, settingPixelDefault).toDouble());
	ui->checkBoxLed->setChecked(settings.value(settingLedPath, settingLedDefault).toBool());
	ui->spinBoxSensorGain->setValue(settings.value(settingSensorGainPath, settingSensorGainDefault).toDouble());
	ui->spinBoxAdcGain->setValue(settings.value(settingAdcGainPath, settingAdcGainDefault).toDouble());
	ui->checkBoxIsCooledCameraLegacyMode->setChecked(settings.value(settingIsCooledCameraLegacyModePath, settingIsCooledCameraLegacyModeDefault).toBool());
	ui->comboBoxCamCool->setCurrentIndex(settings.value(settingCoolingPath, settingCoolingDefault).toDouble());
	ui->spinBoxGpxOffset->setValue(settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toDouble());
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toDouble());
	ui->checkBoxUseSoftwarePolling->setChecked(settings.value(settingsUseSoftwarePollingPath, settingsUseSoftwarePollingDefault).toBool());
	if(settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble() == sensor_type_hsvis)
		ui->spinBoxSensorResetOrHsirEcIn1ns->setValue(settings.value(settingSensorResetOrHsirEcPath, settingSensorResetOrHsIrDefault).toDouble() * 4);
	else
		ui->spinBoxSensorResetOrHsirEcIn1ns->setValue(settings.value(settingSensorResetOrHsirEcPath, settingSensorResetOrHsIrDefault).toDouble() * 160);
	ui->comboBoxChannelSelect->setCurrentIndex(settings.value(settingChannelSelectPath, settingChannelSelectDefault).toDouble());
	ui->checkBoxShiftS1S2ToNextScan->setChecked(settings.value(settingShiftS1S2ToNextScanPath, settingShiftS1S2ToNextScanDefault).toBool());
	//FFT mode
	ui->spinBoxLines->setValue(settings.value(settingLinesPath, settingLinesDefault).toDouble());
	ui->spinBoxVfreq->setValue(settings.value(settingVfreqPath, settingVfreqDefault).toDouble());
	ui->comboBoxFftMode->setCurrentIndex(settings.value(settingFftModePath, settingFftModeDefault).toDouble());
	ui->spinBoxLinesBinning->setValue(settings.value(settingLinesBinningPath, settingLinesBinningDefault).toDouble());
	ui->spinBoxNumberOfRegions->setValue(settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toDouble());
	ui->checkBoxRegionsEqual->setChecked(settings.value(settingRegionSizeEqualPath, settingRegionSizeEqualDefault).toBool());
	ui->spinBoxRegion1->setValue(settings.value(settingRegionSize1Path, settingRegionSize1Default).toDouble());
	ui->spinBoxRegion2->setValue(settings.value(settingRegionSize2Path, settingRegionSize2Default).toDouble());
	ui->spinBoxRegion3->setValue(settings.value(settingRegionSize3Path, settingRegionSize3Default).toDouble());
	ui->spinBoxRegion4->setValue(settings.value(settingRegionSize4Path, settingRegionSize4Default).toDouble());
	ui->spinBoxRegion5->setValue(settings.value(settingRegionSize5Path, settingRegionSize5Default).toDouble());
	//other
	ui->comboBoxMonitor->setCurrentIndex(settings.value(settingMonitorPath, settingMonitorDefault).toDouble());
	ui->comboBoxOutput->setCurrentIndex(settings.value(settingTorPath, settingTorDefault).toDouble());
	ui->comboBoxAdcMode->setCurrentIndex(settings.value(settingAdcModePath, settingAdcModeDefault).toDouble());
	ui->spinBoxAdcCustom->setValue(settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toDouble());
	ui->checkBoxWriteDataToDisc->setChecked(settings.value(settingWriteDataToDiscPath, settingWriteToDiscDefault).toBool());
	ui->plainTextEditFilePath->setPlainText(settings.value(settingFilePathPath, QDir::currentPath()).toString());
	return;
}

void CameraSettingsWidget::on_spinBoxLines_valueChanged(int value)
{
	if (_settings_level == settings_level_guided && ui->comboBoxFftMode->currentIndex() == area_mode)
	{
		if (ds)
		{
			ds->ui->doubleSpinBoxNos->setValue(value / ui->spinBoxLinesBinning->value());
		}
	}
	on_spinBoxNumberOfRegions_valueChanged(ui->spinBoxNumberOfRegions->value());
}

void CameraSettingsWidget::on_spinBoxLinesBinning_valueChanged(int value)
{
	if (_settings_level == settings_level_guided && ui->comboBoxFftMode->currentIndex() == area_mode)
	{
		if (ds)
		{
			ds->ui->doubleSpinBoxNos->setValue(ui->spinBoxLines->value() / value);
		}
	}
}
