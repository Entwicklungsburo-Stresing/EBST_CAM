/*****************************************************************//**
 * @file   dialogtriggerinfo.cpp
 * @copydoc dialogtriggerinfo.h
 *********************************************************************/

#include "dialogtriggerinfo.h"
#include "ui_dialogtriggerinfo.h"
#include "lsc-gui.h"

DialogTriggerInfo::DialogTriggerInfo(QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint)
	, ui(new Ui::DialogTriggerInfo)
{
	ui->setupUi(this);
	ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	if(mainWindow->lsc.numberOfBoards > 1)
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
	connect(ui->spinBoxBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogTriggerInfo::on_measureDone);
}

DialogTriggerInfo::~DialogTriggerInfo()
{
	delete ui;
}

void DialogTriggerInfo::on_measureDone()
{
	uint32_t trigger_info_data;
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.getXckLength(drvno, &trigger_info_data);
	ui->doubleSpinBoxXckLength->setValue(((double)trigger_info_data) / 100.);
	mainWindow->lsc.getXckPeriod(drvno, &trigger_info_data);
	ui->doubleSpinBoxXckPeriod->setValue(((double)trigger_info_data) / 100.);
	mainWindow->lsc.getBonLength(drvno, &trigger_info_data);
	ui->doubleSpinBoxBonLength->setValue(((double)trigger_info_data) / 100.);
	mainWindow->lsc.getBonPeriod(drvno, &trigger_info_data);
	ui->doubleSpinBoxBonPeriod->setValue(((double)trigger_info_data) / 100.);
	return;
}
