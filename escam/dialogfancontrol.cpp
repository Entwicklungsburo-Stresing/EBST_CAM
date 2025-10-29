#include "dialogfancontrol.h"

DialogFanControl::DialogFanControl(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogFanControlClass())
{
	ui->setupUi(this);
	settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
	ui->comboBoxMonitor->setCurrentIndex(settings.value(settingMonitorPath, settingMonitorDefault).toInt());
	settings.value(settingMonitorPath, settingMonitorDefault).toInt() == monitor_Vin_Fan_control ? ui->checkBoxFanOn->setEnabled(true) : ui->checkBoxFanOn->setEnabled(false);
	settings.endGroup();
}

DialogFanControl::~DialogFanControl()
{
	delete ui;
}

void DialogFanControl::on_comboBoxMonitor_currentIndexChanged(int index)
{
	uint32_t drvno = ui->spinBoxPcie->value();
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingMonitorPath, index);
	settings.endGroup();
	index == monitor_Vin_Fan_control ? ui->checkBoxFanOn->setEnabled(true) : ui->checkBoxFanOn->setEnabled(false);
	return;
}

void DialogFanControl::on_checkBoxFanOn_stateChanged(int state)
{
	uint32_t drvno = ui->spinBoxPcie->value();
	uint16_t fanState = (state == Qt::CheckState::Checked) ? 1 : 0;
	mainWindow->lsc.setFanControlState(drvno, fanState);
	return;
}



