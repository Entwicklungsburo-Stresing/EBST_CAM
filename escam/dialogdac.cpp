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
	bool is_hs_ir = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	settings.endGroup();
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

void DialogDac::loadSettings()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxPcie->value()));
	if (ui->comboBoxLocation->currentIndex() == DAC8568_camera)
	{
		ui->spinBoxChannel1->setValue(settings.value(settingDacCameraChannel1Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel2->setValue(settings.value(settingDacCameraChannel2Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel3->setValue(settings.value(settingDacCameraChannel3Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel4->setValue(settings.value(settingDacCameraChannel4Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel5->setValue(settings.value(settingDacCameraChannel5Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel6->setValue(settings.value(settingDacCameraChannel6Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel7->setValue(settings.value(settingDacCameraChannel7Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
		ui->spinBoxChannel8->setValue(settings.value(settingDacCameraChannel8Path + QString::number(ui->spinBoxCamera->value()), settingDacCameraDefault).toInt());
	}
	else
	{
		ui->spinBoxChannel1->setValue(settings.value(settingDacPcieChannel1Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel2->setValue(settings.value(settingDacPcieChannel2Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel3->setValue(settings.value(settingDacPcieChannel3Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel4->setValue(settings.value(settingDacPcieChannel4Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel5->setValue(settings.value(settingDacPcieChannel5Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel6->setValue(settings.value(settingDacPcieChannel6Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel7->setValue(settings.value(settingDacPcieChannel7Path, settingDacPcieDefault).toInt());
		ui->spinBoxChannel8->setValue(settings.value(settingDacPcieChannel8Path, settingDacPcieDefault).toInt());
	}

	int camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toInt();
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
