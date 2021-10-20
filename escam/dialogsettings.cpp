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
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));

	//don't rearrange widgets when hiding other widgets
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
	ui->labelKeep->setSizePolicy(sp_retain);
	ui->labelSslope->setSizePolicy(sp_retain);
	ui->labelBslope->setSizePolicy(sp_retain);
	ui->labelBoardSel->setSizePolicy(sp_retain);

	sp_retain = ui->doubleSpinBoxSTime_in_ms->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->doubleSpinBoxSTime_in_ms->setSizePolicy(sp_retain);
	ui->doubleSpinBoxBTimer_in_ms->setSizePolicy(sp_retain);

	sp_retain = ui->checkBoxUseDac->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->checkBoxUseDac->setSizePolicy(sp_retain);
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
	ui->comboBoxBoardSel->setSizePolicy(sp_retain);

	// Here the saved settings on the system are applied to the UI.
    // For some settings there are two calls, to trigger the according slot for greying out options. I don't know why this is nesceserry, but without it the slots are not triggered.
	//Measurement
    ui->doubleSpinBoxNos->setValue(settings.value(settingNosPath, settingNosDefault).toDouble());
    ui->doubleSpinBoxNob->setValue(settings.value(settingNobPath, settingNobDefault).toDouble());
    ui->comboBoxSti->setCurrentIndex(settings.value(settingStiPath, settingStiDefault).toUInt());
    ui->comboBoxBti->setCurrentIndex(settings.value(settingBtiPath, settingBtiDefault).toUInt());
    ui->doubleSpinBoxSTime_in_ms->setValue(settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble() / 1000);
    ui->doubleSpinBoxBTimer_in_ms->setValue(settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble() / 1000);
    ui->doubleSpinBoxSdatIn10ns->setValue(settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
    ui->doubleSpinBoxBdatIn10ns->setValue(settings.value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble());
    ui->comboBoxSslope->setCurrentIndex(settings.value(settingSslopePath, settingSslopeDefault).toUInt());
    ui->comboBoxBslope->setCurrentIndex(settings.value(settingBslopePath, settingBslopeDefault).toUInt());
    ui->doubleSpinBoxXckdelayIn10ns->setValue(settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toDouble());
    ui->doubleSpinBoxSecIn10ns->setValue(settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toDouble());
    ui->doubleSpinBoxBecIn10ns->setValue(settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toDouble());
    ui->comboBoxTriggerModeCC->setCurrentIndex(settings.value(settingTriggerCcPath, settingTriggerCcDefault).toUInt());
	//Camera setup
    ui->comboBoxBoardSel->setCurrentIndex(settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt());
    ui->comboBoxSensorType->setCurrentIndex(settings.value(settingSensorTypePath, settingSensorTypeDefault).toUInt());
    ui->comboBoxSensorType->currentIndexChanged(settings.value(settingSensorTypePath, settingSensorTypeDefault).toUInt());
    ui->comboBoxCameraSystem->setCurrentIndex(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt());
    ui->comboBoxCameraSystem->currentIndexChanged(settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt());
    ui->spinBoxCamcnt->setValue(settings.value(settingCamcntPath, settingCamcntDefault).toInt());
    ui->spinBoxPixel->setValue(settings.value(settingPixelPath, settingPixelDefault).toUInt());
    ui->checkBoxMshut->setChecked(settings.value(settingMshutPath, settingMshutDefault).toBool());
    ui->checkBoxMshut->stateChanged(settings.value(settingMshutPath, settingMshutDefault).toBool());
    ui->checkBoxLed->setChecked(settings.value(settingLedPath, settingLedDefault).toBool());
    ui->spinBoxSensorGain->setValue(settings.value(settingSensorGainPath, settingSensorGainDefault).toUInt());
    ui->spinBoxAdcGain->setValue(settings.value(settingAdcGainPath, settingAdcGainDefault).toUInt());
    ui->comboBoxCamCool->setCurrentIndex(settings.value(settingCoolingPath, settingCoolingDefault).toUInt());
    ui->checkBoxUseDac->setChecked(settings.value(settingDacPath, settingDacDefault).toBool());
    ui->checkBoxUseDac->stateChanged(settings.value(settingDacPath, settingDacDefault).toBool());
    ui->checkBoxGpx->setChecked(settings.value(settingGpxPath, settingGpxDefault).toBool());
    ui->spinBoxGpxOffset->setValue(settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toUInt());
	ui->checkBoxIr->setChecked(settings.value(settingIsIrPath, settingIsIrDefault).toBool());
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toUInt());
	//FFT mode
    ui->spinBoxLines->setValue(settings.value(settingLinesPath, settingLinesDefault).toUInt());
    ui->spinBoxVfreq->setValue(settings.value(settingVfreqPath, settingVfreqDefault).toUInt());
    ui->comboBoxFftMode->setCurrentIndex(settings.value(settingFftModePath, settingFftModeDefault).toUInt());
    ui->spinBoxLinesBinning->setValue(settings.value(settingLinesBinningPath, settingLinesBinningDefault).toUInt());
    ui->spinBoxNumberOfRegions->setValue(settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toUInt());
    ui->checkBoxRegionsEqual->setChecked(settings.value(settingRegionSizeEqualPath, settingRegionSizeEqualDefault).toBool());
	QVariant  keep1 = settings.value(settingKeepPath, settingKeepDefault).toUInt() & 0x1;
	QVariant  keep2 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 1) & 0x1;
	QVariant  keep3 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 2) & 0x1;
	QVariant  keep4 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 3) & 0x1;
	QVariant  keep5 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 4) & 0x1;
	QVariant  keep6 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 5) & 0x1;
	QVariant  keep7 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 6) & 0x1;
	QVariant  keep8 = (settings.value(settingKeepPath, settingKeepDefault).toUInt() >> 7) & 0x1;
    ui->checkBoxKeep1->setChecked(keep1.toBool());
    ui->checkBoxKeep2->setChecked(keep2.toBool());
    ui->checkBoxKeep3->setChecked(keep3.toBool());
    ui->checkBoxKeep4->setChecked(keep4.toBool());
    ui->checkBoxKeep5->setChecked(keep5.toBool());
    ui->checkBoxKeep6->setChecked(keep6.toBool());
    ui->checkBoxKeep7->setChecked(keep7.toBool());
    ui->checkBoxKeep8->setChecked(keep8.toBool());
    ui->spinBoxRegion1->setValue(settings.value(settingRegionSize1Path, settingRegionSize1Default).toUInt());
    ui->spinBoxRegion2->setValue(settings.value(settingRegionSize2Path, settingRegionSize2Default).toUInt());
    ui->spinBoxRegion3->setValue(settings.value(settingRegionSize3Path, settingRegionSize3Default).toUInt());
    ui->spinBoxRegion4->setValue(settings.value(settingRegionSize4Path, settingRegionSize4Default).toUInt());
    ui->spinBoxRegion5->setValue(settings.value(settingRegionSize5Path, settingRegionSize5Default).toUInt());
    ui->spinBoxRegion6->setValue(settings.value(settingRegionSize6Path, settingRegionSize6Default).toUInt());
    ui->spinBoxRegion7->setValue(settings.value(settingRegionSize7Path, settingRegionSize7Default).toUInt());
    ui->spinBoxRegion8->setValue(settings.value(settingRegionSize8Path, settingRegionSize8Default).toUInt());
    ui->comboBoxOutput->setCurrentIndex(settings.value(settingTorPath, settingTorDefault).toUInt());
    ui->comboBoxAdcMode->setCurrentIndex(settings.value(settingAdcModePath, settingAdcModeDefault).toUInt());
    ui->spinBoxAdcCustom->setValue(settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toUInt());
	ui->comboBoxTheme->setCurrentIndex(settings.value(settingThemePath, settingThemeDefault).toUInt());
	ui->comboBoxSettingsLevel->setCurrentIndex(settings.value(settingSettingsLevelPath, settingSettingsLevelDefault).toUInt());
	ui->comboBoxSettingsLevel->currentIndexChanged(ui->comboBoxSettingsLevel->currentIndex());

	// hide board select, when there is only one board
	if (number_of_boards == 1)
	{
		ui->comboBoxBoardSel->setCurrentIndex(0);
		ui->labelBoardSel->setVisible(false);
		ui->comboBoxBoardSel->setVisible(false);
		settings.setValue(settingBoardSelPath, 0);
	}

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
    settings.setValue(settingShutterSecIn10nsPath , ui->doubleSpinBoxSecIn10ns->value());
    settings.setValue(settingShutterBecIn10nsPath , ui->doubleSpinBoxBecIn10ns->value());
    settings.setValue(settingTriggerCcPath, ui->comboBoxTriggerModeCC->currentIndex());
	//Camera setup
    settings.setValue(settingBoardSelPath, ui->comboBoxBoardSel->currentIndex());
    settings.setValue(settingSensorTypePath, ui->comboBoxSensorType->currentIndex());
    settings.setValue(settingCameraSystemPath, ui->comboBoxCameraSystem->currentIndex());
    settings.setValue(settingCamcntPath, ui->spinBoxCamcnt->value());
    settings.setValue(settingPixelPath, ui->spinBoxPixel->value());
    settings.setValue(settingMshutPath, ui->checkBoxMshut->isChecked());
    settings.setValue(settingLedPath, ui->checkBoxLed->isChecked());
    settings.setValue(settingSensorGainPath, ui->spinBoxSensorGain->value());
    settings.setValue(settingAdcGainPath, ui->spinBoxAdcGain->value());
    settings.setValue(settingCoolingPath, ui->comboBoxCamCool->currentIndex());
    settings.setValue(settingDacPath, ui->checkBoxUseDac->isChecked());
    settings.setValue(settingGpxPath, ui->checkBoxGpx->isChecked());
    settings.setValue(settingGpxOffsetPath, ui->spinBoxGpxOffset->value());
	settings.setValue(settingIsIrPath, ui->checkBoxIr->isChecked());
	settings.setValue(settingIOCtrlImpactStartPixelPath, ui->spinBoxIOCtrlImpactStartPixel->value());
	//Fft mode
    settings.setValue(settingLinesPath, ui->spinBoxLines->value());
    settings.setValue(settingVfreqPath, ui->spinBoxVfreq->value());
    settings.setValue(settingFftModePath, ui->comboBoxFftMode->currentIndex());
    settings.setValue(settingLinesBinningPath, ui->spinBoxLinesBinning->value());
    settings.setValue(settingNumberOfRegionsPath, ui->spinBoxNumberOfRegions->value());
    settings.setValue(settingRegionSizeEqualPath, ui->checkBoxRegionsEqual->isChecked());
	uint8_t keep = ui->checkBoxKeep1->isChecked();
	keep |= (ui->checkBoxKeep2->isChecked() ? 1 : 0) << 1;
	keep |= (ui->checkBoxKeep3->isChecked() ? 1 : 0) << 2;
	keep |= (ui->checkBoxKeep4->isChecked() ? 1 : 0) << 3;
	keep |= (ui->checkBoxKeep5->isChecked() ? 1 : 0) << 4;
	keep |= (ui->checkBoxKeep6->isChecked() ? 1 : 0) << 5;
	keep |= (ui->checkBoxKeep7->isChecked() ? 1 : 0) << 6;
	keep |= (ui->checkBoxKeep8->isChecked() ? 1 : 0) << 7;
    settings.setValue(settingKeepPath, keep);
    settings.setValue(settingRegionSize1Path, ui->spinBoxRegion1->value());
    settings.setValue(settingRegionSize2Path, ui->spinBoxRegion2->value());
    settings.setValue(settingRegionSize3Path, ui->spinBoxRegion3->value());
    settings.setValue(settingRegionSize4Path, ui->spinBoxRegion4->value());
    settings.setValue(settingRegionSize5Path, ui->spinBoxRegion5->value());
    settings.setValue(settingRegionSize6Path, ui->spinBoxRegion6->value());
    settings.setValue(settingRegionSize7Path, ui->spinBoxRegion7->value());
    settings.setValue(settingRegionSize8Path, ui->spinBoxRegion8->value());
	//Debug
    settings.setValue(settingTorPath, ui->comboBoxOutput->currentIndex());
    settings.setValue(settingAdcModePath, ui->comboBoxAdcMode->currentIndex());
    settings.setValue(settingAdcCustomValuePath, ui->spinBoxAdcCustom->value());
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
		ui->doubleSpinBoxSTime_in_ms->setEnabled(true || enabled);
		ui->doubleSpinBoxSTime_in_ms->setVisible(true || visible);
		ui->labelSTimer->setVisible(true || visible);
		break;
    default:
		ui->doubleSpinBoxSTime_in_ms->setEnabled(false || enabled);
		ui->doubleSpinBoxSTime_in_ms->setVisible(false || visible);
		ui->labelSTimer->setVisible(false || visible);
	}
	switch (index)
	{
	case 0:
	case 1:
	case 2:
		ui->comboBoxSslope->setEnabled(true || enabled);
		ui->comboBoxSslope->setVisible(true || visible);
		ui->labelSslope->setVisible(true || visible);
		break;
	default:
		ui->comboBoxSslope->setEnabled(false || enabled);
		ui->comboBoxSslope->setVisible(false || visible);
		ui->labelSslope->setVisible(false || visible);
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
		ui->doubleSpinBoxBTimer_in_ms->setEnabled(true || enabled);
		ui->doubleSpinBoxBTimer_in_ms->setVisible(true || visible);
		ui->labelBTimer->setVisible(true || visible);
		ui->comboBoxBslope->setEnabled(false || enabled);
		ui->comboBoxBslope->setVisible(false || visible);
		ui->labelBslope->setVisible(false || visible);
		break;
    default:
		ui->doubleSpinBoxBTimer_in_ms->setEnabled(false || enabled);
		ui->doubleSpinBoxBTimer_in_ms->setVisible(false || visible);
		ui->labelBTimer->setVisible(false || visible);
		ui->comboBoxBslope->setEnabled(true || enabled);
		ui->comboBoxBslope->setVisible(true || visible);
		ui->labelBslope->setVisible(true || visible);
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
#endif
}

