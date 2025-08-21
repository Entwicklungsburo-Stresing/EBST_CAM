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
	// Connect spin boxes to slots
	// Lambda syntax is used to pass additional arguments
	connect(ui->spinBoxO1D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(1, ui->spinBoxO1W->value(), ui->spinBoxO1D->value()); });
	connect(ui->spinBoxO2D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(2, ui->spinBoxO2W->value(), ui->spinBoxO2D->value()); });
	connect(ui->spinBoxO3D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(3, ui->spinBoxO3W->value(), ui->spinBoxO3D->value()); });
	connect(ui->spinBoxO4D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(4, ui->spinBoxO4W->value(), ui->spinBoxO4D->value()); });
	connect(ui->spinBoxO5D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(5, ui->spinBoxO5W->value(), ui->spinBoxO5D->value()); });
	connect(ui->spinBoxO6D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(6, ui->spinBoxO6W->value(), ui->spinBoxO6D->value()); });
	connect(ui->spinBoxO7D, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(7, ui->spinBoxO7W->value(), ui->spinBoxO7D->value()); });
	connect(ui->spinBoxO1W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(1, ui->spinBoxO1W->value(), ui->spinBoxO1D->value()); });
	connect(ui->spinBoxO2W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(2, ui->spinBoxO2W->value(), ui->spinBoxO2D->value()); });
	connect(ui->spinBoxO3W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(3, ui->spinBoxO3W->value(), ui->spinBoxO3D->value()); });
	connect(ui->spinBoxO4W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(4, ui->spinBoxO4W->value(), ui->spinBoxO4D->value()); });
	connect(ui->spinBoxO5W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(5, ui->spinBoxO5W->value(), ui->spinBoxO5D->value()); });
	connect(ui->spinBoxO6W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(6, ui->spinBoxO6W->value(), ui->spinBoxO6D->value()); });
	connect(ui->spinBoxO7W, qOverload<int>(&QSpinBox::valueChanged), this, [this] {setOutput(7, ui->spinBoxO7W->value(), ui->spinBoxO7D->value()); });

	connect(ui->spinBoxO1D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO2D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO3D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO4D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO5D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO6D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO7D, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO1W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO2W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO3W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO4W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO5W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO6W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);
	connect(ui->spinBoxO7W, qOverload<int>(&QSpinBox::valueChanged), this, &DialogIoctrl::spinBox_valueChanged);

	connect(ui->doubleSpinBoxT0, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DialogIoctrl::setT0);
	if (mainWindow->lsc.numberOfBoards > 1)
		ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	// apply saved values to UI
	on_spinBoxBoard_valueChanged();
}

DialogIoctrl::~DialogIoctrl()
{
}

void DialogIoctrl::setOutput(uint8_t outputNumber, uint16_t width_in_5ns, uint16_t delay_in_5ns)
{
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.ioctrl_setOutput(drvno, outputNumber, width_in_5ns, delay_in_5ns);
	return;
}

void DialogIoctrl::setT0(uint32_t period_in_10ns)
{
	uint32_t drvno = ui->spinBoxBoard->value();
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingIOCtrlT0PeriodIn10nsPath, ui->doubleSpinBoxT0->value());
	settings.endGroup();
	mainWindow->lsc.ioctrl_setT0(drvno, period_in_10ns);
	return;
}

