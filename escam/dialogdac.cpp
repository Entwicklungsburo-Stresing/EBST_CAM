#include "dialogdac.h"
#include "ui_dialogdac.h"

DialogDac::DialogDac(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::DialogDac)
{
	ui->setupUi(this);
	// Save old values
	output_old[0][0] = settings.value(settingSensorOffsetChannel1Path, settingSensorOffsetChannel1Default).toInt();
	output_old[0][1] = settings.value(settingSensorOffsetChannel2Path, settingSensorOffsetChannel2Default).toInt();
	output_old[0][2] = settings.value(settingSensorOffsetChannel3Path, settingSensorOffsetChannel3Default).toInt();
	output_old[0][3] = settings.value(settingSensorOffsetChannel4Path, settingSensorOffsetChannel4Default).toInt();
	output_old[0][4] = settings.value(settingSensorOffsetChannel5Path, settingSensorOffsetChannel5Default).toInt();
	output_old[0][5] = settings.value(settingSensorOffsetChannel6Path, settingSensorOffsetChannel6Default).toInt();
	output_old[0][6] = settings.value(settingSensorOffsetChannel7Path, settingSensorOffsetChannel7Default).toInt();
	output_old[0][7] = settings.value(settingSensorOffsetChannel8Path, settingSensorOffsetChannel8Default).toInt();
	output_old[1][0] = settings.value(settingSensorOffsetBoard2Channel1Path, settingSensorOffsetBoard2Channel1Default).toInt();
	output_old[1][1] = settings.value(settingSensorOffsetBoard2Channel2Path, settingSensorOffsetBoard2Channel2Default).toInt();
	output_old[1][2] = settings.value(settingSensorOffsetBoard2Channel3Path, settingSensorOffsetBoard2Channel3Default).toInt();
	output_old[1][3] = settings.value(settingSensorOffsetBoard2Channel4Path, settingSensorOffsetBoard2Channel4Default).toInt();
	output_old[1][4] = settings.value(settingSensorOffsetBoard2Channel5Path, settingSensorOffsetBoard2Channel5Default).toInt();
	output_old[1][5] = settings.value(settingSensorOffsetBoard2Channel6Path, settingSensorOffsetBoard2Channel6Default).toInt();
	output_old[1][6] = settings.value(settingSensorOffsetBoard2Channel7Path, settingSensorOffsetBoard2Channel7Default).toInt();
	output_old[1][7] = settings.value(settingSensorOffsetBoard2Channel8Path, settingSensorOffsetBoard2Channel8Default).toInt();
	// Write the old values to UI
	ui->spinBoxChannel1->setValue(output_old[0][0]);
	ui->spinBoxChannel2->setValue(output_old[0][1]);
	ui->spinBoxChannel3->setValue(output_old[0][2]);
	ui->spinBoxChannel4->setValue(output_old[0][3]);
	ui->spinBoxChannel5->setValue(output_old[0][4]);
	ui->spinBoxChannel6->setValue(output_old[0][5]);
	ui->spinBoxChannel7->setValue(output_old[0][6]);
	ui->spinBoxChannel8->setValue(output_old[0][7]);
	ui->spinBoxBoard2Channel1->setValue(output_old[1][0]);
	ui->spinBoxBoard2Channel2->setValue(output_old[1][1]);
	ui->spinBoxBoard2Channel3->setValue(output_old[1][2]);
	ui->spinBoxBoard2Channel4->setValue(output_old[1][3]);
	ui->spinBoxBoard2Channel5->setValue(output_old[1][4]);
	ui->spinBoxBoard2Channel6->setValue(output_old[1][5]);
	ui->spinBoxBoard2Channel7->setValue(output_old[1][6]);
	ui->spinBoxBoard2Channel8->setValue(output_old[1][7]);
	// Connect all spin boxes (valueChanged) to the same slot
	connect(ui->spinBoxChannel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxBoard2Channel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);

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
	}
	else
	{
		int board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt();
		switch (board_sel)
		{
		default:
		case 0:
			ui->spinBoxBoard2Channel1->setEnabled(false);
			ui->spinBoxBoard2Channel2->setEnabled(false);
			ui->spinBoxBoard2Channel3->setEnabled(false);
			ui->spinBoxBoard2Channel4->setEnabled(false);
			ui->spinBoxBoard2Channel5->setEnabled(false);
			ui->spinBoxBoard2Channel6->setEnabled(false);
			ui->spinBoxBoard2Channel7->setEnabled(false);
			ui->spinBoxBoard2Channel8->setEnabled(false);
			break;
		case 1:
			ui->spinBoxChannel1->setEnabled(false);
			ui->spinBoxChannel2->setEnabled(false);
			ui->spinBoxChannel3->setEnabled(false);
			ui->spinBoxChannel4->setEnabled(false);
			ui->spinBoxChannel5->setEnabled(false);
			ui->spinBoxChannel6->setEnabled(false);
			ui->spinBoxChannel7->setEnabled(false);
			ui->spinBoxChannel8->setEnabled(false);
			break;
		case 2:
			break;
		}
	}
}

