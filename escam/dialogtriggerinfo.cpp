#include "dialogtriggerinfo.h"
#include "ui_dialogtriggerinfo.h"
#include "lsc-gui.h"

DialogTriggerInfo::DialogTriggerInfo(QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint)
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
	on_measureDone();
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
	ui->doubleSpinBoxXckLength->setValue(((double)data) / 100.);
	mainWindow->lsc.getXckPeriod(drvno, &data);
	ui->doubleSpinBoxXckPeriod->setValue(((double)data) / 100.);
	mainWindow->lsc.getBonLength(drvno, &data);
	ui->doubleSpinBoxBonLength->setValue(((double)data) / 100.);
	mainWindow->lsc.getBonPeriod(drvno, &data);
	ui->doubleSpinBoxBonPeriod->setValue(((double)data) / 100.);
	return;
}
