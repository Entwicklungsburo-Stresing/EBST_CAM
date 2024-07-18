#include "dialogdac.h"
#include "ui_dialogdac.h"

DialogDac::DialogDac(QWidget* parent)
	: QDialog(parent),
	ui(new Ui::DialogDac)
{
	ui->setupUi(this);

	// Connect all spin boxes (valueChanged) to the same slot
	connect(ui->spinBoxChannel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxChannel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannel_valueChanged);
	connect(ui->spinBoxPcie, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::loadSettings);
	connect(ui->comboBoxLocation, qOverload<int>(&QComboBox::currentIndexChanged), this, &DialogDac::loadSettings);
	connect(ui->spinBoxCamera, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::loadSettings);
	connect(&mainWindow->lsc, &Lsc::measureStart, this, &DialogDac::on_autotuneStateChanged);
	connect(&mainWindow->lsc, &Lsc::measureDone, this, &DialogDac::on_autotuneStateChanged);
	connect(&mainWindow->lsc, &Lsc::measureDone, this, &DialogDac::checkTargetReached);
	
	ui->spinBoxPcie->setMaximum(number_of_boards - 1);
	if (number_of_boards == 1)
	{
		ui->spinBoxPcie->setVisible(false);
		ui->labelPcie->setVisible(false);
	}
	loadSettings();
}

void DialogDac::spinBoxChannel_valueChanged()
{
	uint32_t output[8] =
	{
		static_cast<uint32_t>(ui->spinBoxChannel1->value()),
		static_cast<uint32_t>(ui->spinBoxChannel2->value()),
		static_cast<uint32_t>(ui->spinBoxChannel3->value()),
		static_cast<uint32_t>(ui->spinBoxChannel4->value()),
		static_cast<uint32_t>(ui->spinBoxChannel5->value()),
		static_cast<uint32_t>(ui->spinBoxChannel6->value()),
		static_cast<uint32_t>(ui->spinBoxChannel7->value()),
		static_cast<uint32_t>(ui->spinBoxChannel8->value())
	};
	settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
	int sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toInt();
	settings.endGroup();
	bool is_hs_ir = false;
	if (sensor_type == sensor_type_hsir)
		is_hs_ir = true;
	bool reorder;
	if (ui->comboBoxLocation->currentIndex() == DAC8568_camera && !is_hs_ir)
		reorder = true;
	else
		reorder = false;
	mainWindow->lsc.dac_setAllOutputs(ui->spinBoxPcie->value(), ui->comboBoxLocation->currentIndex(), ui->spinBoxCamera->value(), output, reorder);

	// Save output values
	settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
	if (ui->comboBoxLocation->currentIndex() == DAC8568_camera)
	{
		settings.setValue(settingDacCameraChannel1Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel1->value());
		settings.setValue(settingDacCameraChannel2Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel2->value());
		settings.setValue(settingDacCameraChannel3Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel3->value());
		settings.setValue(settingDacCameraChannel4Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel4->value());
		settings.setValue(settingDacCameraChannel5Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel5->value());
		settings.setValue(settingDacCameraChannel6Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel6->value());
		settings.setValue(settingDacCameraChannel7Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel7->value());
		settings.setValue(settingDacCameraChannel8Path + QString::number(ui->spinBoxCamera->value()), ui->spinBoxChannel8->value());
	}
	else
	{
		settings.setValue(settingDacPcieChannel1Path, ui->spinBoxChannel1->value());
		settings.setValue(settingDacPcieChannel2Path, ui->spinBoxChannel2->value());
		settings.setValue(settingDacPcieChannel3Path, ui->spinBoxChannel3->value());
		settings.setValue(settingDacPcieChannel4Path, ui->spinBoxChannel4->value());
		settings.setValue(settingDacPcieChannel5Path, ui->spinBoxChannel5->value());
		settings.setValue(settingDacPcieChannel6Path, ui->spinBoxChannel6->value());
		settings.setValue(settingDacPcieChannel7Path, ui->spinBoxChannel7->value());
		settings.setValue(settingDacPcieChannel8Path, ui->spinBoxChannel8->value());
	}
	settings.endGroup();
	return;
}