void DialogIoctrl::spinBox_valueChanged() {
	uint32_t drvno = ui->spinBoxBoard->value();
	// Save output values
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingIOCtrlOutput1DelayIn5nsPath, ui->spinBoxO1D->value());
	settings.setValue(settingIOCtrlOutput2DelayIn5nsPath, ui->spinBoxO2D->value());
	settings.setValue(settingIOCtrlOutput3DelayIn5nsPath, ui->spinBoxO3D->value());
	settings.setValue(settingIOCtrlOutput4DelayIn5nsPath, ui->spinBoxO4D->value());
	settings.setValue(settingIOCtrlOutput5DelayIn5nsPath, ui->spinBoxO5D->value());
	settings.setValue(settingIOCtrlOutput6DelayIn5nsPath, ui->spinBoxO6D->value());
	settings.setValue(settingIOCtrlOutput7DelayIn5nsPath, ui->spinBoxO7D->value());
	settings.setValue(settingIOCtrlOutput1WidthIn5nsPath, ui->spinBoxO1W->value());
	settings.setValue(settingIOCtrlOutput2WidthIn5nsPath, ui->spinBoxO2W->value());
	settings.setValue(settingIOCtrlOutput3WidthIn5nsPath, ui->spinBoxO3W->value());
	settings.setValue(settingIOCtrlOutput4WidthIn5nsPath, ui->spinBoxO4W->value());
	settings.setValue(settingIOCtrlOutput5WidthIn5nsPath, ui->spinBoxO5W->value());
	settings.setValue(settingIOCtrlOutput6WidthIn5nsPath, ui->spinBoxO6W->value());
	settings.setValue(settingIOCtrlOutput7WidthIn5nsPath, ui->spinBoxO7W->value());
	settings.setValue(settingIOCtrlT0PeriodIn10nsPath, ui->doubleSpinBoxT0->value());
	settings.endGroup();
	return;
}

void DialogIoctrl::on_pushButtonDefault_pressed()
{
	uint32_t drvno = ui->spinBoxBoard->value();
	// Save output values
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingIOCtrlOutput1DelayIn5nsPath, settingIOCtrlOutput1DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput2DelayIn5nsPath, settingIOCtrlOutput2DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput3DelayIn5nsPath, settingIOCtrlOutput3DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput4DelayIn5nsPath, settingIOCtrlOutput4DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput5DelayIn5nsPath, settingIOCtrlOutput5DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput6DelayIn5nsPath, settingIOCtrlOutput6DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput7DelayIn5nsPath, settingIOCtrlOutput7DelayIn5nsDefault);
	settings.setValue(settingIOCtrlOutput1WidthIn5nsPath, settingIOCtrlOutput1WidthIn5nsDefault);
	settings.setValue(settingIOCtrlOutput2WidthIn5nsPath, settingIOCtrlOutput2WidthIn5nsDefault);
	settings.setValue(settingIOCtrlOutput3WidthIn5nsPath, settingIOCtrlOutput3WidthIn5nsDefault);
	settings.setValue(settingIOCtrlOutput4WidthIn5nsPath, settingIOCtrlOutput4WidthIn5nsDefault);
	settings.setValue(settingIOCtrlOutput5WidthIn5nsPath, settingIOCtrlOutput5WidthIn5nsDefault);
	settings.setValue(settingIOCtrlOutput6WidthIn5nsPath, settingIOCtrlOutput6WidthIn5nsDefault);
	settings.setValue(settingIOCtrlOutput7WidthIn5nsPath, settingIOCtrlOutput7WidthIn5nsDefault);
	settings.setValue(settingIOCtrlT0PeriodIn10nsPath, ui->doubleSpinBoxT0->value());
	settings.endGroup();

	ui->spinBoxO1D->setValue(settingIOCtrlOutput1DelayIn5nsDefault);
	ui->spinBoxO2D->setValue(settingIOCtrlOutput2DelayIn5nsDefault);
	ui->spinBoxO3D->setValue(settingIOCtrlOutput3DelayIn5nsDefault);
	ui->spinBoxO4D->setValue(settingIOCtrlOutput4DelayIn5nsDefault);
	ui->spinBoxO5D->setValue(settingIOCtrlOutput5DelayIn5nsDefault);
	ui->spinBoxO6D->setValue(settingIOCtrlOutput6DelayIn5nsDefault);
	ui->spinBoxO7D->setValue(settingIOCtrlOutput7DelayIn5nsDefault);
	ui->spinBoxO1W->setValue(settingIOCtrlOutput1WidthIn5nsDefault);
	ui->spinBoxO2W->setValue(settingIOCtrlOutput2WidthIn5nsDefault);
	ui->spinBoxO3W->setValue(settingIOCtrlOutput3WidthIn5nsDefault);
	ui->spinBoxO4W->setValue(settingIOCtrlOutput4WidthIn5nsDefault);
	ui->spinBoxO5W->setValue(settingIOCtrlOutput5WidthIn5nsDefault);
	ui->spinBoxO6W->setValue(settingIOCtrlOutput6WidthIn5nsDefault);
	ui->spinBoxO7W->setValue(settingIOCtrlOutput7WidthIn5nsDefault);
	ui->doubleSpinBoxT0->setValue(settingIOCtrlT0PeriodIn10nsDefault);
	return;
}

