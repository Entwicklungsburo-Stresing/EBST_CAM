#include "dialogtriggerinfo.h"
#include "ui_dialogtriggerinfo.h"
#include "lsc-gui.h"

DialogTriggerInfo::DialogTriggerInfo(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogTriggerInfo)
{
	ui->setupUi(this);
	ui->spinBoxBoard->setMaximum(number_of_boards - 1);
	if(number_of_boards > 1)
	{
		ui->spinBoxBoard->setVisible(true);
		ui->labelBoard->setVisible(true);
	}
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
		ui->spinBoxBoard->setValue(0);
	}
}

DialogTriggerInfo::~DialogTriggerInfo()
{
	delete ui;
}

void DialogTriggerInfo::on_measureDone()
{
	uint32_t data;
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.getXckLength(drvno, &data);
	ui->doubleSpinBoxXckLength->setValue(data / 1000);
	mainWindow->lsc.getXckPeriod(drvno, &data);
	ui->doubleSpinBoxXckPeriod->setValue(data / 1000);
	mainWindow->lsc.getBonLength(drvno, &data);
	ui->doubleSpinBoxBonLength->setValue(data / 1000);
	mainWindow->lsc.getBonPeriod(drvno, &data);
	ui->doubleSpinBoxBonPeriod->setValue(data / 1000);
	return;
}
