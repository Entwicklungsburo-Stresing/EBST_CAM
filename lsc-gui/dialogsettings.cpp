#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));
    _settings = settings;
    ui->comboBoxTheme->setCurrentIndex(_settings->value(settingThemePath, settingThemeDefault).toInt());
    ui->comboBoxOutput->setCurrentIndex(_settings->value(settingTorPath, settingTorDefault).toInt());
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_accepted()
{
    _settings->setValue(settingNosPath, ui->spinBoxNos->value());
    _settings->setValue(settingNobPath, ui->spinBoxNob->value());
    _settings->setValue(settingCamcntPath, ui->spinBoxCamcnt->value());
    _settings->setValue(settingThemePath, ui->comboBoxTheme->currentIndex());
    _settings->setValue(settingTorPath, ui->comboBoxOutput->currentIndex());
    _settings->setValue(settingPixelPath, ui->spinBoxPixel->value());
    emit settings_saved();
    return;
}
