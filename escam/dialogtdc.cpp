#include "dialogtdc.h"
#include "ui_dialogtdc.h"
#include <QMessageBox>

#include "lsc-gui.h"

DialogTDC::DialogTDC(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogTDC)
{
	ui->setupUi(this);
	connect(ui->spinBoxBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogTDC::updateTDC);
}

DialogTDC::~DialogTDC()
{
	delete ui;
}

void DialogTDC::updateTDC()
{
	int drvno = ui->spinBoxBoard->value();
	int sample = mainWindow->ui->horizontalSliderSample->value() - 1;
	int block = mainWindow->ui->horizontalSliderBlock->value() - 1;
	uint16_t data[10];
	mainWindow->lsc.returnFrame(drvno, sample, block, 0, data, 10);
	//send pixels 6 to 9 to the tdc window
	//tdc 1: pixel 6 high bytes, 7 low bytes
	//tdc 2: pixel 8 high bytes, 9 low bytes
	uint32_t tdc1 = (static_cast<uint32_t>(*(data + pixel_impact_signal_1_high))) << 16 | (static_cast<uint32_t>(*(data + pixel_impact_signal_1_low)));
	uint32_t tdc2 = (static_cast<uint32_t>(*(data + pixel_impact_signal_2_high))) << 16 | (static_cast<uint32_t>(*(data + pixel_impact_signal_2_low)));
	//pixel 6low/7high of tdc1 and 8low/9high of tdc2 to tdc view
	QString stdc1 = QString::number(tdc1);
	QString stdc2 = QString::number(tdc2);
	ui->viewTDC1->setText(stdc1);
	ui->viewTDC2->setText(stdc2);
	return;
}

void DialogTDC::initDialogTdc()
{
	if (number_of_boards > 1)
		ui->spinBoxBoard->setMaximum(number_of_boards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	return;
}
