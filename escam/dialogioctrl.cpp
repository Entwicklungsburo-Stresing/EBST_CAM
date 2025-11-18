/*****************************************************************//**
 * @file   dialogioctrl.cpp
 * @copydoc dialogioctrl.h
 *********************************************************************/

#include "dialogioctrl.h"
#include "ui_dialogioctrl.h"
#include "lsc-gui.h"

DialogIoctrl::DialogIoctrl(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::DialogIoctrl)
{
	ui->setupUi(this);	
	if (mainWindow->lsc.numberOfBoards > 1)
		ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	// setup ioctrlWidgets
	ui->ioctrlwidgetCh_1->channel = 1;
	ui->ioctrlwidgetCh_2->channel = 2;
	ui->ioctrlwidgetCh_3->channel = 3;
	ui->ioctrlwidgetCh_4->channel = 4;
	ui->ioctrlwidgetCh_5->channel = 5;
	ui->ioctrlwidgetCh_6->channel = 6;
	ui->ioctrlwidgetCh_7->channel = 7;
	ui->ioctrlwidgetCh_8->channel = 8;
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_1, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_2, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_3, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_4, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_5, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_6, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_7, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::defaults_loaded, ui->ioctrlwidgetCh_8, &IoctrlWidget::loadDefaults);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_1, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_2, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_3, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_4, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_5, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_6, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_7, &IoctrlWidget::loadSettings);
	connect(this, &DialogIoctrl::settingsLoaded, ui->ioctrlwidgetCh_8, &IoctrlWidget::loadSettings);
	loadSettings();
}

DialogIoctrl::~DialogIoctrl()
{
	delete ui;
}

void DialogIoctrl::on_comboBoxTrigSource_currentIndexChanged(int index)
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingTriggerSourcePath, index);
	settings.endGroup();
	mainWindow->lsc.setStateControlRegister(ui->spinBoxBoard->value(), index);
	return;
}

void DialogIoctrl::loadSettings()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	ui->comboBoxTrigSource->setCurrentIndex(settings.value(settingTriggerSourcePath, settingTriggerSourceDefault).toInt());
	settings.endGroup();
	emit settingsLoaded((ui->spinBoxBoard->value()));
	return;
}

void DialogIoctrl::on_spinBoxBoard_valueChanged(int value)
{
	(void)value;
	loadSettings();
	return;
}
