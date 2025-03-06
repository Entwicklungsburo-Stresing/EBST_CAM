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
	// Connect the shutter check boxes with lambda syntax to the same slot
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
	connect(ui.checkBoxShutter1, &QCheckBox::stateChanged, this, [this](int state) { on_checkBoxShutterX_checkStateChanged(static_cast<Qt::CheckState>(state), 0); });
	connect(ui.checkBoxShutter2, &QCheckBox::stateChanged, this, [this](int state) { on_checkBoxShutterX_checkStateChanged(static_cast<Qt::CheckState>(state), 1); });
	connect(ui.checkBoxShutter3, &QCheckBox::stateChanged, this, [this](int state) { on_checkBoxShutterX_checkStateChanged(static_cast<Qt::CheckState>(state), 2); });
	connect(ui.checkBoxShutter4, &QCheckBox::stateChanged, this, [this](int state) { on_checkBoxShutterX_checkStateChanged(static_cast<Qt::CheckState>(state), 3); });

#else
	connect(ui.checkBoxShutter1, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState checkState) { on_checkBoxShutterX_checkStateChanged(checkState, 0); });
	connect(ui.checkBoxShutter2, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState checkState) { on_checkBoxShutterX_checkStateChanged(checkState, 1); });
	connect(ui.checkBoxShutter3, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState checkState) { on_checkBoxShutterX_checkStateChanged(checkState, 2); });
	connect(ui.checkBoxShutter4, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState checkState) { on_checkBoxShutterX_checkStateChanged(checkState, 3); });
#endif

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

void DialogShutter::on_checkBoxShutterX_checkStateChanged(Qt::CheckState checkState, int shutterNumber)
{
	return;
}
