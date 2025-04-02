/*****************************************************************//**
 * @file   dialogshutter.cpp
 * @copydoc dialogshutter.h
 *********************************************************************/

#include "dialogshutter.h"

DialogShutter::DialogShutter(QWidget *parent)
	: QDialog(parent, Qt::Dialog | Qt::WindowContextHelpButtonHint | Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	// Set the maximum value of the spin box to the number of boards - 1
	ui.spinBoxPcieBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	// If there is only one board, hide the spin box and label
	if (mainWindow->lsc.numberOfBoards == 1)
	{
		ui.spinBoxPcieBoard->setVisible(false);
		ui.labelPcieBoard->setVisible(false);
	}

	// Connect the shutter check boxes to the same slot
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
	// Cast the parameter state to Qt::CheckState
	connect(ui.checkBoxMshut, &QCheckBox::stateChanged, this, [this](int state) { on_checkBoxMshut_checkStateChanged(static_cast<Qt::CheckState>(state));  });
	connect(ui.checkBoxShutter1, &QCheckBox::stateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
	connect(ui.checkBoxShutter2, &QCheckBox::stateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
	connect(ui.checkBoxShutter3, &QCheckBox::stateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
	connect(ui.checkBoxShutter4, &QCheckBox::stateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
#else
	connect(ui.checkBoxShutter1, &QCheckBox::checkStateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
	connect(ui.checkBoxShutter2, &QCheckBox::checkStateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
	connect(ui.checkBoxShutter3, &QCheckBox::checkStateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
	connect(ui.checkBoxShutter4, &QCheckBox::checkStateChanged, this, &DialogShutter::on_checkBoxShutterX_checkStateChanged);
#endif
	connect(ui.spinBoxPcieBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogShutter::loadSavedValues);
	loadSavedValues();
}

DialogShutter::~DialogShutter()
{}

void DialogShutter::on_checkBoxMshut_checkStateChanged(Qt::CheckState checkState)
{
	switch (checkState)
	{
	case Qt::Checked:
		mainWindow->lsc.openShutter(ui.spinBoxPcieBoard->value());
		break;
	default:
	case Qt::PartiallyChecked:
	case Qt::Unchecked:
		mainWindow->lsc.closeShutter(ui.spinBoxPcieBoard->value());
		break;
	}
	return;
}

void DialogShutter::on_checkBoxShutterX_checkStateChanged()
{
	// Get the state of the check boxes
	Qt::CheckState state_shutter1 = ui.checkBoxShutter1->checkState();
	Qt::CheckState state_shutter2 = ui.checkBoxShutter2->checkState();
	Qt::CheckState state_shutter3 = ui.checkBoxShutter3->checkState();
	Qt::CheckState state_shutter4 = ui.checkBoxShutter4->checkState();
	
	// Put all shutter states into one variable
	uint16_t shutter_states = 0;
	shutter_states |= ((state_shutter1 == Qt::Checked) ? 1 : 0) << ioctrl_shutter_bitindex_shutter1;
	shutter_states |= ((state_shutter2 == Qt::Checked) ? 1 : 0) << ioctrl_shutter_bitindex_shutter2;
	shutter_states |= ((state_shutter3 == Qt::Checked) ? 1 : 0) << ioctrl_shutter_bitindex_shutter3;
	shutter_states |= ((state_shutter4 == Qt::Checked) ? 1 : 0) << ioctrl_shutter_bitindex_shutter4;

	mainWindow->lsc.setShutterStates(ui.spinBoxPcieBoard->value(), shutter_states);
	return;
}

void DialogShutter::on_buttonBox_accepted()
{
	uint32_t drvno = ui.spinBoxPcieBoard->value();
	// Save output values
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingShutterMshutPath, ui.checkBoxMshut->checkState());
	settings.setValue(settingShutter1Path, ui.checkBoxShutter1->checkState());
	settings.setValue(settingShutter2Path, ui.checkBoxShutter2->checkState());
	settings.setValue(settingShutter3Path, ui.checkBoxShutter3->checkState());
	settings.setValue(settingShutter4Path, ui.checkBoxShutter4->checkState());
	settings.endGroup();
	return;
}

void DialogShutter::loadSavedValues()
{
	uint32_t drvno = ui.spinBoxPcieBoard->value();
	settings.beginGroup("board" + QString::number(drvno));
	ui.checkBoxMshut->setCheckState(settings.value(settingShutterMshutPath, Qt::Unchecked).value<Qt::CheckState>());
	// Block signals for the check boxes
	bool blockStateCheckBoxShutter1 = ui.checkBoxShutter1->blockSignals(true);
	bool blockStateCheckBoxShutter2 = ui.checkBoxShutter2->blockSignals(true);
	bool blockStateCheckBoxShutter3 = ui.checkBoxShutter3->blockSignals(true);
	bool blockStateCheckBoxShutter4 = ui.checkBoxShutter4->blockSignals(true);
	ui.checkBoxShutter1->setCheckState(settings.value(settingShutter1Path, Qt::Unchecked).value<Qt::CheckState>());
	ui.checkBoxShutter2->setCheckState(settings.value(settingShutter2Path, Qt::Unchecked).value<Qt::CheckState>());
	ui.checkBoxShutter3->setCheckState(settings.value(settingShutter3Path, Qt::Unchecked).value<Qt::CheckState>());
	ui.checkBoxShutter4->setCheckState(settings.value(settingShutter4Path, Qt::Unchecked).value<Qt::CheckState>());
	// Unblock signals for the check boxes
	ui.checkBoxShutter1->blockSignals(blockStateCheckBoxShutter1);
	ui.checkBoxShutter2->blockSignals(blockStateCheckBoxShutter2);
	ui.checkBoxShutter3->blockSignals(blockStateCheckBoxShutter3);
	ui.checkBoxShutter4->blockSignals(blockStateCheckBoxShutter4);
	settings.endGroup();
	// Set the shutter states
	on_checkBoxShutterX_checkStateChanged();
}