void DialogIoctrl::on_spinBoxBoard_valueChanged()
{
	uint32_t drvno = ui->spinBoxBoard->value();
	settings.beginGroup("board" + QString::number(drvno));
	//save old values
	outputDelay_old[0] = settings.value(settingIOCtrlOutput1DelayIn5nsPath, settingIOCtrlOutput1DelayIn5nsDefault).toDouble();
	outputWidth_old[0] = settings.value(settingIOCtrlOutput1WidthIn5nsPath, settingIOCtrlOutput1WidthIn5nsDefault).toDouble();
	outputDelay_old[1] = settings.value(settingIOCtrlOutput2DelayIn5nsPath, settingIOCtrlOutput2DelayIn5nsDefault).toDouble();
	outputWidth_old[1] = settings.value(settingIOCtrlOutput2WidthIn5nsPath, settingIOCtrlOutput2WidthIn5nsDefault).toDouble();
	outputDelay_old[2] = settings.value(settingIOCtrlOutput3DelayIn5nsPath, settingIOCtrlOutput3DelayIn5nsDefault).toDouble();
	outputWidth_old[2] = settings.value(settingIOCtrlOutput3WidthIn5nsPath, settingIOCtrlOutput3WidthIn5nsDefault).toDouble();
	outputDelay_old[3] = settings.value(settingIOCtrlOutput4DelayIn5nsPath, settingIOCtrlOutput4DelayIn5nsDefault).toDouble();
	outputWidth_old[3] = settings.value(settingIOCtrlOutput4WidthIn5nsPath, settingIOCtrlOutput4WidthIn5nsDefault).toDouble();
	outputDelay_old[4] = settings.value(settingIOCtrlOutput5DelayIn5nsPath, settingIOCtrlOutput5DelayIn5nsDefault).toDouble();
	outputWidth_old[4] = settings.value(settingIOCtrlOutput5WidthIn5nsPath, settingIOCtrlOutput5WidthIn5nsDefault).toDouble();
	outputDelay_old[5] = settings.value(settingIOCtrlOutput6DelayIn5nsPath, settingIOCtrlOutput6DelayIn5nsDefault).toDouble();
	outputWidth_old[5] = settings.value(settingIOCtrlOutput6WidthIn5nsPath, settingIOCtrlOutput6WidthIn5nsDefault).toDouble();
	outputDelay_old[6] = settings.value(settingIOCtrlOutput7DelayIn5nsPath, settingIOCtrlOutput7DelayIn5nsDefault).toDouble();
	outputWidth_old[6] = settings.value(settingIOCtrlOutput7WidthIn5nsPath, settingIOCtrlOutput7WidthIn5nsDefault).toDouble();
	t0_old = settings.value(settingIOCtrlT0PeriodIn10nsPath, settingIOCtrlT0PeriodIn10nsDefault).toDouble();
	settings.endGroup();
	// Write the old values to UI
	ui->spinBoxO1D->setValue(outputDelay_old[0]);
	ui->spinBoxO2D->setValue(outputDelay_old[1]);
	ui->spinBoxO3D->setValue(outputDelay_old[2]);
	ui->spinBoxO4D->setValue(outputDelay_old[3]);
	ui->spinBoxO5D->setValue(outputDelay_old[4]);
	ui->spinBoxO6D->setValue(outputDelay_old[5]);
	ui->spinBoxO7D->setValue(outputDelay_old[6]);
	ui->spinBoxO1W->setValue(outputWidth_old[0]);
	ui->spinBoxO2W->setValue(outputWidth_old[1]);
	ui->spinBoxO3W->setValue(outputWidth_old[2]);
	ui->spinBoxO4W->setValue(outputWidth_old[3]);
	ui->spinBoxO5W->setValue(outputWidth_old[4]);
	ui->spinBoxO6W->setValue(outputWidth_old[5]);
	ui->spinBoxO7W->setValue(outputWidth_old[6]);
	ui->doubleSpinBoxT0->setValue(t0_old);
	return;
}
