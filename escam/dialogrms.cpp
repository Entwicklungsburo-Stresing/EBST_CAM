#include "dialogrms.h"
#include "ui_dialogrms.h"
#include "lsc-gui.h"

DialogRMS::DialogRMS(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRMS)
{
    ui->setupUi(this);

	// connect all ui changed signals to updateRMS slot
	connect(ui->comboBoxDrvno, qOverload<int>(&QComboBox::currentIndexChanged), this, &DialogRMS::updateRMS);
	connect(ui->spinBoxCampos, qOverload<int>(&QSpinBox::valueChanged), this, &DialogRMS::updateRMS);
	connect(ui->spinBox_firstsample, qOverload<int>(&QSpinBox::valueChanged), this, &DialogRMS::updateRMS);
	connect(ui->spinBox_lastsample, qOverload<int>(&QSpinBox::valueChanged), this, &DialogRMS::updateRMS);
	connect(ui->spinBox_pixel, qOverload<int>(&QSpinBox::valueChanged), this, &DialogRMS::updateRMS);
}

DialogRMS::~DialogRMS()
{
    delete ui;
}

void DialogRMS::updateRMS()
{
	//get values from ui
	uint32_t firstSample = ui->spinBox_firstsample->value() - 1;
	uint32_t lastSample = ui->spinBox_lastsample->value() - 1;
	uint32_t pixel = ui->spinBox_pixel->value();
	uint32_t campos = ui->spinBoxCampos->value() - 1;
	uint32_t drvno = ui->comboBoxDrvno->currentIndex() + 1;
	double mwf, trms;
	QString smwf, strms;

	//calculate trms
	mainWindow->lsc.calcTRMS( drvno, firstSample, lastSample, pixel, campos, &mwf, &trms );
	//convert the numbers to strings
	smwf = QString::number( mwf );
	strms = QString::number( trms );
	//show values
	ui->label_mwf->setText( smwf );
	ui->label_trms->setText( strms );
}

void DialogRMS::initDialogRMS()
{
	if (number_of_boards == 1)
	{
		ui->comboBoxDrvno->setDisabled(true);
		ui->comboBoxDrvno->setCurrentIndex(0);
	}
	int camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
	ui->spinBoxCampos->setMaximum(camcnt);
	if (camcnt == 1)
		ui->spinBoxCampos->setDisabled(true);
	int pixel = settings.value(settingPixelPath, settingPixelDefault).toUInt();
	ui->spinBox_pixel->setMaximum(pixel - 1);
	int nos = settings.value(settingNosPath, settingNosDefault).toUInt();
	ui->spinBox_lastsample->setMaximum(nos);
	ui->spinBox_firstsample->setMaximum(nos - 1);
	return;
}

void DialogRMS::on_spinBox_firstsample_valueChanged(int value)
{
	if (value >= ui->spinBox_lastsample->value())
		ui->spinBox_lastsample->setValue(value + 1);
	return;
}

void DialogRMS::on_spinBox_lastsample_valueChanged(int value)
{
	if (value <= ui->spinBox_firstsample->value())
		ui->spinBox_firstsample->setValue(value - 1);
	return;
}