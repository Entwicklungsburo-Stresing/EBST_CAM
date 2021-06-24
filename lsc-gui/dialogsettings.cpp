#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QMessageBox>

DialogSettings::DialogSettings(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));
    _settings = settings;

    // Here the saved settings on the system are applied to the UI.
    // For some settings there are two calls, to trigger the according slot for greying out options. I don't know why this is nesceserry, but without it the slots are not triggered.
    ui->spinBoxNos->setValue(_settings->value(settingNosPath, settingNosDefault).toInt());
    ui->spinBoxNob->setValue(_settings->value(settingNobPath, settingNobDefault).toInt());
    ui->comboBoxSti->setCurrentIndex(_settings->value(settingStiPath, settingStiDefault).toInt());
    ui->comboBoxBti->setCurrentIndex(_settings->value(settingBtiPath, settingBtiDefault).toInt());
    ui->doubleSpinBoxSTimer->setValue(_settings->value(settingStimerPath, settingStimerDefault).toDouble()); //TODO: in microsec
    ui->doubleSpinBoxBTimer->setValue(_settings->value(settingBtimerPath, settingBtimerDefault).toDouble()); //TODO: in microsec
    ui->spinBoxSdat->setValue(_settings->value(settingSdatPath, settingSdatDefault).toInt());
    ui->spinBoxBdat->setValue(_settings->value(settingBdatPath, settingBdatDefault).toInt());
    ui->comboBoxSslope->setCurrentIndex(_settings->value(settingSslopePath, settingSslopeDefault).toInt());
    ui->comboBoxBslope->setCurrentIndex(_settings->value(settingBslopePath, settingBslopeDefault).toInt());
    ui->spinBoxXckdelay->setValue(_settings->value(settingXckdelayPath, settingXckdelayDefault).toInt());
    ui->doubleSpinBoxExpTime->setValue(_settings->value(settingShutterExpTimePath, settingShutterExpTimeDefault).toDouble());
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
    _settings->setValue(settingStimerPath, ui->doubleSpinBoxSTimer->value());
    _settings->setValue(settingBtimerPath, ui->doubleSpinBoxBTimer->value());
    _settings->setValue(settingSdatPath, ui->spinBoxSdat->value());
    _settings->setValue(settingBdatPath, ui->spinBoxBdat->value());
    _settings->setValue(settingSslopePath, ui->comboBoxSslope->currentIndex());
    _settings->setValue(settingBslopePath, ui->comboBoxBslope->currentIndex());
    _settings->setValue(settingXckdelayPath, ui->spinBoxXckdelay->value());
    _settings->setValue(settingShutterExpTimePath , ui->doubleSpinBoxExpTime->value());
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
    emit settings_saved();
    return;
}

void DialogSettings::on_comboBoxSti_currentIndexChanged(int index)
{
    switch(index)
    {
    case 4:
        ui->doubleSpinBoxSTimer->setEnabled(true);
        break;
    default:
        ui->doubleSpinBoxSTimer->setEnabled(false);
    }
}

void DialogSettings::on_comboBoxBti_currentIndexChanged(int index)
{
    switch(index)
    {
    case 4:
        ui->doubleSpinBoxBTimer->setEnabled(true);
        break;
    default:
        ui->doubleSpinBoxBTimer->setEnabled(false);
    }
}

void DialogSettings::on_comboBoxSensorType_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->fftmode->setEnabled(false);
        break;
    default:
        ui->fftmode->setEnabled(true);
    }
}

void DialogSettings::on_checkBoxUseDac_stateChanged(int arg1)
{
    ui->dac->setEnabled(arg1);
}

void DialogSettings::on_comboBoxCameraSystem_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->checkBoxGain3010->setEnabled(true);
        ui->spinBoxGain3030->setEnabled(false);
        ui->checkBoxUseDac->setEnabled(false);
        ui->dac->setEnabled(false);
        ui->comboBoxAdcMode->setEnabled(false);
        ui->spinBoxAdcCustom->setEnabled(false);
        break;
    case 1:
        ui->checkBoxGain3010->setEnabled(true);
        ui->spinBoxGain3030->setEnabled(false);
        ui->checkBoxUseDac->setEnabled(false);
        ui->dac->setEnabled(false);
        ui->comboBoxAdcMode->setEnabled(true);
        ui->spinBoxAdcCustom->setEnabled(true);
        break;
    case 2:
        ui->checkBoxGain3010->setEnabled(false);
        ui->spinBoxGain3030->setEnabled(true);
        ui->checkBoxUseDac->setEnabled(true);
        ui->dac->setEnabled(true);
        ui->comboBoxAdcMode->setEnabled(true);
        ui->spinBoxAdcCustom->setEnabled(true);
        break;
    }
}

void DialogSettings::on_checkBoxMshut_stateChanged(int arg1)
{
    ui->doubleSpinBoxExpTime->setEnabled(arg1);
}

void DialogSettings::on_checkBoxRegionsEqual_stateChanged(int arg1)
{
    ui->spinBoxRegion1->setEnabled(!arg1);
    ui->spinBoxRegion2->setEnabled(!arg1);
    ui->spinBoxRegion3->setEnabled(!arg1);
    ui->spinBoxRegion4->setEnabled(!arg1);
    ui->spinBoxRegion5->setEnabled(!arg1);
    ui->spinBoxRegion6->setEnabled(!arg1);
    ui->spinBoxRegion7->setEnabled(!arg1);
    ui->spinBoxRegion8->setEnabled(!arg1);
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
    ui->doubleSpinBoxSTimer->setValue(settingStimerDefault); //TODO: in microsec
    ui->doubleSpinBoxBTimer->setValue(settingBtimerDefault); //TODO: in microsec
    ui->spinBoxSdat->setValue(settingSdatDefault);
    ui->spinBoxBdat->setValue(settingBdatDefault);
    ui->comboBoxSslope->setCurrentIndex(settingSslopeDefault);
    ui->comboBoxBslope->setCurrentIndex(settingBslopeDefault);
    ui->spinBoxXckdelay->setValue(settingXckdelayDefault);
    ui->doubleSpinBoxExpTime->setValue(settingShutterExpTimeDefault);
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
    return;
}