void DialogSettings::on_checkBoxUseDac_stateChanged(int arg1)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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
	mainWindow->ui->actionDAC->setEnabled(enabled);
	mainWindow->ui->actionDAC->setVisible(visible);
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
		ui->spinBoxAdcGain->setEnabled(false || enabled);
		ui->spinBoxAdcGain->setVisible(false || visible);
		ui->checkBoxUseDac->setEnabled(false || enabled);
		ui->checkBoxUseDac->setVisible(false || visible);
		mainWindow->ui->actionDAC->setEnabled(false || enabled);
		mainWindow->ui->actionDAC->setVisible(false || visible);
		ui->comboBoxAdcMode->setEnabled(false || enabled);
		ui->comboBoxAdcMode->setVisible(false || visible);
		ui->spinBoxAdcCustom->setEnabled(false || enabled);
		ui->spinBoxAdcCustom->setVisible(false || visible);
		break;
    case 1:
		ui->spinBoxAdcGain->setEnabled(false || enabled);
		ui->spinBoxAdcGain->setVisible(false || visible);
		ui->checkBoxUseDac->setEnabled(false || enabled);
		ui->checkBoxUseDac->setVisible(false || visible);
		mainWindow->ui->actionDAC->setEnabled(false || enabled);
		mainWindow->ui->actionDAC->setVisible(false || visible);
		ui->comboBoxAdcMode->setEnabled(true || enabled);
		ui->comboBoxAdcMode->setVisible(true || visible);
		ui->spinBoxAdcCustom->setEnabled(true || enabled);
		ui->spinBoxAdcCustom->setVisible(true || visible);
        break;
    case 2:
		ui->spinBoxAdcGain->setEnabled(true || enabled);
		ui->spinBoxAdcGain->setVisible(true || visible);
		ui->checkBoxUseDac->setEnabled(true || enabled);
		ui->checkBoxUseDac->setVisible(true || visible);
		mainWindow->ui->actionDAC->setEnabled(ui->checkBoxUseDac->checkState() || enabled);
		mainWindow->ui->actionDAC->setVisible(ui->checkBoxUseDac->checkState() || visible);
		ui->comboBoxAdcMode->setEnabled(true || enabled);
		ui->comboBoxAdcMode->setVisible(true || visible);
		ui->spinBoxAdcCustom->setEnabled(true || enabled);
		ui->spinBoxAdcCustom->setVisible(true || visible);
        break;
    }
}

