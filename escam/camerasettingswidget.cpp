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

	sp_retain = ui->doubleSpinBoxSTime_in_ms->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->doubleSpinBoxSTime_in_ms->setSizePolicy(sp_retain);
	ui->doubleSpinBoxBTimer_in_ms->setSizePolicy(sp_retain);

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
	ui->comboBoxCamCool->setSizePolicy(sp_retain);

	sp_retain = ui->plainTextEditFilePath->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->plainTextEditFilePath->setSizePolicy(sp_retain);

	sp_retain = ui->pushButtonFilePath->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->pushButtonFilePath->setSizePolicy(sp_retain);

	// Here the saved settings on the system are applied to the UI.
	// For some settings there are two calls, to trigger the according slot for graying out options. I don't know why this is necessary, but without it the slots are not triggered.
	//Measurement
	ui->doubleSpinBoxNos->setValue(settings.value(settingNosPath, settingNosDefault).toDouble());
	ui->doubleSpinBoxNob->setValue(settings.value(settingNobPath, settingNobDefault).toDouble());
	ui->comboBoxSti->setCurrentIndex(settings.value(settingStiPath, settingStiDefault).toInt());
	ui->comboBoxBti->setCurrentIndex(settings.value(settingBtiPath, settingBtiDefault).toInt());
	ui->doubleSpinBoxSTime_in_ms->setValue(settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble() / 1000);
	ui->doubleSpinBoxBTimer_in_ms->setValue(settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble() / 1000);
	ui->doubleSpinBoxSdatIn10ns->setValue(settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
	ui->doubleSpinBoxBdatIn10ns->setValue(settings.value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
	ui->comboBoxSslope->setCurrentIndex(settings.value(settingSslopePath, settingSslopeDefault).toInt());
	ui->comboBoxBslope->setCurrentIndex(settings.value(settingBslopePath, settingBslopeDefault).toInt());
	ui->doubleSpinBoxXckdelayIn10ns->setValue(settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toDouble());
	ui->doubleSpinBoxSecIn10ns->setValue(settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toDouble());
	ui->doubleSpinBoxBecIn10ns->setValue(settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toDouble());
	ui->comboBoxTriggerModeCC->setCurrentIndex(settings.value(settingTriggerCcPath, settingTriggerCcDefault).toInt());
	//Camera setup
	ui->comboBoxSensorType->setCurrentIndex(settings.value(settingSensorTypePath, settingSensorTypeDefault).toInt());
	ui->comboBoxSensorType->currentIndexChanged(settings.value(settingSensorTypePath, settingSensorTypeDefault).toInt());
	ui->comboBoxCameraSystem->setCurrentIndex(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toInt());
	ui->comboBoxCameraSystem->currentIndexChanged(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toInt());
	ui->spinBoxCamcnt->setValue(settings.value(settingCamcntPath, settingCamcntDefault).toInt());
	ui->spinBoxPixel->setValue(settings.value(settingPixelPath, settingPixelDefault).toInt());
	ui->checkBoxMshut->setChecked(settings.value(settingMshutPath, settingMshutDefault).toBool());
	ui->checkBoxMshut->stateChanged(settings.value(settingMshutPath, settingMshutDefault).toBool());
	ui->checkBoxLed->setChecked(settings.value(settingLedPath, settingLedDefault).toBool());
	ui->spinBoxSensorGain->setValue(settings.value(settingSensorGainPath, settingSensorGainDefault).toInt());
	ui->spinBoxAdcGain->setValue(settings.value(settingAdcGainPath, settingAdcGainDefault).toInt());
	ui->comboBoxCamCool->setCurrentIndex(settings.value(settingCoolingPath, settingCoolingDefault).toInt());
	ui->spinBoxGpxOffset->setValue(settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toInt());
	ui->checkBoxIr->setChecked(settings.value(settingIsIrPath, settingIsIrDefault).toBool());
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toInt());
	ui->checkBoxUseSoftwarePolling->setChecked(settings.value(settingsUseSoftwarePollingPath, settingsUseSoftwarePollingDefault).toBool());
	ui->checkBoxShortrs->setChecked(settings.value(settingShortrsPath, settingShortrsDefault).toBool());
	ui->checkBoxIsCooledCam->setChecked(settings.value(settingIsCooledCamPath, settingIsCooledCamDefault).toBool());
	//FFT mode
	ui->spinBoxLines->setValue(settings.value(settingLinesPath, settingLinesDefault).toInt());
	ui->spinBoxVfreq->setValue(settings.value(settingVfreqPath, settingVfreqDefault).toInt());
	ui->comboBoxFftMode->setCurrentIndex(settings.value(settingFftModePath, settingFftModeDefault).toInt());
	ui->spinBoxLinesBinning->setValue(settings.value(settingLinesBinningPath, settingLinesBinningDefault).toInt());
	ui->spinBoxNumberOfRegions->setValue(settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toInt());
	ui->checkBoxRegionsEqual->setChecked(settings.value(settingRegionSizeEqualPath, settingRegionSizeEqualDefault).toBool());
	ui->spinBoxRegion1->setValue(settings.value(settingRegionSize1Path, settingRegionSize1Default).toInt());
	ui->spinBoxRegion2->setValue(settings.value(settingRegionSize2Path, settingRegionSize2Default).toInt());
	ui->spinBoxRegion3->setValue(settings.value(settingRegionSize3Path, settingRegionSize3Default).toInt());
	ui->spinBoxRegion4->setValue(settings.value(settingRegionSize4Path, settingRegionSize4Default).toInt());
	ui->spinBoxRegion5->setValue(settings.value(settingRegionSize5Path, settingRegionSize5Default).toInt());
	ui->spinBoxRegion6->setValue(settings.value(settingRegionSize6Path, settingRegionSize6Default).toInt());
	ui->spinBoxRegion7->setValue(settings.value(settingRegionSize7Path, settingRegionSize7Default).toInt());
	ui->spinBoxRegion8->setValue(settings.value(settingRegionSize8Path, settingRegionSize8Default).toInt());
	//Export data
	ui->checkBoxWriteDataToDisc->setChecked(settings.value(settingWriteDataToDiscPath, settingWriteToDiscDefault).toBool());
	ui->plainTextEditFilePath->setPlainText(settings.value(settingFilePathPath, QDir::currentPath()).toString());
	//Debug
	ui->comboBoxOutput->setCurrentIndex(settings.value(settingTorPath, settingTorDefault).toInt());
	ui->comboBoxAdcMode->setCurrentIndex(settings.value(settingAdcModePath, settingAdcModeDefault).toInt());
	ui->spinBoxAdcCustom->setValue(settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toInt());
}

