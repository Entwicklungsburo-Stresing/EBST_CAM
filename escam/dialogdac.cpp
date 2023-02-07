#include "dialogdac.h"
#include "ui_dialogdac.h"

DialogDac::DialogDac(QWidget* parent)
	: QDialog(parent),
	ui(new Ui::DialogDac)
{
	ui->setupUi(this);
	// Save old values
	output_old[0][0] = settings.value(settingDacCameraChannel1Path, settingDacCameraDefault).toUInt();
	output_old[0][1] = settings.value(settingDacCameraChannel2Path, settingDacCameraDefault).toUInt();
	output_old[0][2] = settings.value(settingDacCameraChannel3Path, settingDacCameraDefault).toUInt();
	output_old[0][3] = settings.value(settingDacCameraChannel4Path, settingDacCameraDefault).toUInt();
	output_old[0][4] = settings.value(settingDacCameraChannel5Path, settingDacCameraDefault).toUInt();
	output_old[0][5] = settings.value(settingDacCameraChannel6Path, settingDacCameraDefault).toUInt();
	output_old[0][6] = settings.value(settingDacCameraChannel7Path, settingDacCameraDefault).toUInt();
	output_old[0][7] = settings.value(settingDacCameraChannel8Path, settingDacCameraDefault).toUInt();
	output_old[2][0] = settings.value(settingDacPcieChannel1Path, settingDacPcieDefault).toUInt();
	output_old[2][1] = settings.value(settingDacPcieChannel2Path, settingDacPcieDefault).toUInt();
	output_old[2][2] = settings.value(settingDacPcieChannel3Path, settingDacPcieDefault).toUInt();
	output_old[2][3] = settings.value(settingDacPcieChannel4Path, settingDacPcieDefault).toUInt();
	output_old[2][4] = settings.value(settingDacPcieChannel5Path, settingDacPcieDefault).toUInt();
	output_old[2][5] = settings.value(settingDacPcieChannel6Path, settingDacPcieDefault).toUInt();
	output_old[2][6] = settings.value(settingDacPcieChannel7Path, settingDacPcieDefault).toUInt();
	// Write the old values to UI
	ui->spinBoxChannel1->setValue(static_cast<int>(output_old[0][0]));
	ui->spinBoxChannel2->setValue(static_cast<int>(output_old[0][1]));
	ui->spinBoxChannel3->setValue(static_cast<int>(output_old[0][2]));
	ui->spinBoxChannel4->setValue(static_cast<int>(output_old[0][3]));
	ui->spinBoxChannel5->setValue(static_cast<int>(output_old[0][4]));
	ui->spinBoxChannel6->setValue(static_cast<int>(output_old[0][5]));
	ui->spinBoxChannel7->setValue(static_cast<int>(output_old[0][6]));
	ui->spinBoxChannel8->setValue(static_cast<int>(output_old[0][7]));
	ui->spinBoxBoard2Channel1->setValue(static_cast<int>(output_old[1][0]));
	ui->spinBoxBoard2Channel2->setValue(static_cast<int>(output_old[1][1]));
	ui->spinBoxBoard2Channel3->setValue(static_cast<int>(output_old[1][2]));
	ui->spinBoxBoard2Channel4->setValue(static_cast<int>(output_old[1][3]));
	ui->spinBoxBoard2Channel5->setValue(static_cast<int>(output_old[1][4]));
	ui->spinBoxBoard2Channel6->setValue(static_cast<int>(output_old[1][5]));
	ui->spinBoxBoard2Channel7->setValue(static_cast<int>(output_old[1][6]));
	ui->spinBoxBoard2Channel8->setValue(static_cast<int>(output_old[1][7]));
	ui->spinBoxPCIeBoard1Channel1->setValue(static_cast<int>(output_old[2][0]));
	ui->spinBoxPCIeBoard1Channel2->setValue(static_cast<int>(output_old[2][1]));
	ui->spinBoxPCIeBoard1Channel3->setValue(static_cast<int>(output_old[2][2]));
	ui->spinBoxPCIeBoard1Channel4->setValue(static_cast<int>(output_old[2][3]));
	ui->spinBoxPCIeBoard1Channel5->setValue(static_cast<int>(output_old[2][4]));
	ui->spinBoxPCIeBoard1Channel6->setValue(static_cast<int>(output_old[2][5]));
	ui->spinBoxPCIeBoard1Channel7->setValue(static_cast<int>(output_old[2][6]));
	ui->spinBoxPCIeBoard1Channel8->setValue(static_cast<int>(output_old[2][7]));
	ui->spinBoxPCIeBoard2Channel1->setValue(static_cast<int>(output_old[3][0]));
	ui->spinBoxPCIeBoard2Channel2->setValue(static_cast<int>(output_old[3][1]));
	ui->spinBoxPCIeBoard2Channel3->setValue(static_cast<int>(output_old[3][2]));
	ui->spinBoxPCIeBoard2Channel4->setValue(static_cast<int>(output_old[3][3]));
	ui->spinBoxPCIeBoard2Channel5->setValue(static_cast<int>(output_old[3][4]));
	ui->spinBoxPCIeBoard2Channel6->setValue(static_cast<int>(output_old[3][5]));
	ui->spinBoxPCIeBoard2Channel7->setValue(static_cast<int>(output_old[3][6]));
	ui->spinBoxPCIeBoard2Channel8->setValue(static_cast<int>(output_old[3][7]));
	// Connect all spin boxes (valueChanged) to the same slot
	connect(ui->spinBoxChannel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxChannel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxCameraChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard1Channel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);
	connect(ui->spinBoxPCIeBoard2Channel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxPcieBoardChannelX_valueChanged);

	// cosmetics depending on number_of_boards and board_sel
	if (number_of_boards == 1)
	{
		ui->labelBoard2->setVisible(false);
		ui->spinBoxBoard2Channel1->setVisible(false);
		ui->spinBoxBoard2Channel2->setVisible(false);
		ui->spinBoxBoard2Channel3->setVisible(false);
		ui->spinBoxBoard2Channel4->setVisible(false);
		ui->spinBoxBoard2Channel5->setVisible(false);
		ui->spinBoxBoard2Channel6->setVisible(false);
		ui->spinBoxBoard2Channel7->setVisible(false);
		ui->spinBoxBoard2Channel8->setVisible(false);
		ui->labelPcieBoard2->setVisible(false);
		ui->spinBoxPCIeBoard2Channel1->setVisible(false);
		ui->spinBoxPCIeBoard2Channel2->setVisible(false);
		ui->spinBoxPCIeBoard2Channel3->setVisible(false);
		ui->spinBoxPCIeBoard2Channel4->setVisible(false);
		ui->spinBoxPCIeBoard2Channel5->setVisible(false);
		ui->spinBoxPCIeBoard2Channel6->setVisible(false);
		ui->spinBoxPCIeBoard2Channel7->setVisible(false);
		ui->spinBoxPCIeBoard2Channel8->setVisible(false);
	}
	else
	{
		int board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt();
		switch (board_sel)
		{
		default:
		case 1:
			ui->spinBoxBoard2Channel1->setEnabled(false);
			ui->spinBoxBoard2Channel2->setEnabled(false);
			ui->spinBoxBoard2Channel3->setEnabled(false);
			ui->spinBoxBoard2Channel4->setEnabled(false);
			ui->spinBoxBoard2Channel5->setEnabled(false);
			ui->spinBoxBoard2Channel6->setEnabled(false);
			ui->spinBoxBoard2Channel7->setEnabled(false);
			ui->spinBoxBoard2Channel8->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel1->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel2->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel3->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel4->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel5->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel6->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel7->setEnabled(false);
			ui->spinBoxPCIeBoard2Channel8->setEnabled(false);
			break;
		case 2:
			ui->spinBoxChannel1->setEnabled(false);
			ui->spinBoxChannel2->setEnabled(false);
			ui->spinBoxChannel3->setEnabled(false);
			ui->spinBoxChannel4->setEnabled(false);
			ui->spinBoxChannel5->setEnabled(false);
			ui->spinBoxChannel6->setEnabled(false);
			ui->spinBoxChannel7->setEnabled(false);
			ui->spinBoxChannel8->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel1->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel2->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel3->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel4->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel5->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel6->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel7->setEnabled(false);
			ui->spinBoxPCIeBoard1Channel8->setEnabled(false);
			break;
		case 3:
			break;
		}
	}
}

