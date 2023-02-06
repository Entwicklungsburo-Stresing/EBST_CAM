#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QMessageBox>
#include <QtGlobal>
#include "lsc-gui.h"

DialogSettings::DialogSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::on_accepted);

	ui->doubleSpinBoxContiniousPause_in_ms->setValue(settings.value(settingContiniousPauseInMicrosecondsPath, settingContiniousPausInMicrosecondsDefault).toDouble() / 1000);

	//Appearance
	ui->comboBoxTheme->setCurrentIndex(settings.value(settingThemePath, settingThemeDefault).toInt());
	//Misc
	ui->comboBoxSettingsLevel->setCurrentIndex(settings.value(settingSettingsLevelPath, settingSettingsLevelDefault).toInt());
	ui->comboBoxSettingsLevel->currentIndexChanged(ui->comboBoxSettingsLevel->currentIndex());

	// hide all board select elements
	ui->labelBoardSel->setVisible(false);
	ui->checkBoxBoard0->setVisible(false);
	ui->checkBoxBoard1->setVisible(false);
	ui->checkBoxBoard2->setVisible(false);
	ui->checkBoxBoard3->setVisible(false);
	ui->checkBoxBoard4->setVisible(false);
	ui->checkBoxBoard0->setChecked(false);
	ui->checkBoxBoard1->setChecked(false);
	ui->checkBoxBoard2->setChecked(false);
	ui->checkBoxBoard3->setChecked(false);
	ui->checkBoxBoard4->setChecked(false);
	// show board select elements depending on number_of_boards with intended fall through
	switch (number_of_boards)
	{
	case 5:
		ui->checkBoxBoard4->setVisible(true);
		ui->checkBoxBoard4->setChecked(settings.value(settingBoard4Path, settingBoard4Default).toBool());
	case 4:
		ui->checkBoxBoard3->setVisible(true);
		ui->checkBoxBoard3->setChecked(settings.value(settingBoard3Path, settingBoard3Default).toBool());
	case 3:
		ui->checkBoxBoard2->setVisible(true);
		ui->checkBoxBoard2->setChecked(settings.value(settingBoard2Path, settingBoard2Default).toBool());
	case 2:
		ui->checkBoxBoard1->setVisible(true);
		ui->labelBoardSel->setVisible(true);
		ui->checkBoxBoard1->setChecked(settings.value(settingBoard1Path, settingBoard1Default).toBool());
		ui->checkBoxBoard0->setVisible(true);
		ui->checkBoxBoard0->setChecked(settings.value(settingBoard0Path, settingBoard0Default).toBool());
		break;
	default:
	case 1:
		ui->checkBoxBoard0->setChecked(true);
	}

#if __linux__
	// disable option software polling on Linux
	ui->checkBoxUseSoftwarePolling->setChecked(false);
	ui->checkBoxUseSoftwarePolling->setEnabled(false);
#endif

	setWindowModality(Qt::ApplicationModal);
}

DialogSettings::~DialogSettings()
{
	delete ui;
}

void DialogSettings::on_accepted()
{
	//Here the settings on the UI are saved to the system
	//Measurement
	settings.setValue(settingContiniousPauseInMicrosecondsPath, ui->doubleSpinBoxContiniousPause_in_ms->value() * 1000);
	//Camera setup
	settings.setValue(settingBoard0Path, ui->checkBoxBoard0->isChecked());
	settings.setValue(settingBoard1Path, ui->checkBoxBoard1->isChecked());
	settings.setValue(settingBoard2Path, ui->checkBoxBoard2->isChecked());
	settings.setValue(settingBoard3Path, ui->checkBoxBoard3->isChecked());
	settings.setValue(settingBoard4Path, ui->checkBoxBoard4->isChecked());
	uint8_t board_sel = 0;
	board_sel |= ui->checkBoxBoard4->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard3->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard2->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard1->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard0->isChecked();
	settings.setValue(settingBoardSelPath, board_sel);
	//Appearance
	settings.setValue(settingThemePath, ui->comboBoxTheme->currentIndex());
	settings.setValue(settingSettingsLevelPath, ui->comboBoxSettingsLevel->currentIndex());
	emit settings_saved();
	return;
}

