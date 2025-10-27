#include "dialogfancontrol.h"

DialogFanControl::DialogFanControl(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogFanControlClass())
{
	ui->setupUi(this);
}

DialogFanControl::~DialogFanControl()
{
	delete ui;
}

void DialogFanControl::on_checkBoxFanControlSet_stateChanged(int state)
{
	if (state == Qt::CheckState::Checked)
		settings.setValue(settingMonitorPath, monitor_Vin_Fan_control);
	return;
}

void DialogFanControl::on_checkBoxFanOn_stateChanged(int state)
{
	
	uint32_t drvno = ui->spinBoxPcie->value();
	uint16_t fanState = (state == Qt::CheckState::Checked) ? 1 : 0;
	mainWindow->lsc.setFanControlState(drvno, fanState);
	return;
}



