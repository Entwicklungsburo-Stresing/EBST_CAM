#include "dialogrms.h"
#include "ui_dialogrms.h"
#include "lsc-gui.h"

DialogRMS::DialogRMS(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogRMS)
{
	ui->setupUi(this);

	// connect all UI changed signals to updateRMS slot
	connect(ui->spinBoxBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogRMS::updateRMS);
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
	//get values from UI
	uint32_t firstSample = ui->spinBox_firstsample->value() - 1;
	uint32_t lastSample = ui->spinBox_lastsample->value() - 1;
	uint32_t pixel = ui->spinBox_pixel->value();
	uint32_t campos = ui->spinBoxCampos->value();
	uint32_t drvno = ui->spinBoxBoard->value();
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
	if (number_of_boards > 1)
		ui->spinBoxBoard->setMaximum(number_of_boards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	on_spinBoxBoard_valueChanged(ui->spinBoxBoard->value());
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

void DialogRMS::on_spinBoxBoard_valueChanged(int index)
{
	settings.beginGroup("board" + QString::number(index));
	int camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
	int pixel = settings.value(settingPixelPath, settingPixelDefault).toUInt();
	settings.endGroup();
	int nos = settings.value(settingNosPath, settingNosDefault).toUInt();
	// set camcnt limit to UI
	if (camcnt > 0)
		ui->spinBoxCampos->setMaximum(camcnt - 1);
	else
		ui->spinBoxCampos->setMaximum(0);
	if (camcnt <= 1)
		ui->spinBoxCampos->setDisabled(true);
	else
		ui->spinBoxCampos->setDisabled(false);
	ui->spinBox_pixel->setMaximum(pixel - 1);
	ui->spinBox_lastsample->setMaximum(nos);
	ui->spinBox_firstsample->setMaximum(nos - 1);
	return;
}