void DialogSettings::on_checkBoxMshut_stateChanged(int arg1)
{
	bool enabled = true,
		 visible = true;
	switch(ui->comboBoxSettingsLevel->currentIndex())
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
	ui->doubleSpinBoxSecIn10ns->setEnabled(enabled);
	ui->doubleSpinBoxSecIn10ns->setVisible(visible);
	ui->labelSec->setVisible(visible);
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
    ui->doubleSpinBoxSTime_in_ms->setValue(settingStime_in_microseconds_Default);
    ui->doubleSpinBoxBTimer_in_ms->setValue(settingBtime_in_microseconds_Default);
    ui->doubleSpinBoxSdatIn10ns->setValue(settingSdat_in_10nsDefault);
    ui->doubleSpinBoxBdatIn10ns->setValue(settingBdat_in_10nsDefault);
    ui->comboBoxSslope->setCurrentIndex(settingSslopeDefault);
    ui->comboBoxBslope->setCurrentIndex(settingBslopeDefault);
    ui->doubleSpinBoxXckdelayIn10ns->setValue(settingXckdelayIn10nsDefault);
    ui->doubleSpinBoxSecIn10ns->setValue(settingShutterSecIn10nsDefault);
    ui->doubleSpinBoxBecIn10ns->setValue(settingShutterBecIn10nsDefault);
    ui->comboBoxTriggerModeCC->setCurrentIndex(settingTriggerCcDefault);
	//camera setup
    ui->comboBoxBoardSel->setCurrentIndex(settingBoardSelDefault);
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
    ui->checkBoxUseDac->setChecked(settingDacDefault);
    ui->checkBoxUseDac->stateChanged(settingDacDefault);
    ui->checkBoxGpx->setChecked(settingGpxDefault);
    ui->spinBoxGpxOffset->setValue(settingGpxOffsetDefault);
	ui->checkBoxRegionsEqual->setChecked(settingIsIrDefault);
	ui->spinBoxIOCtrlImpactStartPixel->setValue(settingIOCtrlImpactStartPixelDefault);
	//fft mode
    ui->spinBoxLines->setValue(settingLinesDefault);
    ui->spinBoxVfreq->setValue(settingVfreqDefault);
    ui->comboBoxFftMode->setCurrentIndex(settingFftModeDefault);
    ui->spinBoxLinesBinning->setValue(settingLinesBinningDefault);
    ui->spinBoxNumberOfRegions->setValue(settingNumberOfRegionsDefault);
    ui->checkBoxRegionsEqual->setChecked(settingRegionSizeEqualDefault);
	uint8_t keep = settingKeepDefault;
	ui->checkBoxKeep1->setChecked(keep & 0x1);
	ui->checkBoxKeep2->setChecked(keep >> 1 & 0x1);
	ui->checkBoxKeep3->setChecked(keep >> 2 & 0x1);
	ui->checkBoxKeep4->setChecked(keep >> 3 & 0x1);
	ui->checkBoxKeep5->setChecked(keep >> 4 & 0x1);
	ui->checkBoxKeep6->setChecked(keep >> 5 & 0x1);
	ui->checkBoxKeep7->setChecked(keep >> 6 & 0x1);
	ui->checkBoxKeep8->setChecked(keep >> 7 & 0x1);
    ui->spinBoxRegion1->setValue(settingRegionSize1Default);
    ui->spinBoxRegion2->setValue(settingRegionSize2Default);
    ui->spinBoxRegion3->setValue(settingRegionSize3Default);
    ui->spinBoxRegion4->setValue(settingRegionSize4Default);
    ui->spinBoxRegion5->setValue(settingRegionSize5Default);
    ui->spinBoxRegion6->setValue(settingRegionSize6Default);
    ui->spinBoxRegion7->setValue(settingRegionSize7Default);
    ui->spinBoxRegion8->setValue(settingRegionSize8Default);
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
        ui->tabWidget->setTabVisible(3, false);
#endif
		ui->tabWidget->setTabEnabled(1, false);
		ui->tabWidget->setTabEnabled(3, false);
		ui->labelLines->setVisible(false);
		ui->spinBoxLines->setVisible(false);
		ui->spinBoxLines->setEnabled(false);
		break;
	case 1:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        ui->tabWidget->setTabVisible(1, true);
        ui->tabWidget->setTabVisible(3, true);
#endif
		ui->tabWidget->setTabEnabled(1, true);
		ui->tabWidget->setTabEnabled(3, true);
		ui->labelLines->setVisible(true);
		ui->spinBoxLines->setVisible(true);
		ui->spinBoxLines->setEnabled(true);
		break;
	case 2:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        ui->tabWidget->setTabVisible(3, true);
        ui->tabWidget->setTabVisible(1, true);
#endif
		ui->tabWidget->setTabEnabled(1, true);
		ui->tabWidget->setTabEnabled(3, true);
		ui->labelLines->setVisible(true);
		ui->spinBoxLines->setVisible(true);
		ui->spinBoxLines->setEnabled(true);
		break;
	}
	//run all slots to apply visible and enabled changes
	on_comboBoxSti_currentIndexChanged(ui->comboBoxSti->currentIndex());
	on_comboBoxBti_currentIndexChanged(ui->comboBoxBti->currentIndex());
	on_comboBoxSensorType_currentIndexChanged(ui->comboBoxSensorType->currentIndex());
	on_checkBoxUseDac_stateChanged(ui->checkBoxUseDac->checkState());
	on_comboBoxCameraSystem_currentIndexChanged(ui->comboBoxCameraSystem->currentIndex());
	on_checkBoxMshut_stateChanged(ui->checkBoxMshut->checkState());
	on_checkBoxRegionsEqual_stateChanged(ui->checkBoxRegionsEqual->checkState());
	on_comboBoxFftMode_currentIndexChanged(ui->comboBoxFftMode->currentIndex());
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
	case 0:
		ui->labelLinesBinning->setVisible(false || visible);
		ui->spinBoxLinesBinning->setEnabled(false || enabled);
		ui->spinBoxLinesBinning->setVisible(false || visible);
		ui->labelNumberOfRegions->setVisible(false || visible);
		ui->spinBoxNumberOfRegions->setEnabled(false || enabled);
		ui->spinBoxNumberOfRegions->setVisible(false || visible);
		ui->labelRegionsEqual->setVisible(false || visible);
		ui->checkBoxRegionsEqual->setVisible(false || visible);
		ui->checkBoxRegionsEqual->setEnabled(false || enabled);
		ui->labelKeep->setVisible(false || visible);
		ui->checkBoxKeep1->setVisible(false || visible);
		ui->checkBoxKeep2->setVisible(false || visible);
		ui->checkBoxKeep3->setVisible(false || visible);
		ui->checkBoxKeep4->setVisible(false || visible);
		ui->checkBoxKeep5->setVisible(false || visible);
		ui->checkBoxKeep6->setVisible(false || visible);
		ui->checkBoxKeep7->setVisible(false || visible);
		ui->checkBoxKeep8->setVisible(false || visible);
		ui->checkBoxKeep1->setEnabled(false || enabled);
		ui->checkBoxKeep2->setEnabled(false || enabled);
		ui->checkBoxKeep3->setEnabled(false || enabled);
		ui->checkBoxKeep4->setEnabled(false || enabled);
		ui->checkBoxKeep5->setEnabled(false || enabled);
		ui->checkBoxKeep6->setEnabled(false || enabled);
		ui->checkBoxKeep7->setEnabled(false || enabled);
		ui->checkBoxKeep8->setEnabled(false || enabled);
		ui->labelRegion1->setVisible(false || visible);
		ui->labelRegion2->setVisible(false || visible);
		ui->labelRegion3->setVisible(false || visible);
		ui->labelRegion4->setVisible(false || visible);
		ui->labelRegion5->setVisible(false || visible);
		ui->labelRegion6->setVisible(false || visible);
		ui->labelRegion7->setVisible(false || visible);
		ui->labelRegion8->setVisible(false || visible);
		ui->spinBoxRegion1->setVisible(false || visible);
		ui->spinBoxRegion2->setVisible(false || visible);
		ui->spinBoxRegion3->setVisible(false || visible);
		ui->spinBoxRegion4->setVisible(false || visible);
		ui->spinBoxRegion5->setVisible(false || visible);
		ui->spinBoxRegion6->setVisible(false || visible);
		ui->spinBoxRegion7->setVisible(false || visible);
		ui->spinBoxRegion8->setVisible(false || visible);
		ui->spinBoxRegion1->setEnabled(false || enabled);
		ui->spinBoxRegion2->setEnabled(false || enabled);
		ui->spinBoxRegion3->setEnabled(false || enabled);
		ui->spinBoxRegion4->setEnabled(false || enabled);
		ui->spinBoxRegion5->setEnabled(false || enabled);
		ui->spinBoxRegion6->setEnabled(false || enabled);
		ui->spinBoxRegion7->setEnabled(false || enabled);
		ui->spinBoxRegion8->setEnabled(false || enabled);
		break;
	case 1:
		ui->labelLinesBinning->setVisible(false || visible);
		ui->spinBoxLinesBinning->setEnabled(false || enabled);
		ui->spinBoxLinesBinning->setVisible(false || visible);
		ui->labelNumberOfRegions->setVisible(true || visible);
		ui->spinBoxNumberOfRegions->setEnabled(true || enabled);
		ui->spinBoxNumberOfRegions->setVisible(true || visible);
		ui->labelRegionsEqual->setVisible(true || visible);
		ui->checkBoxRegionsEqual->setVisible(true || visible);
		ui->checkBoxRegionsEqual->setEnabled(true || enabled);
		ui->labelKeep->setVisible(true || visible);
		ui->checkBoxKeep1->setVisible(true || visible);
		ui->checkBoxKeep2->setVisible(true || visible);
		ui->checkBoxKeep3->setVisible(true || visible);
		ui->checkBoxKeep4->setVisible(true || visible);
		ui->checkBoxKeep5->setVisible(true || visible);
		ui->checkBoxKeep6->setVisible(true || visible);
		ui->checkBoxKeep7->setVisible(true || visible);
		ui->checkBoxKeep8->setVisible(true || visible);
		ui->checkBoxKeep1->setEnabled(true || enabled);
		ui->checkBoxKeep2->setEnabled(true || enabled);
		ui->checkBoxKeep3->setEnabled(true || enabled);
		ui->checkBoxKeep4->setEnabled(true || enabled);
		ui->checkBoxKeep5->setEnabled(true || enabled);
		ui->checkBoxKeep6->setEnabled(true || enabled);
		ui->checkBoxKeep7->setEnabled(true || enabled);
		ui->checkBoxKeep8->setEnabled(true || enabled);
		ui->labelRegion1->setVisible(true || visible);
		ui->labelRegion2->setVisible(true || visible);
		ui->labelRegion3->setVisible(true || visible);
		ui->labelRegion4->setVisible(true || visible);
		ui->labelRegion5->setVisible(true || visible);
		ui->labelRegion6->setVisible(true || visible);
		ui->labelRegion7->setVisible(true || visible);
		ui->labelRegion8->setVisible(true || visible);
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
	case 2:
		ui->labelLinesBinning->setVisible(true || visible);
		ui->spinBoxLinesBinning->setEnabled(true || enabled);
		ui->spinBoxLinesBinning->setVisible(true || visible);
		ui->labelNumberOfRegions->setVisible(false || visible);
		ui->spinBoxNumberOfRegions->setEnabled(false || enabled);
		ui->spinBoxNumberOfRegions->setVisible(false || visible);
		ui->labelRegionsEqual->setVisible(false || visible);
		ui->checkBoxRegionsEqual->setVisible(false || visible);
		ui->checkBoxRegionsEqual->setEnabled(false || enabled);
		ui->labelKeep->setVisible(false || visible);
		ui->checkBoxKeep1->setVisible(false || visible);
		ui->checkBoxKeep2->setVisible(false || visible);
		ui->checkBoxKeep3->setVisible(false || visible);
		ui->checkBoxKeep4->setVisible(false || visible);
		ui->checkBoxKeep5->setVisible(false || visible);
		ui->checkBoxKeep6->setVisible(false || visible);
		ui->checkBoxKeep7->setVisible(false || visible);
		ui->checkBoxKeep8->setVisible(false || visible);
		ui->checkBoxKeep1->setEnabled(false || enabled);
		ui->checkBoxKeep2->setEnabled(false || enabled);
		ui->checkBoxKeep3->setEnabled(false || enabled);
		ui->checkBoxKeep4->setEnabled(false || enabled);
		ui->checkBoxKeep5->setEnabled(false || enabled);
		ui->checkBoxKeep6->setEnabled(false || enabled);
		ui->checkBoxKeep7->setEnabled(false || enabled);
		ui->checkBoxKeep8->setEnabled(false || enabled);
		ui->labelRegion1->setVisible(false || visible);
		ui->labelRegion2->setVisible(false || visible);
		ui->labelRegion3->setVisible(false || visible);
		ui->labelRegion4->setVisible(false || visible);
		ui->labelRegion5->setVisible(false || visible);
		ui->labelRegion6->setVisible(false || visible);
		ui->labelRegion7->setVisible(false || visible);
		ui->labelRegion8->setVisible(false || visible);
		ui->spinBoxRegion1->setVisible(false || visible);
		ui->spinBoxRegion2->setVisible(false || visible);
		ui->spinBoxRegion3->setVisible(false || visible);
		ui->spinBoxRegion4->setVisible(false || visible);
		ui->spinBoxRegion5->setVisible(false || visible);
		ui->spinBoxRegion6->setVisible(false || visible);
		ui->spinBoxRegion7->setVisible(false || visible);
		ui->spinBoxRegion8->setVisible(false || visible);
		ui->spinBoxRegion1->setEnabled(false || enabled);
		ui->spinBoxRegion2->setEnabled(false || enabled);
		ui->spinBoxRegion3->setEnabled(false || enabled);
		ui->spinBoxRegion4->setEnabled(false || enabled);
		ui->spinBoxRegion5->setEnabled(false || enabled);
		ui->spinBoxRegion6->setEnabled(false || enabled);
		ui->spinBoxRegion7->setEnabled(false || enabled);
		ui->spinBoxRegion8->setEnabled(false || enabled);
		break;
	}
}
