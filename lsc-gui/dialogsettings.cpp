#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QMessageBox>
#include <QtGlobal>

DialogSettings::DialogSettings(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));
    _settings = settings;
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
	ui->labelExpTime->setSizePolicy(sp_retain);
	ui->labelGain3010->setSizePolicy(sp_retain);
	ui->labelGain3030->setSizePolicy(sp_retain);
	ui->labelLines->setSizePolicy(sp_retain);
	ui->labelLinesBinning->setSizePolicy(sp_retain);
	ui->labelNumberOfRegions->setSizePolicy(sp_retain);
	ui->labelRegionsEqual->setSizePolicy(sp_retain);
	sp_retain = ui->doubleSpinBoxSTime_in_ms->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->doubleSpinBoxSTime_in_ms->setSizePolicy(sp_retain);
	ui->doubleSpinBoxBTimer_in_ms->setSizePolicy(sp_retain);
	ui->doubleSpinBoxExpTimeIn10ns->setSizePolicy(sp_retain);
	sp_retain = ui->checkBoxGain3010->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->checkBoxGain3010->setSizePolicy(sp_retain);
	ui->checkBoxUseDac->setSizePolicy(sp_retain);
	ui->checkBoxUseDac->setSizePolicy(sp_retain);
	ui->checkBoxRegionsEqual->setSizePolicy(sp_retain);
	sp_retain = ui->spinBoxGain3030->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->spinBoxGain3030->setSizePolicy(sp_retain);
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

	// Here the saved settings on the system are applied to the UI.
    // For some settings there are two calls, to trigger the according slot for greying out options. I don't know why this is nesceserry, but without it the slots are not triggered.
    ui->spinBoxNos->setValue(_settings->value(settingNosPath, settingNosDefault).toInt());
    ui->spinBoxNob->setValue(_settings->value(settingNobPath, settingNobDefault).toInt());
    ui->comboBoxSti->setCurrentIndex(_settings->value(settingStiPath, settingStiDefault).toInt());
    ui->comboBoxBti->setCurrentIndex(_settings->value(settingBtiPath, settingBtiDefault).toInt());
    ui->doubleSpinBoxSTime_in_ms->setValue(_settings->value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble() / 1000);
    ui->doubleSpinBoxBTimer_in_ms->setValue(_settings->value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble() / 1000);
    ui->spinBoxSdatIn10ns->setValue(_settings->value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toInt());
    ui->spinBoxBdatIn10ns->setValue(_settings->value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toInt());
    ui->comboBoxSslope->setCurrentIndex(_settings->value(settingSslopePath, settingSslopeDefault).toInt());
    ui->comboBoxBslope->setCurrentIndex(_settings->value(settingBslopePath, settingBslopeDefault).toInt());
    ui->spinBoxXckdelayIn10ns->setValue(_settings->value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toInt());
    ui->doubleSpinBoxExpTimeIn10ns->setValue(_settings->value(settingShutterExpTimeIn10nsPath, settingShutterExpTimeIn10nsDefault).toDouble());
    ui->comboBoxTriggerModeCC->setCurrentIndex(_settings->value(settingTriggerCcPath, settingTriggerCcDefault).toInt());
    ui->comboBoxBoardSel->setCurrentIndex(_settings->value(settingBoardSelPath, settingBoardSelDefault).toInt());
    ui->comboBoxSensorType->setCurrentIndex(_settings->value(settingSensorTypePath, settingSensorTypeDefault).toInt());
    ui->comboBoxSensorType->currentIndexChanged(_settings->value(settingSensorTypePath, settingSensorTypeDefault).toInt());
    ui->comboBoxCameraSystem->setCurrentIndex(_settings->value(settingCameraSystemPath, settingCameraSystemDefault).toInt());
    ui->comboBoxCameraSystem->currentIndexChanged(_settings->value(settingCameraSystemPath, settingCameraSystemDefault).toInt());
    ui->spinBoxCamcnt->setValue(_settings->value(settingCamcntPath, settingCamcntDefault).toInt());
    ui->spinBoxPixel->setValue(_settings->value(settingPixelPath, settingPixelDefault).toInt());
    ui->checkBoxMshut->setChecked(_settings->value(settingMshutPath, settingMshutDefault).toBool());
    ui->checkBoxMshut->stateChanged(_settings->value(settingMshutPath, settingMshutDefault).toBool());
    ui->checkBoxLed->setChecked(_settings->value(settingLedPath, settingLedDefault).toBool());
    ui->checkBoxGain3010->setChecked(_settings->value(settingGain3010Path, settingGain3010Default).toBool());
    ui->spinBoxGain3030->setValue(_settings->value(settingGain3030Path, settingGain3030Default).toInt());
    ui->comboBoxCamCool->setCurrentIndex(_settings->value(settingCoolingPath, settingCoolingDefault).toInt());
    ui->checkBoxUseDac->setChecked(_settings->value(settingDacPath, settingDacDefault).toBool());
    ui->checkBoxUseDac->stateChanged(_settings->value(settingDacPath, settingDacDefault).toBool());
    ui->checkBoxGpx->setChecked(_settings->value(settingGpxPath, settingGpxDefault).toBool());
    ui->spinBoxGpxOffset->setValue(_settings->value(settingGpxOffsetPath, settingGpxOffsetDefault).toInt());
    ui->spinBoxLines->setValue(_settings->value(settingLinesPath, settingLinesDefault).toInt());
    ui->spinBoxVfreq->setValue(_settings->value(settingVfreqPath, settingVfreqDefault).toInt());
    ui->comboBoxFftMode->setCurrentIndex(_settings->value(settingFftModePath, settingFftModeDefault).toInt());
    ui->spinBoxLinesBinning->setValue(_settings->value(settingLinesBinningPath, settingLinesBinningDefault).toInt());
    ui->spinBoxNumberOfRegions->setValue(_settings->value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toInt());
    ui->checkBoxRegionsEqual->setChecked(_settings->value(settingRegionSizeEqualPath, settingRegionSizeEqualDefault).toBool());
    ui->spinBoxRegion1->setValue(_settings->value(settingRegionSize1Path, settingRegionSize1Default).toInt());
    ui->spinBoxRegion2->setValue(_settings->value(settingRegionSize2Path, settingRegionSize2Default).toInt());
    ui->spinBoxRegion3->setValue(_settings->value(settingRegionSize3Path, settingRegionSize3Default).toInt());
    ui->spinBoxRegion4->setValue(_settings->value(settingRegionSize4Path, settingRegionSize4Default).toInt());
    ui->spinBoxRegion5->setValue(_settings->value(settingRegionSize5Path, settingRegionSize5Default).toInt());
    ui->spinBoxRegion6->setValue(_settings->value(settingRegionSize6Path, settingRegionSize6Default).toInt());
    ui->spinBoxRegion7->setValue(_settings->value(settingRegionSize7Path, settingRegionSize7Default).toInt());
    ui->spinBoxRegion8->setValue(_settings->value(settingRegionSize8Path, settingRegionSize8Default).toInt());
    ui->spinBoxChannel1->setValue(_settings->value(settingSensorOffsetChannel1Path, settingSensorOffsetChannel1Default).toInt());
    ui->spinBoxChannel2->setValue(_settings->value(settingSensorOffsetChannel2Path, settingSensorOffsetChannel2Default).toInt());
    ui->spinBoxChannel3->setValue(_settings->value(settingSensorOffsetChannel3Path, settingSensorOffsetChannel3Default).toInt());
    ui->spinBoxChannel4->setValue(_settings->value(settingSensorOffsetChannel4Path, settingSensorOffsetChannel4Default).toInt());
    ui->spinBoxChannel5->setValue(_settings->value(settingSensorOffsetChannel5Path, settingSensorOffsetChannel5Default).toInt());
    ui->spinBoxChannel6->setValue(_settings->value(settingSensorOffsetChannel6Path, settingSensorOffsetChannel6Default).toInt());
    ui->spinBoxChannel7->setValue(_settings->value(settingSensorOffsetChannel7Path, settingSensorOffsetChannel7Default).toInt());
    ui->spinBoxChannel8->setValue(_settings->value(settingSensorOffsetChannel8Path, settingSensorOffsetChannel8Default).toInt());
    ui->comboBoxOutput->setCurrentIndex(_settings->value(settingTorPath, settingTorDefault).toInt());
    ui->comboBoxAdcMode->setCurrentIndex(_settings->value(settingAdcModePath, settingAdcModeDefault).toInt());
    ui->spinBoxAdcCustom->setValue(_settings->value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toInt());
	ui->comboBoxTheme->setCurrentIndex(_settings->value(settingThemePath, settingThemeDefault).toInt());
	ui->comboBoxSettingsLevel->setCurrentIndex(_settings->value(settingSettingsLevelPath, settingSettingsLevelDefault).toInt());
	ui->comboBoxSettingsLevel->currentIndexChanged(ui->comboBoxSettingsLevel->currentIndex());
    setWindowModality(Qt::ApplicationModal);
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_accepted()
{
    //Here the settings on the UI are saved to the system
    _settings->setValue(settingNosPath, ui->spinBoxNos->value());
    _settings->setValue(settingNobPath, ui->spinBoxNob->value());
    _settings->setValue(settingStiPath, ui->comboBoxSti->currentIndex());
    _settings->setValue(settingBtiPath, ui->comboBoxBti->currentIndex());
    _settings->setValue(settingStime_in_microseconds_Path, ui->doubleSpinBoxSTime_in_ms->value() * 1000);
    _settings->setValue(settingBtime_in_microseconds_Path, ui->doubleSpinBoxBTimer_in_ms->value() * 1000);
    _settings->setValue(settingSdat_in_10nsPath, ui->spinBoxSdatIn10ns->value());
    _settings->setValue(settingBdat_in_10nsPath, ui->spinBoxBdatIn10ns->value());
    _settings->setValue(settingSslopePath, ui->comboBoxSslope->currentIndex());
    _settings->setValue(settingBslopePath, ui->comboBoxBslope->currentIndex());
    _settings->setValue(settingXckdelayIn10nsPath, ui->spinBoxXckdelayIn10ns->value());
    _settings->setValue(settingShutterExpTimeIn10nsPath , ui->doubleSpinBoxExpTimeIn10ns->value());
    _settings->setValue(settingTriggerCcPath, ui->comboBoxTriggerModeCC->currentIndex());
    _settings->setValue(settingBoardSelPath, ui->comboBoxBoardSel->currentIndex());
    _settings->setValue(settingSensorTypePath, ui->comboBoxSensorType->currentIndex());
    _settings->setValue(settingCameraSystemPath, ui->comboBoxCameraSystem->currentIndex());
    _settings->setValue(settingCamcntPath, ui->spinBoxCamcnt->value());
    _settings->setValue(settingPixelPath, ui->spinBoxPixel->value());
    _settings->setValue(settingMshutPath, ui->checkBoxMshut->isChecked());
    _settings->setValue(settingLedPath, ui->checkBoxLed->isChecked());
    _settings->setValue(settingGain3010Path, ui->checkBoxGain3010->isChecked());
    _settings->setValue(settingGain3030Path, ui->spinBoxGain3030->value());
    _settings->setValue(settingCoolingPath, ui->comboBoxCamCool->currentIndex());
    _settings->setValue(settingDacPath, ui->checkBoxUseDac->isChecked());
    _settings->setValue(settingGpxPath, ui->checkBoxGpx->isChecked());
    _settings->setValue(settingGpxOffsetPath, ui->spinBoxGpxOffset->value());
    _settings->setValue(settingLinesPath, ui->spinBoxLines->value());
    _settings->setValue(settingVfreqPath, ui->spinBoxVfreq->value());
    _settings->setValue(settingFftModePath, ui->comboBoxFftMode->currentIndex());
    _settings->setValue(settingLinesBinningPath, ui->spinBoxLinesBinning->value());
    _settings->setValue(settingNumberOfRegionsPath, ui->spinBoxNumberOfRegions->value());
    _settings->setValue(settingRegionSizeEqualPath, ui->checkBoxRegionsEqual->isChecked());
    _settings->setValue(settingRegionSize1Path, ui->spinBoxRegion1->value());
    _settings->setValue(settingRegionSize2Path, ui->spinBoxRegion2->value());
    _settings->setValue(settingRegionSize3Path, ui->spinBoxRegion3->value());
    _settings->setValue(settingRegionSize4Path, ui->spinBoxRegion4->value());
    _settings->setValue(settingRegionSize5Path, ui->spinBoxRegion5->value());
    _settings->setValue(settingRegionSize6Path, ui->spinBoxRegion6->value());
    _settings->setValue(settingRegionSize7Path, ui->spinBoxRegion7->value());
    _settings->setValue(settingRegionSize8Path, ui->spinBoxRegion8->value());
    _settings->setValue(settingSensorOffsetChannel1Path, ui->spinBoxChannel1->value());
    _settings->setValue(settingSensorOffsetChannel2Path, ui->spinBoxChannel2->value());
    _settings->setValue(settingSensorOffsetChannel3Path, ui->spinBoxChannel3->value());
    _settings->setValue(settingSensorOffsetChannel4Path, ui->spinBoxChannel4->value());
    _settings->setValue(settingSensorOffsetChannel5Path, ui->spinBoxChannel5->value());
    _settings->setValue(settingSensorOffsetChannel6Path, ui->spinBoxChannel6->value());
    _settings->setValue(settingSensorOffsetChannel7Path, ui->spinBoxChannel7->value());
    _settings->setValue(settingSensorOffsetChannel8Path, ui->spinBoxChannel8->value());
    _settings->setValue(settingTorPath, ui->comboBoxOutput->currentIndex());
    _settings->setValue(settingAdcModePath, ui->comboBoxAdcMode->currentIndex());
    _settings->setValue(settingAdcCustomValuePath, ui->spinBoxAdcCustom->value());
	_settings->setValue(settingThemePath, ui->comboBoxTheme->currentIndex());
	_settings->setValue(settingSettingsLevelPath, ui->comboBoxSettingsLevel->currentIndex());
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
		break;
    default:
		ui->doubleSpinBoxBTimer_in_ms->setEnabled(false || enabled);
		ui->doubleSpinBoxBTimer_in_ms->setVisible(false || visible);
		ui->labelBTimer->setVisible(false || visible);
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
	ui->tabWidget->setTabEnabled(3, enabled);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    ui->tabWidget->setTabVisible(3, visible);
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
		ui->checkBoxGain3010->setEnabled(true || enabled);
		ui->checkBoxGain3010->setVisible(true || visible);
		ui->spinBoxGain3030->setEnabled(false || enabled);
		ui->spinBoxGain3030->setVisible(false || visible);
		ui->checkBoxUseDac->setEnabled(false || enabled);
		ui->checkBoxUseDac->setVisible(false || visible);
		ui->tabWidget->setTabEnabled(3, (false || enabled));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        ui->tabWidget->setTabVisible(3, (false || visible));
#endif
		ui->comboBoxAdcMode->setEnabled(false || enabled);
		ui->comboBoxAdcMode->setVisible(false || visible);
		ui->spinBoxAdcCustom->setEnabled(false || enabled);
		ui->spinBoxAdcCustom->setVisible(false || visible);
		break;
    case 1:
		ui->checkBoxGain3010->setEnabled(true || enabled);
		ui->checkBoxGain3010->setVisible(true || visible);
		ui->spinBoxGain3030->setEnabled(false || enabled);
		ui->spinBoxGain3030->setVisible(false || visible);
		ui->checkBoxUseDac->setEnabled(false || enabled);
		ui->checkBoxUseDac->setVisible(false || visible);
		ui->tabWidget->setTabEnabled(3, (false || enabled));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        ui->tabWidget->setTabVisible(3, (false || visible));
#endif
		ui->comboBoxAdcMode->setEnabled(true || enabled);
		ui->comboBoxAdcMode->setVisible(true || visible);
		ui->spinBoxAdcCustom->setEnabled(true || enabled);
		ui->spinBoxAdcCustom->setVisible(true || visible);
        break;
    case 2:
		ui->checkBoxGain3010->setEnabled(false || enabled);
		ui->checkBoxGain3010->setVisible(false || visible);
		ui->spinBoxGain3030->setEnabled(true || enabled);
		ui->spinBoxGain3030->setVisible(true || visible);
		ui->checkBoxUseDac->setEnabled(true || enabled);
		ui->checkBoxUseDac->setVisible(true || visible);
		ui->tabWidget->setTabEnabled(3, (ui->checkBoxUseDac->checkState() || enabled));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        ui->tabWidget->setTabVisible(3, (ui->checkBoxUseDac->checkState() || visible));
#endif
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
	ui->doubleSpinBoxExpTimeIn10ns->setEnabled(enabled);
	ui->doubleSpinBoxExpTimeIn10ns->setVisible(visible);
	ui->labelExpTime->setVisible(visible);
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
    ui->spinBoxNos->setValue(settingNosDefault);
    ui->spinBoxNob->setValue(settingNobDefault);
    ui->comboBoxSti->setCurrentIndex(settingStiDefault);
    ui->comboBoxBti->setCurrentIndex(settingBtiDefault);
    ui->doubleSpinBoxSTime_in_ms->setValue(settingStime_in_microseconds_Default);
    ui->doubleSpinBoxBTimer_in_ms->setValue(settingBtime_in_microseconds_Default);
    ui->spinBoxSdatIn10ns->setValue(settingSdat_in_10nsDefault);
    ui->spinBoxBdatIn10ns->setValue(settingBdat_in_10nsDefault);
    ui->comboBoxSslope->setCurrentIndex(settingSslopeDefault);
    ui->comboBoxBslope->setCurrentIndex(settingBslopeDefault);
    ui->spinBoxXckdelayIn10ns->setValue(settingXckdelayIn10nsDefault);
    ui->doubleSpinBoxExpTimeIn10ns->setValue(settingShutterExpTimeIn10nsDefault);
    ui->comboBoxTriggerModeCC->setCurrentIndex(settingTriggerCcDefault);
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
    ui->checkBoxGain3010->setChecked(settingGain3010Default);
    ui->spinBoxGain3030->setValue(settingGain3030Default);
    ui->comboBoxCamCool->setCurrentIndex(settingCoolingDefault);
    ui->checkBoxUseDac->setChecked(settingDacDefault);
    ui->checkBoxUseDac->stateChanged(settingDacDefault);
    ui->checkBoxGpx->setChecked(settingGpxDefault);
    ui->spinBoxGpxOffset->setValue(settingGpxOffsetDefault);
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
    ui->spinBoxChannel1->setValue(settingSensorOffsetChannel1Default);
    ui->spinBoxChannel2->setValue(settingSensorOffsetChannel2Default);
    ui->spinBoxChannel3->setValue(settingSensorOffsetChannel3Default);
    ui->spinBoxChannel4->setValue(settingSensorOffsetChannel4Default);
    ui->spinBoxChannel5->setValue(settingSensorOffsetChannel5Default);
    ui->spinBoxChannel6->setValue(settingSensorOffsetChannel6Default);
    ui->spinBoxChannel7->setValue(settingSensorOffsetChannel7Default);
    ui->spinBoxChannel8->setValue(settingSensorOffsetChannel8Default);
    ui->comboBoxOutput->setCurrentIndex(settingTorDefault);
    ui->comboBoxAdcMode->setCurrentIndex(settingAdcModeDefault);
    ui->spinBoxAdcCustom->setValue(settingAdcCustomValueDefault);
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
