#include "dacspinboxes.h"

DacSpinBoxes::DacSpinBoxes(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::DacSpinBoxesClass())
{
	ui->setupUi(this);

	// Connect all spin boxes (valueChanged) to the same slot
	connect(ui->spinBoxChannel1, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel2, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel3, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel4, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel5, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel6, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel7, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);
	connect(ui->spinBoxChannel8, qOverload<int>(&QSpinBox::valueChanged), this, &DacSpinBoxes::spinBox_valueChanged);

}

void DacSpinBoxes::spinBox_valueChanged()
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
	settings.beginGroup("board" + QString::number(drvno));
	bool is_hs_ir = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	settings.endGroup();
	bool reorder;
	if (location == DAC8568_camera && !is_hs_ir)
		reorder = true;
	else
		reorder = false;
	mainWindow->lsc.dac_setAllOutputs(drvno, location, output, reorder);
	return;
}

void DacSpinBoxes::on_rejected()
{
	// Send old values to DAC
	settings.beginGroup("board" + QString::number(drvno));
	bool is_hs_ir = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	settings.endGroup();
	bool reorder;
	if (location == DAC8568_camera && !is_hs_ir)
		reorder = true;
	else
		reorder = false;
	mainWindow->lsc.dac_setAllOutputs(drvno, location, output_old, reorder);
	return;
}

void DacSpinBoxes::on_accepted()
{
	// Save output values
	settings.beginGroup("board" + QString::number(drvno));
	if (location == DAC8568_camera)
	{
		settings.setValue(settingDacCameraChannel1Path, ui->spinBoxChannel1->value());
		settings.setValue(settingDacCameraChannel2Path, ui->spinBoxChannel2->value());
		settings.setValue(settingDacCameraChannel3Path, ui->spinBoxChannel3->value());
		settings.setValue(settingDacCameraChannel4Path, ui->spinBoxChannel4->value());
		settings.setValue(settingDacCameraChannel5Path, ui->spinBoxChannel5->value());
		settings.setValue(settingDacCameraChannel6Path, ui->spinBoxChannel6->value());
		settings.setValue(settingDacCameraChannel7Path, ui->spinBoxChannel7->value());
		settings.setValue(settingDacCameraChannel8Path, ui->spinBoxChannel8->value());
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

void DacSpinBoxes::on_default_pressed()
{
	settings.beginGroup("board" + QString::number(drvno));
	if (location == DAC8568_camera)
	{
		ui->spinBoxChannel1->setValue(settingDacCameraDefault);
		ui->spinBoxChannel2->setValue(settingDacCameraDefault);
		ui->spinBoxChannel3->setValue(settingDacCameraDefault);
		ui->spinBoxChannel4->setValue(settingDacCameraDefault);
		ui->spinBoxChannel5->setValue(settingDacCameraDefault);
		ui->spinBoxChannel6->setValue(settingDacCameraDefault);
		ui->spinBoxChannel7->setValue(settingDacCameraDefault);
		ui->spinBoxChannel8->setValue(settingDacCameraDefault);
	}
	else
	{
		ui->spinBoxChannel1->setValue(settingDacPcieDefault);
		ui->spinBoxChannel2->setValue(settingDacPcieDefault);
		ui->spinBoxChannel3->setValue(settingDacPcieDefault);
		ui->spinBoxChannel4->setValue(settingDacPcieDefault);
		ui->spinBoxChannel5->setValue(settingDacPcieDefault);
		ui->spinBoxChannel6->setValue(settingDacPcieDefault);
		ui->spinBoxChannel7->setValue(settingDacPcieDefault);
		ui->spinBoxChannel8->setValue(settingDacPcieDefault);
	}
	settings.endGroup();
	return;
}

void DacSpinBoxes::initialize()
{
	// Save old values
	settings.beginGroup("board" + QString::number(drvno));
	if (location == DAC8568_camera)
	{
		output_old[0] = settings.value(settingDacCameraChannel1Path, settingDacCameraDefault).toUInt();
		output_old[1] = settings.value(settingDacCameraChannel2Path, settingDacCameraDefault).toUInt();
		output_old[2] = settings.value(settingDacCameraChannel3Path, settingDacCameraDefault).toUInt();
		output_old[3] = settings.value(settingDacCameraChannel4Path, settingDacCameraDefault).toUInt();
		output_old[4] = settings.value(settingDacCameraChannel5Path, settingDacCameraDefault).toUInt();
		output_old[5] = settings.value(settingDacCameraChannel6Path, settingDacCameraDefault).toUInt();
		output_old[6] = settings.value(settingDacCameraChannel7Path, settingDacCameraDefault).toUInt();
		output_old[7] = settings.value(settingDacCameraChannel8Path, settingDacCameraDefault).toUInt();
	}
	else
	{
		output_old[0] = settings.value(settingDacPcieChannel1Path, settingDacPcieDefault).toUInt();
		output_old[1] = settings.value(settingDacPcieChannel2Path, settingDacPcieDefault).toUInt();
		output_old[2] = settings.value(settingDacPcieChannel3Path, settingDacPcieDefault).toUInt();
		output_old[3] = settings.value(settingDacPcieChannel4Path, settingDacPcieDefault).toUInt();
		output_old[4] = settings.value(settingDacPcieChannel5Path, settingDacPcieDefault).toUInt();
		output_old[5] = settings.value(settingDacPcieChannel6Path, settingDacPcieDefault).toUInt();
		output_old[6] = settings.value(settingDacPcieChannel7Path, settingDacPcieDefault).toUInt();
		output_old[7] = settings.value(settingDacPcieChannel8Path, settingDacPcieDefault).toUInt();
	}
	settings.endGroup();
	// Write the old values to UI
	ui->spinBoxChannel1->setValue(static_cast<int>(output_old[0]));
	ui->spinBoxChannel2->setValue(static_cast<int>(output_old[1]));
	ui->spinBoxChannel3->setValue(static_cast<int>(output_old[2]));
	ui->spinBoxChannel4->setValue(static_cast<int>(output_old[3]));
	ui->spinBoxChannel5->setValue(static_cast<int>(output_old[4]));
	ui->spinBoxChannel6->setValue(static_cast<int>(output_old[5]));
	ui->spinBoxChannel7->setValue(static_cast<int>(output_old[6]));
	ui->spinBoxChannel8->setValue(static_cast<int>(output_old[7]));

	// set board label
	ui->labelBoardNr->setText("board " + QString::number(drvno));
	return;
}

DacSpinBoxes::~DacSpinBoxes()
{
	delete ui;
}