DialogDac::~DialogDac()
{
}

void DialogDac::on_pushButtonDefault_pressed()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
	if (ui->comboBoxLocation->currentIndex() == DAC8568_camera)
	{
		//Blocking signals for spin box 1 to 7
		bool blockStateSpinBoxChannel1 = ui->spinBoxChannel1->blockSignals(true);
		bool blockStateSpinBoxChannel2 = ui->spinBoxChannel2->blockSignals(true);
		bool blockStateSpinBoxChannel3 = ui->spinBoxChannel3->blockSignals(true);
		bool blockStateSpinBoxChannel4 = ui->spinBoxChannel4->blockSignals(true);
		bool blockStateSpinBoxChannel5 = ui->spinBoxChannel5->blockSignals(true);
		bool blockStateSpinBoxChannel6 = ui->spinBoxChannel6->blockSignals(true);
		bool blockStateSpinBoxChannel7 = ui->spinBoxChannel7->blockSignals(true);

		ui->spinBoxChannel1->setValue(settingDacCameraDefault);
		ui->spinBoxChannel2->setValue(settingDacCameraDefault);
		ui->spinBoxChannel3->setValue(settingDacCameraDefault);
		ui->spinBoxChannel4->setValue(settingDacCameraDefault);
		ui->spinBoxChannel5->setValue(settingDacCameraDefault);
		ui->spinBoxChannel6->setValue(settingDacCameraDefault);
		ui->spinBoxChannel7->setValue(settingDacCameraDefault);
		ui->spinBoxChannel8->setValue(settingDacCameraDefault);

		//Unblocking signals for spin box 1 to 7
		ui->spinBoxChannel1->blockSignals(blockStateSpinBoxChannel1);
		ui->spinBoxChannel2->blockSignals(blockStateSpinBoxChannel2);
		ui->spinBoxChannel3->blockSignals(blockStateSpinBoxChannel3);
		ui->spinBoxChannel4->blockSignals(blockStateSpinBoxChannel4);
		ui->spinBoxChannel5->blockSignals(blockStateSpinBoxChannel5);
		ui->spinBoxChannel6->blockSignals(blockStateSpinBoxChannel6);
		ui->spinBoxChannel7->blockSignals(blockStateSpinBoxChannel7);
	}
	else
	{
		//Blocking signals for spin box 1 to 7
		bool blockStateSpinBoxChannel1 = ui->spinBoxChannel1->blockSignals(true);
		bool blockStateSpinBoxChannel2 = ui->spinBoxChannel2->blockSignals(true);
		bool blockStateSpinBoxChannel3 = ui->spinBoxChannel3->blockSignals(true);
		bool blockStateSpinBoxChannel4 = ui->spinBoxChannel4->blockSignals(true);
		bool blockStateSpinBoxChannel5 = ui->spinBoxChannel5->blockSignals(true);
		bool blockStateSpinBoxChannel6 = ui->spinBoxChannel6->blockSignals(true);
		bool blockStateSpinBoxChannel7 = ui->spinBoxChannel7->blockSignals(true);

		ui->spinBoxChannel1->setValue(settingDacPcieDefault);
		ui->spinBoxChannel2->setValue(settingDacPcieDefault);
		ui->spinBoxChannel3->setValue(settingDacPcieDefault);
		ui->spinBoxChannel4->setValue(settingDacPcieDefault);
		ui->spinBoxChannel5->setValue(settingDacPcieDefault);
		ui->spinBoxChannel6->setValue(settingDacPcieDefault);
		ui->spinBoxChannel7->setValue(settingDacPcieDefault);
		ui->spinBoxChannel8->setValue(settingDacPcieDefault);

		//Unblocking signals for spin box 1 to 7
		ui->spinBoxChannel1->blockSignals(blockStateSpinBoxChannel1);
		ui->spinBoxChannel2->blockSignals(blockStateSpinBoxChannel2);
		ui->spinBoxChannel3->blockSignals(blockStateSpinBoxChannel3);
		ui->spinBoxChannel4->blockSignals(blockStateSpinBoxChannel4);
		ui->spinBoxChannel5->blockSignals(blockStateSpinBoxChannel5);
		ui->spinBoxChannel6->blockSignals(blockStateSpinBoxChannel6);
		ui->spinBoxChannel7->blockSignals(blockStateSpinBoxChannel7);
	}
	settings.endGroup();
	spinBoxChannel_valueChanged();
	return;
}