void DialogSettings::on_comboBoxSti_currentIndexChanged(int index)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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
	switch(index)
	{
	case 4:
		ui->doubleSpinBoxSTime_in_ms->setEnabled(true);
		ui->doubleSpinBoxSTime_in_ms->setVisible(true);
		ui->labelSTimer->setVisible(true);
		break;
	default:
		ui->doubleSpinBoxSTime_in_ms->setEnabled(enabled);
		ui->doubleSpinBoxSTime_in_ms->setVisible(visible);
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

void DialogSettings::on_comboBoxBti_currentIndexChanged(int index)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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
	switch(index)
	{
	case 4:
		ui->doubleSpinBoxBTimer_in_ms->setEnabled(true);
		ui->doubleSpinBoxBTimer_in_ms->setVisible(true);
		ui->labelBTimer->setVisible(true);
		ui->comboBoxBslope->setEnabled(enabled);
		ui->comboBoxBslope->setVisible(visible);
		ui->labelBslope->setVisible(visible);
		break;
	default:
		ui->doubleSpinBoxBTimer_in_ms->setEnabled(enabled);
		ui->doubleSpinBoxBTimer_in_ms->setVisible(visible);
		ui->labelBTimer->setVisible(visible);
		ui->comboBoxBslope->setEnabled(true);
		ui->comboBoxBslope->setVisible(true);
		ui->labelBslope->setVisible(true);
	}

}

void DialogSettings::on_comboBoxSensorType_currentIndexChanged(int index)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
	{
	//basic
	case 0:
		enabled = index;
		visible = index;
		break;
	//advanced
	case 1:
		enabled = index;
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

void DialogSettings::on_comboBoxCameraSystem_currentIndexChanged(int index)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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
	switch(index)
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

void DialogSettings::on_checkBoxRegionsEqual_stateChanged(int arg1)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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

void DialogSettings::on_pushButtonDefault_clicked()
{
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning", "All settings are going to be replaced by its default values. Are you sure?", QMessageBox::Yes|QMessageBox::No);
	if (reply == QMessageBox::Yes)
		loadDefaults();
	return;
}

void DialogSettings::loadDefaults()
{
	//measurement
	ui->doubleSpinBoxNos->setValue(settingNosDefault);
	ui->doubleSpinBoxNob->setValue(settingNobDefault);
	ui->comboBoxSti->setCurrentIndex(settingStiDefault);
	ui->comboBoxBti->setCurrentIndex(settingBtiDefault);
	ui->doubleSpinBoxSTime_in_ms->setValue(settingStime_in_microseconds_Default / 1000);
	ui->doubleSpinBoxBTimer_in_ms->setValue(settingBtime_in_microseconds_Default / 1000);
	ui->doubleSpinBoxSdatIn10ns->setValue(settingSdat_in_10nsDefault);
	ui->doubleSpinBoxBdatIn10ns->setValue(settingBdat_in_10nsDefault);
	ui->comboBoxSslope->setCurrentIndex(settingSslopeDefault);
	ui->comboBoxBslope->setCurrentIndex(settingBslopeDefault);
	ui->doubleSpinBoxXckdelayIn10ns->setValue(settingXckdelayIn10nsDefault);
	ui->doubleSpinBoxSecIn10ns->setValue(settingShutterSecIn10nsDefault);
	ui->doubleSpinBoxBecIn10ns->setValue(settingShutterBecIn10nsDefault);
	ui->comboBoxTriggerModeCC->setCurrentIndex(settingTriggerCcDefault);
	ui->doubleSpinBoxContiniousPause_in_ms->setValue(settingContiniousPausInMicrosecondsDefault / 1000);
	//camera setup
	ui->checkBoxBoard0->setChecked(settingBoard0Default);
	ui->checkBoxBoard1->setChecked(settingBoard1Default);
	ui->checkBoxBoard2->setChecked(settingBoard2Default);
	ui->checkBoxBoard3->setChecked(settingBoard3Default);
	ui->checkBoxBoard4->setChecked(settingBoard4Default);
	ui->comboBoxSensorType->setCurrentIndex(settingSensorTypeDefault);
	ui->comboBoxSensorType->currentIndexChanged(settingSensorTypeDefault);
	ui->comboBoxCameraSystem->setCurrentIndex(settingCameraSystemDefault);
	ui->comboBoxCameraSystem->currentIndexChanged(settingCameraSystemDefault);
	ui->spinBoxCamcnt->setValue(settingCamcntDefault);
	ui->spinBoxPixel->setValue(settingPixelDefault);
	ui->checkBoxMshut->setChecked(settingMshutDefault);
	ui->checkBoxMshut->stateChanged(settingMshutDefault);
	ui->checkBoxLed->setChecked(settingLedDefault);
	ui->spinBoxSensorGain->setValue(settingSensorGainDefault);
	ui->spinBoxAdcGain->setValue(settingAdcGainDefault);
	ui->comboBoxCamCool->setCurrentIndex(settingCoolingDefault);
	ui->spinBoxGpxOffset->setValue(settingGpxOffsetDefault);
	ui->checkBoxRegionsEqual->setChecked(settingIsIrDefault);
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settingIOCtrlImpactStartPixelDefault);
	ui->checkBoxUseSoftwarePolling->setChecked(settingsUseSoftwarePollingDefault);
	ui->checkBoxShortrs->setChecked(settingShortrsDefault);
	ui->checkBoxIsCooledCam->setChecked(settingIsCooledCamDefault);
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
	//appearance
	ui->comboBoxTheme->setCurrentIndex(settingThemeDefault);
	ui->comboBoxSettingsLevel->setCurrentIndex(settingSettingsLevelDefault);
	return;
}

