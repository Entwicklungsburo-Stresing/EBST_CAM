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
	int target = ui->spinBoxTarget->value();
	int tolerance = 3;
	const int timeoutCount = 50;
	int timeout = timeoutCount;
	int meanChannel1 = 0, meanChannel2 = 0, meanChannel3 = 0, meanChannel4 = 0, meanChannel5 = 0, meanChannel6 = 0, meanChannel7 = 0, meanChannel8 = 0;

	int meanArray[8][timeoutCount];
	int iterator = 0;

	bool targetReached = false, ch1TargetReached = false, ch2TargetReached = false, ch3Target = false, ch3TargetReached = false, ch4TargetReached = false, ch5TargetReached = false, ch6TargetReached = false, ch7TargetReached = false, ch8TargetReached = false;
	while (!targetReached && timeout > 0)
	{
		mainWindow->initSettings();
		es_status_codes status = mainWindow->lsc.initMeasurement();
		if (status != es_no_error)
		{
			QMessageBox* msgBox = new QMessageBox(this);
			msgBox->setWindowTitle("No camera found");
			msgBox->setText("No camera found");
			msgBox->setStandardButtons(QMessageBox::Ok);
			msgBox->exec();
			return;
		}
		mainWindow->lsc.startMeasurement();
		uint32_t drvno = QString::number(ui->spinBoxPcie->value()).toInt();
		uint32_t sample = settings.value(settingNosPath, settingNosDefault).toDouble() - 1;
		uint32_t block = settings.value(settingNobPath, settingNobDefault).toDouble() - 1;
		uint16_t camera = QString::number(ui->spinBoxCamera->value()).toInt();
		uint32_t pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
		size_t data_array_size = 0;
		data_array_size += pixel;
		uint16_t* data = static_cast<uint16_t*>(malloc(data_array_size * sizeof(uint16_t)));
		//loadSettings();
		
		status = mainWindow->lsc.returnFrame(drvno, sample, block, camera, pixel, data);
		if (status != es_no_error) return;

		meanChannel1 = calculateMean(data, pixel_range_start, pixel_range_1);
		meanArray[0][iterator] = meanChannel1;

		if ((meanChannel1 <= target + tolerance) && (meanChannel1 >= target - tolerance))
			ch1TargetReached = true;
		else {
			int ch1Distance = 0;
			if (target < meanChannel1)
			{
				ch1Distance = meanChannel1 - target;
				ui->spinBoxChannel1->setValue(ui->spinBoxChannel1->value() - ch1Distance / 2);

			}
			else if (target > meanChannel1)
			{
				ch1Distance = target - meanChannel1;
				ui->spinBoxChannel1->setValue(ui->spinBoxChannel1->value() + ch1Distance / 2);
			}
			qDebug() << "Channel 1 Distance: " << ch1Distance;
			qDebug() << "Channel 1 Value: " << ui->spinBoxChannel1->value();
		}

		meanChannel2 = calculateMean(data, pixel_range_1, pixel_range_2);
		meanArray[1][iterator] = meanChannel2;

		if((meanChannel2 <= target + tolerance) && (meanChannel2 >= target - tolerance))
						ch2TargetReached = true;
		else {
			int ch2Distance = 0;
			if (target < meanChannel2)
			{
				ch2Distance = meanChannel2 - target;
				ui->spinBoxChannel2->setValue(ui->spinBoxChannel2->value() - ch2Distance / 2);
			}
			else if (target > meanChannel2)
			{
				ch2Distance = target - meanChannel2;
				ui->spinBoxChannel2->setValue(ui->spinBoxChannel2->value() + ch2Distance / 2);
			}
			qDebug() << "Channel 2 Distance: " << ch2Distance;
		}

		meanChannel3 = calculateMean(data, pixel_range_2, pixel_range_3);
		meanArray[2][iterator] = meanChannel3;
		
		if ((meanChannel3 <= target + tolerance) && (meanChannel3 >= target - tolerance))
			ch3TargetReached = true;
		else {
			int ch3Distance = 0;
			if (target < meanChannel3)
			{
				ch3Distance = meanChannel3 - target;
				ui->spinBoxChannel3->setValue(ui->spinBoxChannel3->value() - ch3Distance / 2);
			}
			else if (target > meanChannel3)
			{
				ch3Distance = target - meanChannel3;
				ui->spinBoxChannel3->setValue(ui->spinBoxChannel3->value() + ch3Distance / 2);
			}
			qDebug() << "Channel 3 Distance: " << ch3Distance;
		}

		meanChannel4 = calculateMean(data, pixel_range_3, pixel_range_4);
		meanArray[3][iterator] = meanChannel4;
		
		if ((meanChannel4 <= target + tolerance) && (meanChannel4 >= target - tolerance))
			ch4TargetReached = true;
		else {
			int ch4Distance = 0;
			if (target < meanChannel4)
			{
				ch4Distance = meanChannel4 - target;
				ui->spinBoxChannel4->setValue(ui->spinBoxChannel4->value() - ch4Distance / 2);
			}
			else if (target > meanChannel4)
			{
				ch4Distance = target - meanChannel4;
				ui->spinBoxChannel4->setValue(ui->spinBoxChannel4->value() + ch4Distance / 2);
			}
			qDebug() << "Channel 4 Distance: " << ch4Distance;
		}

		meanChannel5 = calculateMean(data, pixel_range_4, pixel_range_5);
		meanArray[4][iterator] = meanChannel5;
		
		if ((meanChannel5 <= target + tolerance) && (meanChannel5 >= target - tolerance))
			ch5TargetReached = true;
		else {
			int ch5Distance = 0;
			if (target < meanChannel5)
			{
				ch5Distance = meanChannel5 - target;
				ui->spinBoxChannel5->setValue(ui->spinBoxChannel5->value() - ch5Distance / 2);
			}
			else if (target > meanChannel5)
			{
				ch5Distance = target - meanChannel5;
				ui->spinBoxChannel5->setValue(ui->spinBoxChannel5->value() + ch5Distance / 2);
			}
			qDebug() << "Channel 5 Distance: " << ch5Distance;
		}

		meanChannel6 = calculateMean(data, pixel_range_5, pixel_range_6);
		meanArray[5][iterator] = meanChannel6;
		
		if ((meanChannel6 <= target + tolerance) && (meanChannel6 >= target - tolerance))
			ch6TargetReached = true;
		else {
			int ch6Distance = 0;
			if (target < meanChannel6)
			{
				ch6Distance = meanChannel6 - target;
				ui->spinBoxChannel6->setValue(ui->spinBoxChannel6->value() - ch6Distance / 2);
			}
			else if (target > meanChannel6)
			{
				ch6Distance = target - meanChannel6;
				ui->spinBoxChannel6->setValue(ui->spinBoxChannel6->value() + ch6Distance / 2);
			}
			qDebug() << "Channel 6 Distance: " << ch6Distance;
		}

		meanChannel7 = calculateMean(data, pixel_range_6, pixel_range_7);
		meanArray[6][iterator] = meanChannel7;
		
		if ((meanChannel7 <= target + tolerance) && (meanChannel7 >= target - tolerance))
			ch7TargetReached = true;
		else {
			int ch7Distance = 0;
			if (target < meanChannel7)
			{
				ch7Distance = meanChannel7 - target;
				ui->spinBoxChannel7->setValue(ui->spinBoxChannel7->value() - ch7Distance / 2);
			}
			else if (target > meanChannel7)
			{
				ch7Distance = target - meanChannel7;
				ui->spinBoxChannel7->setValue(ui->spinBoxChannel7->value() + ch7Distance / 2);
			}
			qDebug() << "Channel 7 Distance: " << ch7Distance;
		}

		meanChannel8 = calculateMean(data, pixel_range_7, pixel_range_8);
		meanArray[7][iterator] = meanChannel8;
		
		if ((meanChannel8 <= target + tolerance) && (meanChannel8 >= target - tolerance))
			ch8TargetReached = true;
		else {
			int ch8Distance = 0;
			if (target < meanChannel8)
			{
				ch8Distance = meanChannel8 - target;
				ui->spinBoxChannel8->setValue(ui->spinBoxChannel8->value() - ch8Distance / 2);
			}
			else if (target > meanChannel8)
			{
				ch8Distance = target - meanChannel8;
				ui->spinBoxChannel8->setValue(ui->spinBoxChannel8->value() + ch8Distance / 2);
			}
			qDebug() << "Channel 8 Distance: " << ch8Distance;
		}

		//create a delay of 300 milliseconds
		QTime dieTime = QTime::currentTime().addMSecs(300);
		while (QTime::currentTime() < dieTime)
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

		free(data);
		timeout--;
		iterator++;
		if (ch1TargetReached && ch2TargetReached && ch3TargetReached && ch4TargetReached && ch5TargetReached && ch6TargetReached && ch7TargetReached && ch8TargetReached)
			targetReached = true;
	}
	qDebug() << "================\nIterations: " << iterator << "\n================";

	for (int i = 0; i < 8; i++) {
		qDebug() << "Mean Array Channel " << i + 1 << ": " << meanArray[i];
		int distance = 0;
		for (int j = 0; j < iterator; j++)
		{
			qDebug() << "\tValue " << j << ": " << meanArray[i][j];
			if(j >= 1) distance = meanArray[i][j - 1] - meanArray[i][j];
			qDebug() << "\tDistance: " << distance;
		}
	}
	return;
}

int DialogDac::calculateMean(uint16_t* data, int start, int end)
{
	int mean = 0;
	for (int i = start; i < end; i++)
	{
		mean += data[i];
	}
	mean /= (end - start);
	return mean;
}
