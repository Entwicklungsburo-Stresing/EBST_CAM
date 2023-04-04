#include "dialogspecialpixels.h"
#include "lsc-gui.h"

DialogSpecialPixels::DialogSpecialPixels(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogSpecialPixelsClass())
{
	ui->setupUi(this);
	connect(ui->spinBoxBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogSpecialPixels::updateValues);
	connect(ui->spinBoxCamera, qOverload<int>(&QSpinBox::valueChanged), this, &DialogSpecialPixels::updateValues);
	if (number_of_boards > 1)
		ui->spinBoxBoard->setMaximum(number_of_boards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	on_spinBoxBoard_valueChanged(0);
}

DialogSpecialPixels::~DialogSpecialPixels()
{
	delete ui;
}

void DialogSpecialPixels::updateValues()
{
	struct special_pixels sp;
	es_status_codes status = mainWindow->lsc.getAllSpecialPixelInformation(ui->spinBoxBoard->value(), _sample, _block, ui->spinBoxCamera->value(), &sp);
	if (status != es_no_error) return;
	ui->labelOverTempValue->setText(QString::number(sp.overTemp));
	ui->labelTempGoodValue->setText(QString::number(sp.tempGood));
	ui->labelBlockIndexValue->setText(QString::number(sp.blockIndex));
	ui->labelScanIndexValue->setText(QString::number(sp.scanIndex));
	ui->labelScanIndex2Value->setText(QString::number(sp.scanIndex2));
	ui->labelS1Value->setText(QString::number(sp.s1State));
	ui->labelS2Value->setText(QString::number(sp.s2State));
	ui->labelImpact1Value->setText(QString::number(sp.impactSignal1));
	ui->labelImpact2Value->setText(QString::number(sp.impactSignal2));
	ui->labelCameraSystem3001Value->setText(QString::number(sp.cameraSystem3001));
	ui->labelCameraSystem3010Value->setText(QString::number(sp.cameraSystem3010));
	ui->labelCameraSystem3030Value->setText(QString::number(sp.cameraSystem3030));
	return;
}

void DialogSpecialPixels::updateSample(int sample)
{
	_sample = sample - 1;
	updateValues();
	return;
}

void DialogSpecialPixels::updateBlock(int block)
{
	_block = block - 1;
	updateValues();
	return;
}

void DialogSpecialPixels::on_spinBoxBoard_valueChanged(int index)
{
	settings.beginGroup("board" + QString::number(index));
	int camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
	settings.endGroup();
	// set camcnt limit to UI
	if (camcnt > 0)
		ui->spinBoxCamera->setMaximum(camcnt - 1);
	else
		ui->spinBoxCamera->setMaximum(0);
	if (camcnt <= 1)
		ui->spinBoxCamera->setDisabled(true);
	else
		ui->spinBoxCamera->setDisabled(false);
	return;
}
