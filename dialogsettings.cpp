#include "dialogsettings.h"
#include "ui_dialogsettings.h"

DialogSettings::DialogSettings(QSettings* settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_accepted()));
    this->settings = settings;
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_accepted()
{
    this->settings->setValue(SETTING_NOS, ui->spinBoxNos->value());
    this->settings->setValue(SETTING_NOB, ui->spinBoxNob->value());
    this->settings->setValue(SETTING_CAMCNT, ui->spinBoxCamcnt->value());
    return;
}