DialogDac::~DialogDac()
{
}

void DialogDac::spinBoxChannelX_valueChanged()
{
	uint32_t output[2][8] =
	{
		ui->spinBoxChannel1->value(),
		ui->spinBoxChannel2->value(),
		ui->spinBoxChannel3->value(),
		ui->spinBoxChannel4->value(),
		ui->spinBoxChannel5->value(),
		ui->spinBoxChannel6->value(),
		ui->spinBoxChannel7->value(),
		ui->spinBoxChannel8->value(),
		ui->spinBoxBoard2Channel1->value(),
		ui->spinBoxBoard2Channel2->value(),
		ui->spinBoxBoard2Channel3->value(),
		ui->spinBoxBoard2Channel4->value(),
		ui->spinBoxBoard2Channel5->value(),
		ui->spinBoxBoard2Channel6->value(),
		ui->spinBoxBoard2Channel7->value(),
		ui->spinBoxBoard2Channel8->value()
	};
	bool isIr = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	for(uint32_t drvno=1; drvno<=number_of_boards; drvno++)
		mainWindow->lsc.dac_setAllOutputs(drvno, output[drvno], isIr);
	return;
}

void DialogDac::on_buttonBox_rejected()
{
	// Send old values to DAC
	bool isIr = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	for (uint32_t drvno = 1; drvno <= number_of_boards; drvno++)
		mainWindow->lsc.dac_setAllOutputs(drvno, output_old[drvno], isIr);
	return;
}

void DialogDac::on_buttonBox_accepted()
{
	// Save output values
	settings.setValue(settingSensorOffsetChannel1Path, ui->spinBoxChannel1->value());
	settings.setValue(settingSensorOffsetChannel2Path, ui->spinBoxChannel2->value());
	settings.setValue(settingSensorOffsetChannel3Path, ui->spinBoxChannel3->value());
	settings.setValue(settingSensorOffsetChannel4Path, ui->spinBoxChannel4->value());
	settings.setValue(settingSensorOffsetChannel5Path, ui->spinBoxChannel5->value());
	settings.setValue(settingSensorOffsetChannel6Path, ui->spinBoxChannel6->value());
	settings.setValue(settingSensorOffsetChannel7Path, ui->spinBoxChannel7->value());
	settings.setValue(settingSensorOffsetChannel8Path, ui->spinBoxChannel8->value());
	settings.setValue(settingSensorOffsetBoard2Channel1Path, ui->spinBoxBoard2Channel1->value());
	settings.setValue(settingSensorOffsetBoard2Channel2Path, ui->spinBoxBoard2Channel2->value());
	settings.setValue(settingSensorOffsetBoard2Channel3Path, ui->spinBoxBoard2Channel3->value());
	settings.setValue(settingSensorOffsetBoard2Channel4Path, ui->spinBoxBoard2Channel4->value());
	settings.setValue(settingSensorOffsetBoard2Channel5Path, ui->spinBoxBoard2Channel5->value());
	settings.setValue(settingSensorOffsetBoard2Channel6Path, ui->spinBoxBoard2Channel6->value());
	settings.setValue(settingSensorOffsetBoard2Channel7Path, ui->spinBoxBoard2Channel7->value());
	settings.setValue(settingSensorOffsetBoard2Channel8Path, ui->spinBoxBoard2Channel8->value());
	return;
}

void DialogDac::on_pushButtonDefault_pressed()
{
	ui->spinBoxChannel1->setValue(settingSensorOffsetChannel1Default);
	ui->spinBoxChannel2->setValue(settingSensorOffsetChannel2Default);
	ui->spinBoxChannel3->setValue(settingSensorOffsetChannel3Default);
	ui->spinBoxChannel4->setValue(settingSensorOffsetChannel4Default);
	ui->spinBoxChannel5->setValue(settingSensorOffsetChannel5Default);
	ui->spinBoxChannel6->setValue(settingSensorOffsetChannel6Default);
	ui->spinBoxChannel7->setValue(settingSensorOffsetChannel7Default);
	ui->spinBoxChannel8->setValue(settingSensorOffsetChannel8Default);
	ui->spinBoxBoard2Channel1->setValue(settingSensorOffsetBoard2Channel1Default);
	ui->spinBoxBoard2Channel2->setValue(settingSensorOffsetBoard2Channel2Default);
	ui->spinBoxBoard2Channel3->setValue(settingSensorOffsetBoard2Channel3Default);
	ui->spinBoxBoard2Channel4->setValue(settingSensorOffsetBoard2Channel4Default);
	ui->spinBoxBoard2Channel5->setValue(settingSensorOffsetBoard2Channel5Default);
	ui->spinBoxBoard2Channel6->setValue(settingSensorOffsetBoard2Channel6Default);
	ui->spinBoxBoard2Channel7->setValue(settingSensorOffsetBoard2Channel7Default);
	ui->spinBoxBoard2Channel8->setValue(settingSensorOffsetBoard2Channel8Default);
	return;
}