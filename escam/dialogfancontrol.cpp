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
		ui->spinBoxPcie->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxPcie->setVisible(false);
		ui->labelPcie->setVisible(false);
	}
	on_spinBoxBoard_valueChanged(ui->spinBoxPcie->value());
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
	ui->checkBoxFanOn->setChecked(fanState == 1 ? true : false);
	return;
}

/**
 * @brief Saves the selected monitor state to the settings. Disable if fan control is not selected.
 * @return none
 */
void DialogFanControl::on_comboBoxMonitor_currentIndexChanged(int index)
{
	uint32_t drvno = ui->spinBoxPcie->value();
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingMonitorPath, index);
	settings.endGroup();
	index == monitor_Vin_Fan_control ? ui->checkBoxFanOn->setEnabled(true) : ui->checkBoxFanOn->setEnabled(false);
	return;
}

/**
 * @brief Saves the selected fan state to the settings and applies it to the selected camera.
 * @return none
 */
void DialogFanControl::on_checkBoxFanOn_stateChanged(int state)
{
	uint32_t drvno = ui->spinBoxPcie->value();
	uint16_t fanState = (state == Qt::CheckState::Checked) ? 1 : 0;
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingFanControlFanStatePath, fanState);
	mainWindow->lsc.setFanControlState(drvno, fanState);
	return;
}



