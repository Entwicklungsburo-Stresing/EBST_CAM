#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));
    _settings = settings;
    ui->spinBoxNos->setValue(_settings->value(settingNosPath, settingNosDefault).toInt());
    ui->spinBoxNob->setValue(_settings->value(settingNobPath, settingNobDefault).toInt());
    ui->comboBoxSti->setCurrentIndex(_settings->value(settingStiPath, settingStiDefault).toInt());
    ui->comboBoxBti->setCurrentIndex(_settings->value(settingBtiPath, settingBtiDefault).toInt());
    ui->doubleSpinBoxSTimer->setValue(_settings->value(settingStimerPath, settingStimerDefault).toDouble());
    ui->doubleSpinBoxBTimer->setValue(_settings->value(settingBtimerPath, settingBtimerDefault).toDouble());
    ui->spinBoxSdat->setValue(_settings->value(settingSdatPath, settingSdatDefault).toInt());
    ui->spinBoxBdat->setValue(_settings->value(settingBdatPath, settingBdatDefault).toInt());
    ui->comboBoxSslope->setCurrentIndex(_settings->value(settingSslopePath, settingSslopePath).toInt());
    ui->comboBoxBslope->setCurrentIndex(_settings->value(settingBslopePath, settingBslopePath).toInt());
    ui->spinBoxXckdelay->setValue(_settings->value(settingXckdelayPath, settingXckdelayDefault).toInt());
    ui->doubleSpinBoxExpTime->setValue(_settings->value(settingShutterExpTimePath, settingShutterExpTimeDefault).toDouble());
    ui->comboBoxTriggerModeCC->setCurrentIndex(_settings->value(settingXckdelayPath, settingXckdelayDefault).toInt());
    ui->comboBoxBoardSel->setCurrentIndex(_settings->value(settingBoardSelPath, settingBoardSelDefault).toInt());
    ui->comboBoxSensorType->setCurrentIndex(_settings->value(settingSensorTypePath, settingSensorTypeDefault).toInt());
    ui->comboBoxCameraSystem->setCurrentIndex(_settings->value(settingCameraSystemPath, settingCameraSystemDefault).toInt());
    ui->spinBoxCamcnt->setValue(_settings->value(settingCamcntPath, settingCamcntDefault).toInt());
    ui->spinBoxPixel->setValue(_settings->value(settingPixelPath, settingPixelDefault).toInt());
    ui->checkBoxMshut->setChecked(_settings->value(settingMshutPath, settingMshutDefault).toBool());
    ui->checkBoxLed->setChecked(_settings->value(settingLedPath, settingLedDefault).toBool());
    ui->checkBoxGain3010->setChecked(_settings->value(settingGain3010Path, settingGain3010Default).toBool());
    ui->spinBoxGain3030->setValue(_settings->value(settingGain3030Path, settingGain3030Default).toInt());
    ui->comboBoxCamCool->setCurrentIndex(_settings->value(settingCoolingPath, settingCoolingDefault).toInt());
    ui->checkBoxUseDac->setChecked(_settings->value(settingDacPath, settingDacDefault).toBool());
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
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_accepted()
{
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