CameraSettingsWidget::~CameraSettingsWidget()
{}

void CameraSettingsWidget::on_accepted()
{
	//Here the settings on the UI are saved to the system
	//Measurement
	settings.setValue(settingNosPath, ui->doubleSpinBoxNos->value());
	settings.setValue(settingNobPath, ui->doubleSpinBoxNob->value());
	settings.setValue(settingStiPath, ui->comboBoxSti->currentIndex());
	settings.setValue(settingBtiPath, ui->comboBoxBti->currentIndex());
	settings.setValue(settingStime_in_microseconds_Path, ui->doubleSpinBoxSTime_in_ms->value() * 1000);
	settings.setValue(settingBtime_in_microseconds_Path, ui->doubleSpinBoxBTimer_in_ms->value() * 1000);
	settings.setValue(settingSdat_in_10nsPath, ui->doubleSpinBoxSdatIn10ns->value());
	settings.setValue(settingBdat_in_10nsPath, ui->doubleSpinBoxBdatIn10ns->value());
	settings.setValue(settingSslopePath, ui->comboBoxSslope->currentIndex());
	settings.setValue(settingBslopePath, ui->comboBoxBslope->currentIndex());
	settings.setValue(settingXckdelayIn10nsPath, ui->doubleSpinBoxXckdelayIn10ns->value());
	settings.setValue(settingShutterSecIn10nsPath, ui->doubleSpinBoxSecIn10ns->value());
	settings.setValue(settingShutterBecIn10nsPath, ui->doubleSpinBoxBecIn10ns->value());
	settings.setValue(settingTriggerCcPath, ui->comboBoxTriggerModeCC->currentIndex());
	//Camera setup
	settings.setValue(settingSensorTypePath, ui->comboBoxSensorType->currentIndex());
	settings.setValue(settingCameraSystemPath, ui->comboBoxCameraSystem->currentIndex());
	settings.setValue(settingCamcntPath, ui->spinBoxCamcnt->value());
	settings.setValue(settingPixelPath, ui->spinBoxPixel->value());
	settings.setValue(settingMshutPath, ui->checkBoxMshut->isChecked());
	settings.setValue(settingLedPath, ui->checkBoxLed->isChecked());
	settings.setValue(settingSensorGainPath, ui->spinBoxSensorGain->value());
	settings.setValue(settingAdcGainPath, ui->spinBoxAdcGain->value());
	settings.setValue(settingCoolingPath, ui->comboBoxCamCool->currentIndex());
	settings.setValue(settingGpxOffsetPath, ui->spinBoxGpxOffset->value());
	settings.setValue(settingIsIrPath, ui->checkBoxIr->isChecked());
	settings.setValue(settingIOCtrlImpactStartPixelPath, ui->spinBoxIOCtrlImpactStartPixel->value());
	settings.setValue(settingsUseSoftwarePollingPath, ui->checkBoxUseSoftwarePolling->isChecked());
	settings.setValue(settingShortrsPath, ui->checkBoxShortrs->isChecked());
	settings.setValue(settingIsCooledCamPath, ui->checkBoxIsCooledCam->isChecked());
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
	return;
}
