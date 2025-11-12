#include "dialogfancontrol.h"

DialogFanControl::DialogFanControl(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogFanControlClass())
{
	ui->setupUi(this);
	initDialogFanControl();
}

DialogFanControl::~DialogFanControl()
{
	delete ui;
}

/**
 * @brief Initializes the Fan Control dialog. Disables the PCIe selection if only one board is present.
 * @return none
 */
void DialogFanControl::initDialogFanControl()
{
	if (mainWindow->lsc.numberOfBoards > 1)
		ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	on_spinBoxBoard_valueChanged(ui->spinBoxBoard->value());
	return;
}

/**
 * @brief Loads saved values from the settings, when the selected board changes.
 * @return none
 */
void DialogFanControl::on_spinBoxBoard_valueChanged(int index)
{
	settings.beginGroup("board" + QString::number(index));
	int monitorIndex = settings.value(settingMonitorPath, settingMonitorDefault).toInt();
	int fanState = settings.value(settingFanControlFanStatePath, settingFanControlFanStateDefault).toInt();
	settings.endGroup();
	ui->comboBoxMonitor->setCurrentIndex(monitorIndex);
	ui->checkBoxFanOn->setChecked(fanState == 1);
	updateFanCheckboxState(monitorIndex);
	return;
}

/**
 * @brief Saves the selected monitor state to the settings. Disable if fan control is not selected.
 * @return none
 */
void DialogFanControl::on_comboBoxMonitor_currentIndexChanged(int index)
{
	uint32_t drvno = ui->spinBoxBoard->value();
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingMonitorPath, index);
	settings.endGroup();
	updateFanCheckboxState(index);
	return;
}

/**
 * @brief Saves the selected fan state to the settings and applies it to the selected camera.
 * @return none
 */
void DialogFanControl::on_checkBoxFanOn_stateChanged(int state)
{
	uint32_t drvno = ui->spinBoxBoard->value();
	uint16_t fanState = (state == Qt::Checked) ? 1 : 0;
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingFanControlFanStatePath, fanState);
	mainWindow->lsc.setFanControlState(drvno, fanState);
	settings.endGroup();
	return;
}

void DialogFanControl::updateFanCheckboxState(int monitorIndex)
{
	ui->checkBoxFanOn->setEnabled(monitorIndex == monitor_Vin_Fan_control);
}



