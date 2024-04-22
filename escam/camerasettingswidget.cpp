#include "camerasettingswidget.h"

CameraSettingsWidget::CameraSettingsWidget(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::CameraSettingsWidgetClass())
{
	ui->setupUi(this);
	//don't rearrange widgets when hiding other widgets
	// QSizePolicy is retrieved for every different type of widgets separately, because I don't know whether the size policies differ between the widgets. This makes sure, that only the parameter setReainSizeWhenHidden is changed.
	QSizePolicy sp_retain = ui->labelSTimer->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->labelRegion1->setSizePolicy(sp_retain);
	ui->labelRegion2->setSizePolicy(sp_retain);
	ui->labelRegion3->setSizePolicy(sp_retain);
	ui->labelRegion4->setSizePolicy(sp_retain);
	ui->labelRegion5->setSizePolicy(sp_retain);
	ui->labelRegion6->setSizePolicy(sp_retain);
	ui->labelRegion7->setSizePolicy(sp_retain);
	ui->labelRegion8->setSizePolicy(sp_retain);
	ui->labelSTimer->setSizePolicy(sp_retain);
	ui->labelBTimer->setSizePolicy(sp_retain);
	ui->labelSec->setSizePolicy(sp_retain);
	ui->labelGain3010->setSizePolicy(sp_retain);
	ui->labelGain3030->setSizePolicy(sp_retain);
	ui->labelLines->setSizePolicy(sp_retain);
	ui->labelLinesBinning->setSizePolicy(sp_retain);
	ui->labelNumberOfRegions->setSizePolicy(sp_retain);
	ui->labelRegionsEqual->setSizePolicy(sp_retain);
	ui->labelSslope->setSizePolicy(sp_retain);
	ui->labelBslope->setSizePolicy(sp_retain);
	ui->labelFilePath->setSizePolicy(sp_retain);
	ui->labelCamCool->setSizePolicy(sp_retain);

	sp_retain = ui->doubleSpinBoxSTime_in_us->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->doubleSpinBoxSTime_in_us->setSizePolicy(sp_retain);
	ui->doubleSpinBoxBTimer_in_us->setSizePolicy(sp_retain);

	sp_retain = ui->checkBoxRegionsEqual->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->checkBoxRegionsEqual->setSizePolicy(sp_retain);

	sp_retain = ui->spinBoxAdcGain->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->spinBoxAdcGain->setSizePolicy(sp_retain);
	ui->spinBoxLines->setSizePolicy(sp_retain);
	ui->spinBoxRegion1->setSizePolicy(sp_retain);
	ui->spinBoxRegion2->setSizePolicy(sp_retain);
	ui->spinBoxRegion3->setSizePolicy(sp_retain);
	ui->spinBoxRegion4->setSizePolicy(sp_retain);
	ui->spinBoxRegion5->setSizePolicy(sp_retain);
	ui->spinBoxRegion6->setSizePolicy(sp_retain);
	ui->spinBoxRegion7->setSizePolicy(sp_retain);
	ui->spinBoxRegion8->setSizePolicy(sp_retain);
	ui->spinBoxLinesBinning->setSizePolicy(sp_retain);
	ui->spinBoxNumberOfRegions->setSizePolicy(sp_retain);
	ui->doubleSpinBoxSecIn10ns->setSizePolicy(sp_retain);

	sp_retain = ui->comboBoxSslope->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->comboBoxSslope->setSizePolicy(sp_retain);
	ui->comboBoxBslope->setSizePolicy(sp_retain);

	sp_retain = ui->plainTextEditFilePath->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->plainTextEditFilePath->setSizePolicy(sp_retain);

	sp_retain = ui->pushButtonFilePath->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->pushButtonFilePath->setSizePolicy(sp_retain);

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
	//Here the settings on the UI are saved to the system
	//Measurement
	settings.setValue(settingStiPath, ui->comboBoxSti->currentIndex());
	settings.setValue(settingBtiPath, ui->comboBoxBti->currentIndex());
	settings.setValue(settingStime_in_microseconds_Path, ui->doubleSpinBoxSTime_in_us->value());
	settings.setValue(settingBtime_in_microseconds_Path, ui->doubleSpinBoxBTimer_in_us->value());
	settings.setValue(settingSdat_in_10nsPath, ui->doubleSpinBoxSdatIn10ns->value());
	settings.setValue(settingBdat_in_10nsPath, ui->doubleSpinBoxBdatIn10ns->value());
	settings.setValue(settingSslopePath, ui->comboBoxSslope->currentIndex());
	settings.setValue(settingBslopePath, ui->comboBoxBslope->currentIndex());
	settings.setValue(settingXckdelayIn10nsPath, ui->doubleSpinBoxXckdelayIn10ns->value());
	settings.setValue(settingShutterSecIn10nsPath, ui->doubleSpinBoxSecIn10ns->value());
	settings.setValue(settingShutterBecIn10nsPath, ui->doubleSpinBoxBecIn10ns->value());
	settings.setValue(settingTriggerModeIntegratorPath, ui->comboBoxTriggerModeIntegrator->currentIndex());
	settings.setValue(settingS1S2ReadDelayIn10nsPath, ui->doubleSpinBoxS1S2ReadDelayIn10ns->value());
	//Camera setup
	settings.setValue(settingSensorTypePath, ui->comboBoxSensorType->currentIndex());
	settings.setValue(settingIsFftLegacyPath, ui->checkBoxIsFftLegacy->isChecked());
	settings.setValue(settingCameraSystemPath, ui->comboBoxCameraSystem->currentIndex());
	settings.setValue(settingCamcntPath, ui->spinBoxCamcnt->value());
	settings.setValue(settingPixelPath, ui->spinBoxPixel->value());
	settings.setValue(settingLedPath, ui->checkBoxLed->isChecked());
	settings.setValue(settingSensorGainPath, ui->spinBoxSensorGain->value());
	settings.setValue(settingAdcGainPath, ui->spinBoxAdcGain->value());
	settings.setValue(settingCoolingPath, ui->comboBoxCamCool->currentIndex());
	settings.setValue(settingGpxOffsetPath, ui->spinBoxGpxOffset->value());
	settings.setValue(settingIOCtrlImpactStartPixelPath, ui->spinBoxIOCtrlImpactStartPixel->value());
	settings.setValue(settingsUseSoftwarePollingPath, ui->checkBoxUseSoftwarePolling->isChecked());
	settings.setValue(settingIsCooledCamPath, ui->checkBoxIsCooledCam->isChecked());
	settings.setValue(settingSensorResetLengthIn4nsPath, ui->spinBoxSensorResetLengthIn1ns->value() / 4);
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
	settings.setValue(settingRegionSize6Path, ui->spinBoxRegion6->value());
	settings.setValue(settingRegionSize7Path, ui->spinBoxRegion7->value());
	settings.setValue(settingRegionSize8Path, ui->spinBoxRegion8->value());
	//Export data
	settings.setValue(settingWriteDataToDiscPath, ui->checkBoxWriteDataToDisc->isChecked());
	settings.setValue(settingFilePathPath, ui->plainTextEditFilePath->toPlainText());
	//Debug
	settings.setValue(settingTorPath, ui->comboBoxOutput->currentIndex());
	settings.setValue(settingAdcModePath, ui->comboBoxAdcMode->currentIndex());
	settings.setValue(settingAdcCustomValuePath, ui->spinBoxAdcCustom->value());
	settings.setValue(settingBncOutPath, ui->comboBoxBncOut->currentIndex());
	return;
}