void DialogDac::loadSettings()
{
	int ch8Value = ui->spinBoxChannel8->value();
	if (ui->comboBoxLocation->currentIndex() == DAC8568_camera)
	{
		settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
		//Blocking signals for spin box 1 to 7
		bool blockStateSpinBoxChannel1 = ui->spinBoxChannel1->blockSignals(true);
		bool blockStateSpinBoxChannel2 = ui->spinBoxChannel2->blockSignals(true);
		bool blockStateSpinBoxChannel3 = ui->spinBoxChannel3->blockSignals(true);
		bool blockStateSpinBoxChannel4 = ui->spinBoxChannel4->blockSignals(true);
		bool blockStateSpinBoxChannel5 = ui->spinBoxChannel5->blockSignals(true);
		bool blockStateSpinBoxChannel6 = ui->spinBoxChannel6->blockSignals(true);
		bool blockStateSpinBoxChannel7 = ui->spinBoxChannel7->blockSignals(true);

		ui->spinBoxChannel1->setValue(settings.value(settingDacCameraChannel1Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ui->spinBoxChannel2->setValue(settings.value(settingDacCameraChannel2Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ui->spinBoxChannel3->setValue(settings.value(settingDacCameraChannel3Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ui->spinBoxChannel4->setValue(settings.value(settingDacCameraChannel4Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ui->spinBoxChannel5->setValue(settings.value(settingDacCameraChannel5Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ui->spinBoxChannel6->setValue(settings.value(settingDacCameraChannel6Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ui->spinBoxChannel7->setValue(settings.value(settingDacCameraChannel7Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		//ui->spinBoxChannel8->setValue(settings.value(settingDacCameraChannel8Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble());
		ch8Value = settings.value(settingDacCameraChannel8Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toDouble();

		//Unblocking signals for spin box 1 to 7
		ui->spinBoxChannel1->blockSignals(blockStateSpinBoxChannel1);
		ui->spinBoxChannel2->blockSignals(blockStateSpinBoxChannel2);
		ui->spinBoxChannel3->blockSignals(blockStateSpinBoxChannel3);
		ui->spinBoxChannel4->blockSignals(blockStateSpinBoxChannel4);
		ui->spinBoxChannel5->blockSignals(blockStateSpinBoxChannel5);
		ui->spinBoxChannel6->blockSignals(blockStateSpinBoxChannel6);
		ui->spinBoxChannel7->blockSignals(blockStateSpinBoxChannel7);
		settings.endGroup();
	}
	else
	{
		settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
		//Blocking signals for spin box 1 to 7
		bool blockStateSpinBoxChannel1 = ui->spinBoxChannel1->blockSignals(true);
		bool blockStateSpinBoxChannel2 = ui->spinBoxChannel2->blockSignals(true);
		bool blockStateSpinBoxChannel3 = ui->spinBoxChannel3->blockSignals(true);
		bool blockStateSpinBoxChannel4 = ui->spinBoxChannel4->blockSignals(true);
		bool blockStateSpinBoxChannel5 = ui->spinBoxChannel5->blockSignals(true);
		bool blockStateSpinBoxChannel6 = ui->spinBoxChannel6->blockSignals(true);
		bool blockStateSpinBoxChannel7 = ui->spinBoxChannel7->blockSignals(true);

		ui->spinBoxChannel1->setValue(settings.value(settingDacPcieChannel1Path, settingDacPcieDefault).toDouble());
		ui->spinBoxChannel2->setValue(settings.value(settingDacPcieChannel2Path, settingDacPcieDefault).toDouble());
		ui->spinBoxChannel3->setValue(settings.value(settingDacPcieChannel3Path, settingDacPcieDefault).toDouble());
		ui->spinBoxChannel4->setValue(settings.value(settingDacPcieChannel4Path, settingDacPcieDefault).toDouble());
		ui->spinBoxChannel5->setValue(settings.value(settingDacPcieChannel5Path, settingDacPcieDefault).toDouble());
		ui->spinBoxChannel6->setValue(settings.value(settingDacPcieChannel6Path, settingDacPcieDefault).toDouble());
		ui->spinBoxChannel7->setValue(settings.value(settingDacPcieChannel7Path, settingDacPcieDefault).toDouble());
		//ui->spinBoxChannel8->setValue(settings.value(settingDacPcieChannel8Path, settingDacPcieDefault).toDouble());
		ch8Value = settings.value(settingDacPcieChannel8Path, settingDacPcieDefault).toDouble();

		//Unblocking signals for spin box 1 to 7
		ui->spinBoxChannel1->blockSignals(blockStateSpinBoxChannel1);
		ui->spinBoxChannel2->blockSignals(blockStateSpinBoxChannel2);
		ui->spinBoxChannel3->blockSignals(blockStateSpinBoxChannel3);
		ui->spinBoxChannel4->blockSignals(blockStateSpinBoxChannel4);
		ui->spinBoxChannel5->blockSignals(blockStateSpinBoxChannel5);
		ui->spinBoxChannel6->blockSignals(blockStateSpinBoxChannel6);
		ui->spinBoxChannel7->blockSignals(blockStateSpinBoxChannel7);
		settings.endGroup();
	}

	ui->spinBoxChannel8->setValue(ch8Value);

	settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
	int camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
	if (camcnt == 0)
		camcnt = 1;
	ui->spinBoxCamera->setMaximum(camcnt - 1);
	if (camcnt == 1)
	{
		ui->spinBoxCamera->setVisible(false);
		ui->labelCamera->setVisible(false);
	}
	settings.endGroup();
	return;
}

void DialogDac::on_pushButtonAutotune_pressed()
{
	if (autotuneRunning) {
		mainWindow->lsc.abortMeasurement();
		autotuneRunning = false;
		on_autotuneStateChanged();
	}
	else
	{
		allTargetsReached = false;
		ch1TargetReached = false;
		ch2TargetReached = false;
		ch3TargetReached = false;
		ch4TargetReached = false;
		ch5TargetReached = false;
		ch6TargetReached = false;
		ch7TargetReached = false;
		ch8TargetReached = false;
		autotuneRunning = true;
		autotunePressed();
	}
	return;
}

/**
 * @brief Starts the measurement when autotune is pressed
 */
void DialogDac::autotunePressed()
{
	mainWindow->startPressed();
	return;
}

/**
 * @brief Checks if the target is reached. If not adjust the DAC values
 * and call autotunePressed() again
 * */
void DialogDac::checkTargetReached()
{
	QSpinBox* spinBoxArray[8] = { ui->spinBoxChannel1, ui->spinBoxChannel2, ui->spinBoxChannel3, ui->spinBoxChannel4, ui->spinBoxChannel5, ui->spinBoxChannel6, ui->spinBoxChannel7, ui->spinBoxChannel8 };
	int spinBoxArraySize = (sizeof(spinBoxArray) / sizeof(spinBoxArray[0]));

	if (autotuneRunning && !allTargetsReached)
	{
		//define variables used to return data
		uint32_t drvno = QString::number(ui->spinBoxPcie->value()).toInt();
		settings.beginGroup("board" + QString::number(drvno));
		uint32_t sample = 20;
		uint32_t block = settings.value(settingNobPath, settingNobDefault).toDouble() - 1;
		uint16_t camera = QString::number(ui->spinBoxCamera->value()).toInt();
		uint32_t pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
		int sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toInt();
		settings.endGroup();
		size_t data_array_size = 0;
		data_array_size += pixel;
		uint16_t* data = static_cast<uint16_t*>(malloc(data_array_size * sizeof(uint16_t)));

		//return data
		es_status_codes status = mainWindow->lsc.returnFrame(drvno, sample, block, camera, pixel, data);
		if (status != es_no_error)
		{
			autotuneRunning = false;
			on_autotuneStateChanged();
			free(data);
			return;
		}

		//check if DAC value is high
		bool warningNeeded = false;
		for (int i = 0; i < spinBoxArraySize; i++)
		{
			if (spinBoxArray[i]->value() >= 60000)
				warningNeeded = true;
		}
		if (warningNeeded)
			ui->labelAutotuneWarning->setText("Warning: DAC value is high!");
		else
			ui->labelAutotuneWarning->setText("");

		//Adjust DAC value
		switch (sensor_type)
		{
		case sensor_type_hsvis:
			if (!ch1TargetReached) ch1TargetReached = autotuneAdjust(data, autotune_hsvis_ch1_start, autotune_hsvis_ch1_end, spinBoxArray[0], false);
			if (!ch2TargetReached) ch2TargetReached = autotuneAdjust(data, autotune_hsvis_ch2_start, autotune_hsvis_ch2_end, spinBoxArray[1], false);
			if (!ch3TargetReached) ch3TargetReached = autotuneAdjust(data, autotune_hsvis_ch3_start, autotune_hsvis_ch3_end, spinBoxArray[2], false);
			if (!ch4TargetReached) ch4TargetReached = autotuneAdjust(data, autotune_hsvis_ch4_start, autotune_hsvis_ch4_end, spinBoxArray[3], false);
			if (!ch5TargetReached) ch5TargetReached = autotuneAdjust(data, autotune_hsvis_ch5_start, autotune_hsvis_ch5_end, spinBoxArray[4], false);
			if (!ch6TargetReached) ch6TargetReached = autotuneAdjust(data, autotune_hsvis_ch6_start, autotune_hsvis_ch6_end, spinBoxArray[5], false);
			if (!ch7TargetReached) ch7TargetReached = autotuneAdjust(data, autotune_hsvis_ch7_start, autotune_hsvis_ch7_end, spinBoxArray[6], false);
			if (!ch8TargetReached) ch8TargetReached = autotuneAdjust(data, autotune_hsvis_ch8_start, autotune_hsvis_ch8_end, spinBoxArray[7], false);
			break;
		case sensor_type_hsir:
			if (!ch1TargetReached) ch1TargetReached = autotuneAdjust(data, autotune_hsir_ch1_start, autotune_hsir_ch1_end, spinBoxArray[0], true);
			if (!ch2TargetReached) ch2TargetReached = autotuneAdjust(data, autotune_hsir_ch2_start, autotune_hsir_ch2_end, spinBoxArray[1], true);
			if (!ch3TargetReached) ch3TargetReached = autotuneAdjust(data, autotune_hsir_ch3_start, autotune_hsir_ch3_end, spinBoxArray[2], true);
			if (!ch4TargetReached) ch4TargetReached = autotuneAdjust(data, autotune_hsir_ch4_start, autotune_hsir_ch4_end, spinBoxArray[3], true);
			if (!ch5TargetReached) ch5TargetReached = autotuneAdjust(data, autotune_hsir_ch5_start, autotune_hsir_ch5_end, spinBoxArray[4], true);
			if (!ch6TargetReached) ch6TargetReached = autotuneAdjust(data, autotune_hsir_ch6_start, autotune_hsir_ch6_end, spinBoxArray[5], true);
			if (!ch7TargetReached) ch7TargetReached = autotuneAdjust(data, autotune_hsir_ch7_start, autotune_hsir_ch7_end, spinBoxArray[6], true);
			if (!ch8TargetReached) ch8TargetReached = autotuneAdjust(data, autotune_hsir_ch8_start, autotune_hsir_ch8_end, spinBoxArray[7], true);
			break;
		default:
			break;
		}
		free(data);

		// Check if all targets are reached. If not call autotunePressed() again
		if (ch1TargetReached && ch2TargetReached && ch3TargetReached && ch4TargetReached && ch5TargetReached && ch6TargetReached && ch7TargetReached && ch8TargetReached)
		{
			allTargetsReached = true;
			autotuneRunning = false;
			on_autotuneStateChanged();
		}
		else
			autotunePressed();
	}
	return;
}

double DialogDac::calculateMean(uint16_t* data, int start, int end, bool isHsIr)
{
	double mean = 0;
	int skip = 0;
	if (isHsIr)
		skip = 1;
	for (int i = start; i < end; i = i + 1 + skip)
	{
		mean += data[i];
	}
	if(isHsIr)
		mean /= (end - start) / 2;
	else
		mean /= (end - start);
	return mean;
}

bool DialogDac::autotuneAdjust(uint16_t* data, int start, int end, QSpinBox* spinBox, bool isHsIr)
{
	bool targetReached = false;
	int target = ui->spinBoxTarget->value();
	int tolerance = 3;
	double mean = calculateMean(data, start, end, isHsIr);

	if (((mean <= target + tolerance) && (mean >= target - tolerance)) || spinBox->value() >= 65000)
		targetReached = true;
	else {
		double distance = 0;
		if (target < mean)
		{
			distance = mean - target;
			spinBox->setValue(spinBox->value() - distance / 2);
		}
		else if (target > mean)
		{
			distance = target - mean;
			spinBox->setValue(spinBox->value() + distance / 2);
		}
	}
	return targetReached;
}

/**
 * @brief Changes the state of the buttons, when autotune is used
 * */
void DialogDac::on_autotuneStateChanged()
{
	if (!autotuneRunning)
	{
		ui->pushButtonAutotune->setText("Autotune");
		mainWindow->ui->checkBoxLoopMeasurement->setEnabled(true);
		mainWindow->ui->pushButtonStartStop->setEnabled(true);
	}
	else
	{
		QSpinBox* spinBoxArray[8] = { ui->spinBoxChannel1, ui->spinBoxChannel2, ui->spinBoxChannel3, ui->spinBoxChannel4, ui->spinBoxChannel5, ui->spinBoxChannel6, ui->spinBoxChannel7, ui->spinBoxChannel8 };
		int spinBoxArraySize = (sizeof(spinBoxArray) / sizeof(spinBoxArray[0]));
		for (int i = 0; i < spinBoxArraySize; i++)
		{
			if (spinBoxArray[i]->value() >= 65000)
				spinBoxArray[i]->setValue(64999);
		}
		ui->pushButtonAutotune->setText("Abort");
		if (mainWindow->ui->checkBoxLoopMeasurement->isChecked())
			mainWindow->ui->checkBoxLoopMeasurement->setChecked(false);
		mainWindow->ui->checkBoxLoopMeasurement->setEnabled(false);
		mainWindow->ui->pushButtonStartStop->setEnabled(false);
	}
	return;
}

void DialogDac::reject()
{
	if (autotuneRunning)
	{
		QErrorMessage* m = new QErrorMessage(this);
		m->setWindowTitle("Error");
		m->showMessage("Stop autotune before closing");
	}
	else
		QDialog::reject();
	return;
}