// only allow values n * 64
void DialogSettings::on_spinBoxPixel_valueChanged(int arg1)
{
	int newPixelValue = 0;
	if(arg1 % 64 > 32)
		newPixelValue = arg1 + 64 - arg1 % 64;
	else
		newPixelValue = arg1 - arg1 % 64;
	ui->spinBoxPixel->setValue(newPixelValue);
}

void DialogSettings::on_comboBoxSettingsLevel_currentIndexChanged(int index)
{
	switch(index)
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
	on_checkBoxIsCooledCam_stateChanged(ui->checkBoxIsCooledCam->checkState());
}

void DialogSettings::on_comboBoxFftMode_currentIndexChanged(int index)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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
	switch(index)
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
		ui->doubleSpinBoxNos->setEnabled(true);
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
		ui->doubleSpinBoxNos->setEnabled(enabled);
		ui->doubleSpinBoxNos->setValue(ui->spinBoxNumberOfRegions->value());
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
		ui->doubleSpinBoxNos->setEnabled(enabled);
		ui->doubleSpinBoxNos->setValue(ui->spinBoxLines->value() / ui->spinBoxLinesBinning->value());
		break;
	}
}

void DialogSettings::on_pushButtonCopyBtimer_clicked()
{
	ui->doubleSpinBoxContiniousPause_in_ms->setValue(ui->doubleSpinBoxBTimer_in_ms->value());
	return;
}

void DialogSettings::on_pushButtonFilePath_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), nullptr, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(!dir.isEmpty()) ui->plainTextEditFilePath->setPlainText(dir);
	return;
}

void DialogSettings::on_spinBoxLines_valueChanged(int value)
{
	if (ui->comboBoxFftMode->currentIndex() == area_mode)
	{
		ui->doubleSpinBoxNos->setValue(value / ui->spinBoxLinesBinning->value());
	}
}

void DialogSettings::on_spinBoxLinesBinning_valueChanged(int value)
{
	if (ui->comboBoxFftMode->currentIndex() == area_mode)
	{
		ui->doubleSpinBoxNos->setValue(ui->spinBoxLines->value() / value);
	}
}

void DialogSettings::on_spinBoxNumberOfRegions_valueChanged(int value)
{
	if (ui->comboBoxFftMode->currentIndex() == partial_binning)
	{
		ui->doubleSpinBoxNos->setValue(value);
	}
}

void DialogSettings::on_checkBoxWriteDataToDisc_stateChanged(int arg1)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
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

void DialogSettings::on_checkBoxIsCooledCam_stateChanged(int arg1)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
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
	ui->comboBoxCamCool->setEnabled(enabled);
	ui->comboBoxCamCool->setVisible(visible);
	ui->labelCamCool->setVisible(visible);
	return;
}