void CameraSettingsWidget::on_comboBoxSti_currentIndexChanged(int index)
{
	bool enabled = true,
		visible = true;
	switch (_settings_level)
	{
		//basic
	case 0:
		enabled = false;
		visible = false;
		break;
		//advanced
	case 1:
		enabled = false;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	switch (index)
	{
	case 4:
		ui->doubleSpinBoxSTime_in_us->setEnabled(true);
		ui->doubleSpinBoxSTime_in_us->setVisible(true);
		ui->labelSTimer->setVisible(true);
		break;
	default:
		ui->doubleSpinBoxSTime_in_us->setEnabled(enabled);
		ui->doubleSpinBoxSTime_in_us->setVisible(visible);
		ui->labelSTimer->setVisible(visible);
	}
	switch (index)
	{
	case 0:
	case 1:
	case 2:
		ui->comboBoxSslope->setEnabled(true);
		ui->comboBoxSslope->setVisible(true);
		ui->labelSslope->setVisible(true);
		break;
	default:
		ui->comboBoxSslope->setEnabled(enabled);
		ui->comboBoxSslope->setVisible(visible);
		ui->labelSslope->setVisible(visible);
	}
}

void CameraSettingsWidget::on_comboBoxBti_currentIndexChanged(int index)
{
	bool enabled = true,
		visible = true;
	switch (_settings_level)
	{
		//basic
	case 0:
		enabled = false;
		visible = false;
		break;
		//advanced
	case 1:
		enabled = false;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	switch (index)
	{
	case 4:
		ui->doubleSpinBoxBTimer_in_us->setEnabled(true);
		ui->doubleSpinBoxBTimer_in_us->setVisible(true);
		ui->labelBTimer->setVisible(true);
		ui->comboBoxBslope->setEnabled(enabled);
		ui->comboBoxBslope->setVisible(visible);
		ui->labelBslope->setVisible(visible);
		break;
	default:
		ui->doubleSpinBoxBTimer_in_us->setEnabled(enabled);
		ui->doubleSpinBoxBTimer_in_us->setVisible(visible);
		ui->labelBTimer->setVisible(visible);
		ui->comboBoxBslope->setEnabled(true);
		ui->comboBoxBslope->setVisible(true);
		ui->labelBslope->setVisible(true);
	}
}


void CameraSettingsWidget::on_comboBoxSensorType_currentIndexChanged(int index)
{
	bool enabled = false,
		visible = false;
	if (index == sensor_type_fft)
	{
		enabled = true;
		visible = true;
	}
	switch (_settings_level)
	{
		//basic
	case 0:
		break;
		//advanced
	case 1:
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->tabWidget->setTabEnabled(2, enabled);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
	ui->tabWidget->setTabVisible(2, visible);
#else
	(void)visible;
#endif
}

void CameraSettingsWidget::on_comboBoxCameraSystem_currentIndexChanged(int index)
{
	bool enabled = true,
		visible = true;
	switch (_settings_level)
	{
		//basic
	case 0:
		enabled = false;
		visible = false;
		break;
		//advanced
	case 1:
		enabled = false;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	switch (index)
	{
	case 0:
		ui->spinBoxAdcGain->setEnabled(enabled);
		ui->spinBoxAdcGain->setVisible(visible);
		ui->comboBoxAdcMode->setEnabled(enabled);
		ui->comboBoxAdcMode->setVisible(visible);
		ui->spinBoxAdcCustom->setEnabled(enabled);
		ui->spinBoxAdcCustom->setVisible(visible);
		break;
	case 1:
		ui->spinBoxAdcGain->setEnabled(enabled);
		ui->spinBoxAdcGain->setVisible(visible);
		ui->comboBoxAdcMode->setEnabled(true);
		ui->comboBoxAdcMode->setVisible(true);
		ui->spinBoxAdcCustom->setEnabled(true);
		ui->spinBoxAdcCustom->setVisible(true);
		break;
	case 2:
		ui->spinBoxAdcGain->setEnabled(true);
		ui->spinBoxAdcGain->setVisible(true);
		ui->comboBoxAdcMode->setEnabled(true);
		ui->comboBoxAdcMode->setVisible(true);
		ui->spinBoxAdcCustom->setEnabled(true);
		ui->spinBoxAdcCustom->setVisible(true);
		break;
	}
}

void CameraSettingsWidget::on_checkBoxRegionsEqual_stateChanged(int arg1)
{
	bool enabled = true,
		visible = true;
	switch (_settings_level)
	{
		//basic
	case 0:
		enabled = !arg1;
		visible = !arg1;
		break;
		//advanced
	case 1:
		enabled = !arg1;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->spinBoxRegion1->setEnabled(enabled);
	ui->spinBoxRegion2->setEnabled(enabled);
	ui->spinBoxRegion3->setEnabled(enabled);
	ui->spinBoxRegion4->setEnabled(enabled);
	ui->spinBoxRegion5->setEnabled(enabled);
	ui->spinBoxRegion6->setEnabled(enabled);
	ui->spinBoxRegion7->setEnabled(enabled);
	ui->spinBoxRegion8->setEnabled(enabled);
	ui->spinBoxRegion1->setVisible(visible);
	ui->spinBoxRegion2->setVisible(visible);
	ui->spinBoxRegion3->setVisible(visible);
	ui->spinBoxRegion4->setVisible(visible);
	ui->spinBoxRegion5->setVisible(visible);
	ui->spinBoxRegion6->setVisible(visible);
	ui->spinBoxRegion7->setVisible(visible);
	ui->spinBoxRegion8->setVisible(visible);
	ui->labelRegion1->setVisible(visible);
	ui->labelRegion2->setVisible(visible);
	ui->labelRegion3->setVisible(visible);
	ui->labelRegion4->setVisible(visible);
	ui->labelRegion5->setVisible(visible);
	ui->labelRegion6->setVisible(visible);
	ui->labelRegion7->setVisible(visible);
	ui->labelRegion8->setVisible(visible);
}

void CameraSettingsWidget::loadDefaults()
{
	//measurement
	ui->comboBoxSti->setCurrentIndex(settingStiDefault);
	ui->comboBoxBti->setCurrentIndex(settingBtiDefault);
	ui->doubleSpinBoxSTime_in_us->setValue(settingStime_in_microseconds_Default);
	ui->doubleSpinBoxBTimer_in_us->setValue(settingBtime_in_microseconds_Default);
	ui->doubleSpinBoxSdatIn10ns->setValue(settingSdat_in_10nsDefault);
	ui->doubleSpinBoxBdatIn10ns->setValue(settingBdat_in_10nsDefault);
	ui->comboBoxSslope->setCurrentIndex(settingSslopeDefault);
	ui->comboBoxBslope->setCurrentIndex(settingBslopeDefault);
	ui->doubleSpinBoxXckdelayIn10ns->setValue(settingXckdelayIn10nsDefault);
	ui->doubleSpinBoxSecIn10ns->setValue(settingShutterSecIn10nsDefault);
	ui->doubleSpinBoxBecIn10ns->setValue(settingShutterBecIn10nsDefault);
	ui->comboBoxTriggerModeIntegrator->setCurrentIndex(settingTriggerModeIntegratorDefault);
	//camera setup
	ui->comboBoxSensorType->setCurrentIndex(settingSensorTypeDefault);
	ui->comboBoxSensorType->currentIndexChanged(settingSensorTypeDefault);
	ui->comboBoxCameraSystem->setCurrentIndex(settingCameraSystemDefault);
	ui->comboBoxCameraSystem->currentIndexChanged(settingCameraSystemDefault);
	ui->spinBoxCamcnt->setValue(settingCamcntDefault);
	ui->spinBoxPixel->setValue(settingPixelDefault);
	ui->checkBoxLed->setChecked(settingLedDefault);
	ui->spinBoxSensorGain->setValue(settingSensorGainDefault);
	ui->spinBoxAdcGain->setValue(settingAdcGainDefault);
	ui->comboBoxCamCool->setCurrentIndex(settingCoolingDefault);
	ui->spinBoxGpxOffset->setValue(settingGpxOffsetDefault);
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settingIOCtrlImpactStartPixelDefault);
	ui->checkBoxUseSoftwarePolling->setChecked(settingsUseSoftwarePollingDefault);
	ui->checkBoxIsCooledCam->setChecked(settingIsCooledCamDefault);
	ui->spinBoxSensorResetLengthIn1ns->setValue(settingSensorResetLengthIn4nsDefault * 4);
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
	ui->spinBoxRegion6->setValue(settingRegionSize6Default);
	ui->spinBoxRegion7->setValue(settingRegionSize7Default);
	ui->spinBoxRegion8->setValue(settingRegionSize8Default);
	//Export data
	ui->checkBoxWriteDataToDisc->setChecked(settingWriteToDiscDefault);
	ui->plainTextEditFilePath->setPlainText(QDir::currentPath());
	//debug
	ui->comboBoxOutput->setCurrentIndex(settingTorDefault);
	ui->comboBoxAdcMode->setCurrentIndex(settingAdcModeDefault);
	ui->spinBoxAdcCustom->setValue(settingAdcCustomValueDefault);
	ui->comboBoxBncOut->setCurrentIndex(settingBncOutDefault);
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

// only allow values n * 4
void CameraSettingsWidget::on_spinBoxSensorResetLengthIn1ns_valueChanged(int arg1)
{
	int newValue = 0;
	if (arg1 % 4 > 2)
		newValue = arg1 + 4 - arg1 % 4;
	else
		newValue = arg1 - arg1 % 4;
	ui->spinBoxSensorResetLengthIn1ns->setValue(newValue);
}

void CameraSettingsWidget::on_comboBoxFftMode_currentIndexChanged(int index)
{
	bool enabled = true,
		visible = true;
	switch (_settings_level)
	{
		//basic
	case 0:
		enabled = false;
		visible = false;
		break;
		//advanced
	case 1:
		enabled = false;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	switch (index)
	{
		//full binning
	case 0:
		ui->labelLinesBinning->setVisible(visible);
		ui->spinBoxLinesBinning->setEnabled(enabled);
		ui->spinBoxLinesBinning->setVisible(visible);
		ui->labelNumberOfRegions->setVisible(visible);
		ui->spinBoxNumberOfRegions->setEnabled(enabled);
		ui->spinBoxNumberOfRegions->setVisible(visible);
		ui->labelRegionsEqual->setVisible(visible);
		ui->checkBoxRegionsEqual->setVisible(visible);
		ui->checkBoxRegionsEqual->setEnabled(enabled);
		ui->labelRegion1->setVisible(visible);
		ui->labelRegion2->setVisible(visible);
		ui->labelRegion3->setVisible(visible);
		ui->labelRegion4->setVisible(visible);
		ui->labelRegion5->setVisible(visible);
		ui->labelRegion6->setVisible(visible);
		ui->labelRegion7->setVisible(visible);
		ui->labelRegion8->setVisible(visible);
		ui->spinBoxRegion1->setVisible(visible);
		ui->spinBoxRegion2->setVisible(visible);
		ui->spinBoxRegion3->setVisible(visible);
		ui->spinBoxRegion4->setVisible(visible);
		ui->spinBoxRegion5->setVisible(visible);
		ui->spinBoxRegion6->setVisible(visible);
		ui->spinBoxRegion7->setVisible(visible);
		ui->spinBoxRegion8->setVisible(visible);
		ui->spinBoxRegion1->setEnabled(enabled);
		ui->spinBoxRegion2->setEnabled(enabled);
		ui->spinBoxRegion3->setEnabled(enabled);
		ui->spinBoxRegion4->setEnabled(enabled);
		ui->spinBoxRegion5->setEnabled(enabled);
		ui->spinBoxRegion6->setEnabled(enabled);
		ui->spinBoxRegion7->setEnabled(enabled);
		ui->spinBoxRegion8->setEnabled(enabled);
		break;
		//range of interest
	case 1:
		ui->labelLinesBinning->setVisible(visible);
		ui->spinBoxLinesBinning->setEnabled(enabled);
		ui->spinBoxLinesBinning->setVisible(visible);
		ui->labelNumberOfRegions->setVisible(true);
		ui->spinBoxNumberOfRegions->setEnabled(true);
		ui->spinBoxNumberOfRegions->setVisible(true);
		ui->labelRegionsEqual->setVisible(true);
		ui->checkBoxRegionsEqual->setVisible(true);
		ui->checkBoxRegionsEqual->setEnabled(true);
		ui->labelRegion1->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion2->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion3->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion4->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion5->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion6->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion7->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->labelRegion8->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion1->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion2->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion3->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion4->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion5->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion6->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion7->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion8->setVisible(!ui->checkBoxRegionsEqual->checkState() || visible);
		ui->spinBoxRegion1->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion2->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion3->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion4->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion5->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion6->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion7->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		ui->spinBoxRegion8->setEnabled(!ui->checkBoxRegionsEqual->checkState() || enabled);
		break;
		//area
	case 2:
		ui->labelLinesBinning->setVisible(true);
		ui->spinBoxLinesBinning->setEnabled(true);
		ui->spinBoxLinesBinning->setVisible(true);
		ui->labelNumberOfRegions->setVisible(visible);
		ui->spinBoxNumberOfRegions->setEnabled(enabled);
		ui->spinBoxNumberOfRegions->setVisible(visible);
		ui->labelRegionsEqual->setVisible(visible);
		ui->checkBoxRegionsEqual->setVisible(visible);
		ui->checkBoxRegionsEqual->setEnabled(enabled);
		ui->labelRegion1->setVisible(visible);
		ui->labelRegion2->setVisible(visible);
		ui->labelRegion3->setVisible(visible);
		ui->labelRegion4->setVisible(visible);
		ui->labelRegion5->setVisible(visible);
		ui->labelRegion6->setVisible(visible);
		ui->labelRegion7->setVisible(visible);
		ui->labelRegion8->setVisible(visible);
		ui->spinBoxRegion1->setVisible(visible);
		ui->spinBoxRegion2->setVisible(visible);
		ui->spinBoxRegion3->setVisible(visible);
		ui->spinBoxRegion4->setVisible(visible);
		ui->spinBoxRegion5->setVisible(visible);
		ui->spinBoxRegion6->setVisible(visible);
		ui->spinBoxRegion7->setVisible(visible);
		ui->spinBoxRegion8->setVisible(visible);
		ui->spinBoxRegion1->setEnabled(enabled);
		ui->spinBoxRegion2->setEnabled(enabled);
		ui->spinBoxRegion3->setEnabled(enabled);
		ui->spinBoxRegion4->setEnabled(enabled);
		ui->spinBoxRegion5->setEnabled(enabled);
		ui->spinBoxRegion6->setEnabled(enabled);
		ui->spinBoxRegion7->setEnabled(enabled);
		ui->spinBoxRegion8->setEnabled(enabled);
		break;
	}
}

void CameraSettingsWidget::on_pushButtonFilePath_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), nullptr, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!dir.isEmpty()) ui->plainTextEditFilePath->setPlainText(dir);
	return;
}

void CameraSettingsWidget::on_checkBoxWriteDataToDisc_stateChanged(int arg1)
{
	bool enabled = true,
		visible = true;
	switch (_settings_level)
	{
		//basic
	case 0:
		enabled = arg1;
		visible = arg1;
		break;
		//advanced
	case 1:
		enabled = arg1;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->plainTextEditFilePath->setEnabled(enabled);
	ui->pushButtonFilePath->setEnabled(enabled);
	ui->plainTextEditFilePath->setVisible(visible);
	ui->pushButtonFilePath->setVisible(visible);
	ui->labelFilePath->setVisible(visible);
	return;
}

void CameraSettingsWidget::changeSettingsLevel(int settings_level)
{
	_settings_level = settings_level;
	switch (_settings_level)
	{
	case 0:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->tabWidget->setTabVisible(1, false);
		ui->tabWidget->setTabVisible(4, false);
#endif
		ui->tabWidget->setTabEnabled(1, false);
		ui->tabWidget->setTabEnabled(4, false);
		ui->labelLines->setVisible(false);
		ui->spinBoxLines->setVisible(false);
		ui->spinBoxLines->setEnabled(false);
		break;
	case 1:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->tabWidget->setTabVisible(1, true);
		ui->tabWidget->setTabVisible(4, true);
#endif
		ui->tabWidget->setTabEnabled(1, true);
		ui->tabWidget->setTabEnabled(4, true);
		ui->labelLines->setVisible(true);
		ui->spinBoxLines->setVisible(true);
		ui->spinBoxLines->setEnabled(true);
		break;
	case 2:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->tabWidget->setTabVisible(4, true);
		ui->tabWidget->setTabVisible(1, true);
#endif
		ui->tabWidget->setTabEnabled(1, true);
		ui->tabWidget->setTabEnabled(4, true);
		ui->labelLines->setVisible(true);
		ui->spinBoxLines->setVisible(true);
		ui->spinBoxLines->setEnabled(true);
		break;
	}
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
	//Measurement
	ui->comboBoxSti->setCurrentIndex(settings.value(settingStiPath, settingStiDefault).toDouble());
	ui->comboBoxBti->setCurrentIndex(settings.value(settingBtiPath, settingBtiDefault).toDouble());
	ui->doubleSpinBoxSTime_in_us->setValue(settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble());
	ui->doubleSpinBoxBTimer_in_us->setValue(settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble());
	ui->doubleSpinBoxSdatIn10ns->setValue(settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
	ui->doubleSpinBoxBdatIn10ns->setValue(settings.value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
	ui->comboBoxSslope->setCurrentIndex(settings.value(settingSslopePath, settingSslopeDefault).toDouble());
	ui->comboBoxBslope->setCurrentIndex(settings.value(settingBslopePath, settingBslopeDefault).toDouble());
	ui->doubleSpinBoxXckdelayIn10ns->setValue(settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toDouble());
	ui->doubleSpinBoxSecIn10ns->setValue(settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toDouble());
	ui->doubleSpinBoxBecIn10ns->setValue(settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toDouble());
	ui->comboBoxTriggerModeIntegrator->setCurrentIndex(settings.value(settingTriggerModeIntegratorPath, settingTriggerModeIntegratorDefault).toDouble());
	ui->doubleSpinBoxS1S2ReadDelayIn10ns->setValue(settings.value(settingS1S2ReadDelayIn10nsPath, settingS1S2ReadDelayIn10nsDefault).toDouble());
	//Camera setup
	ui->comboBoxSensorType->setCurrentIndex(settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble());
	ui->comboBoxSensorType->currentIndexChanged(settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble());
	ui->checkBoxIsFftLegacy->setChecked(settings.value(settingIsFftLegacyPath, settingIsFftlegacyDefault).toBool());
	ui->comboBoxCameraSystem->setCurrentIndex(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble());
	ui->comboBoxCameraSystem->currentIndexChanged(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble());
	ui->spinBoxCamcnt->setValue(settings.value(settingCamcntPath, settingCamcntDefault).toDouble());
	ui->spinBoxPixel->setValue(settings.value(settingPixelPath, settingPixelDefault).toDouble());
	ui->checkBoxLed->setChecked(settings.value(settingLedPath, settingLedDefault).toBool());
	ui->spinBoxSensorGain->setValue(settings.value(settingSensorGainPath, settingSensorGainDefault).toDouble());
	ui->spinBoxAdcGain->setValue(settings.value(settingAdcGainPath, settingAdcGainDefault).toDouble());
	ui->comboBoxCamCool->setCurrentIndex(settings.value(settingCoolingPath, settingCoolingDefault).toDouble());
	ui->spinBoxGpxOffset->setValue(settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toDouble());
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toDouble());
	ui->checkBoxUseSoftwarePolling->setChecked(settings.value(settingsUseSoftwarePollingPath, settingsUseSoftwarePollingDefault).toBool());
	ui->checkBoxIsCooledCam->setChecked(settings.value(settingIsCooledCamPath, settingIsCooledCamDefault).toBool());
	ui->spinBoxSensorResetLengthIn1ns->setValue(settings.value(settingSensorResetLengthIn4nsPath, settingSensorResetLengthIn4nsDefault).toDouble() * 4);
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
	ui->spinBoxRegion6->setValue(settings.value(settingRegionSize6Path, settingRegionSize6Default).toDouble());
	ui->spinBoxRegion7->setValue(settings.value(settingRegionSize7Path, settingRegionSize7Default).toDouble());
	ui->spinBoxRegion8->setValue(settings.value(settingRegionSize8Path, settingRegionSize8Default).toDouble());
	//Export data
	ui->checkBoxWriteDataToDisc->setChecked(settings.value(settingWriteDataToDiscPath, settingWriteToDiscDefault).toBool());
	ui->plainTextEditFilePath->setPlainText(settings.value(settingFilePathPath, QDir::currentPath()).toString());
	//Debug
	ui->comboBoxOutput->setCurrentIndex(settings.value(settingTorPath, settingTorDefault).toDouble());
	ui->comboBoxAdcMode->setCurrentIndex(settings.value(settingAdcModePath, settingAdcModeDefault).toDouble());
	ui->spinBoxAdcCustom->setValue(settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toDouble());
	ui->comboBoxBncOut->setCurrentIndex(settings.value(settingBncOutPath, settingBncOutDefault).toDouble());
	return;
}
