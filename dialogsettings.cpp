#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));
    _settings = settings;
    ui->comboBoxTheme->setCurrentIndex(_settings->value(SETTING_THEME, THEME_DEFAULT).toInt());
    ui->comboBoxOutput->setCurrentIndex(_settings->value(SETTING_TOR, TOR_DEFAULT).toInt());
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_accepted()
{
    _settings->setValue(SETTING_NOS, ui->spinBoxNos->value());
    _settings->setValue(SETTING_NOB, ui->spinBoxNob->value());
    _settings->setValue(SETTING_CAMCNT, ui->spinBoxCamcnt->value());
    _settings->setValue(SETTING_THEME, ui->comboBoxTheme->currentIndex());
    _settings->setValue(SETTING_TOR, ui->comboBoxOutput->currentIndex());
    emit settings_saved();
    return;
}