DialogDac::~DialogDac()
{
}

void DialogDac::spinBoxCameraChannelX_valueChanged()
{
	uint32_t output[2][8] =
	{
		{
			static_cast<uint32_t>(ui->spinBoxChannel1->value()),
			static_cast<uint32_t>(ui->spinBoxChannel2->value()),
			static_cast<uint32_t>(ui->spinBoxChannel3->value()),
			static_cast<uint32_t>(ui->spinBoxChannel4->value()),
			static_cast<uint32_t>(ui->spinBoxChannel5->value()),
			static_cast<uint32_t>(ui->spinBoxChannel6->value()),
			static_cast<uint32_t>(ui->spinBoxChannel7->value()),
			static_cast<uint32_t>(ui->spinBoxChannel8->value())
		},
		{
			static_cast<uint32_t>(ui->spinBoxBoard2Channel1->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel2->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel3->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel4->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel5->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel6->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel7->value()),
			static_cast<uint32_t>(ui->spinBoxBoard2Channel8->value())
		}
	};
	bool is_hs_ir = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		mainWindow->lsc.dac_setAllOutputs(drvno, DAC8568_camera, output[drvno], !is_hs_ir);
	return;
}

void DialogDac::spinBoxPcieBoardChannelX_valueChanged()
{
	uint32_t output[2][8] =
	{
		{
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel1->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel2->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel3->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel4->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel5->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel6->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel7->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard1Channel8->value())
		},
		{
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel1->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel2->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel3->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel4->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel5->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel6->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel7->value()),
			static_cast<uint32_t>(ui->spinBoxPCIeBoard2Channel8->value())
		}
	};
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		mainWindow->lsc.dac_setAllOutputs(drvno, DAC8568_pcie, output[drvno], false);
	return;
}

void DialogDac::on_buttonBox_rejected()
{
	// Send old values to DAC
	bool is_hs_ir = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		mainWindow->lsc.dac_setAllOutputs(drvno, DAC8568_camera, output_old[drvno], !is_hs_ir);
		mainWindow->lsc.dac_setAllOutputs(drvno, DAC8568_pcie, output_old[2 + drvno], false);
	}
	return;
}

void DialogDac::on_buttonBox_accepted()
{
	// Save output values
	settings.setValue(settingDacCameraChannel1Path, ui->spinBoxChannel1->value());
	settings.setValue(settingDacCameraChannel2Path, ui->spinBoxChannel2->value());
	settings.setValue(settingDacCameraChannel3Path, ui->spinBoxChannel3->value());
	settings.setValue(settingDacCameraChannel4Path, ui->spinBoxChannel4->value());
	settings.setValue(settingDacCameraChannel5Path, ui->spinBoxChannel5->value());
	settings.setValue(settingDacCameraChannel6Path, ui->spinBoxChannel6->value());
	settings.setValue(settingDacCameraChannel7Path, ui->spinBoxChannel7->value());
	settings.setValue(settingDacCameraChannel8Path, ui->spinBoxChannel8->value());
	settings.setValue(settingDacPcieChannel1Path, ui->spinBoxPCIeBoard1Channel1->value());
	settings.setValue(settingDacPcieChannel2Path, ui->spinBoxPCIeBoard1Channel2->value());
	settings.setValue(settingDacPcieChannel3Path, ui->spinBoxPCIeBoard1Channel3->value());
	settings.setValue(settingDacPcieChannel4Path, ui->spinBoxPCIeBoard1Channel4->value());
	settings.setValue(settingDacPcieChannel5Path, ui->spinBoxPCIeBoard1Channel5->value());
	settings.setValue(settingDacPcieChannel6Path, ui->spinBoxPCIeBoard1Channel6->value());
	settings.setValue(settingDacPcieChannel7Path, ui->spinBoxPCIeBoard1Channel7->value());
	return;
}

void DialogDac::on_pushButtonDefault_pressed()
{
	ui->spinBoxChannel1->setValue(settingDacCameraDefault);
	ui->spinBoxChannel2->setValue(settingDacCameraDefault);
	ui->spinBoxChannel3->setValue(settingDacCameraDefault);
	ui->spinBoxChannel4->setValue(settingDacCameraDefault);
	ui->spinBoxChannel5->setValue(settingDacCameraDefault);
	ui->spinBoxChannel6->setValue(settingDacCameraDefault);
	ui->spinBoxChannel7->setValue(settingDacCameraDefault);
	ui->spinBoxChannel8->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel1->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel2->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel3->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel4->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel5->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel6->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel7->setValue(settingDacCameraDefault);
	ui->spinBoxBoard2Channel8->setValue(settingDacCameraDefault);
	ui->spinBoxPCIeBoard1Channel1->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel2->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel3->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel4->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel5->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel6->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel7->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard1Channel8->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel1->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel2->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel3->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel4->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel5->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel6->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel7->setValue(settingDacPcieDefault);
	ui->spinBoxPCIeBoard2Channel8->setValue(settingDacPcieDefault);
	return;
}
