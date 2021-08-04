#include "dialogdac.h"
#include "ui_dialogdac.h"

DialogDac::DialogDac(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::DialogDac)
{
	ui->setupUi(this);
	// Save old values
	output_old[0] = settings.value(settingSensorOffsetChannel1Path, settingSensorOffsetChannel1Default).toInt();
	output_old[1] = settings.value(settingSensorOffsetChannel2Path, settingSensorOffsetChannel2Default).toInt();
	output_old[2] = settings.value(settingSensorOffsetChannel3Path, settingSensorOffsetChannel3Default).toInt();
	output_old[3] = settings.value(settingSensorOffsetChannel4Path, settingSensorOffsetChannel4Default).toInt();
	output_old[4] = settings.value(settingSensorOffsetChannel5Path, settingSensorOffsetChannel5Default).toInt();
	output_old[5] = settings.value(settingSensorOffsetChannel6Path, settingSensorOffsetChannel6Default).toInt();
	output_old[6] = settings.value(settingSensorOffsetChannel7Path, settingSensorOffsetChannel7Default).toInt();
	output_old[7] = settings.value(settingSensorOffsetChannel8Path, settingSensorOffsetChannel8Default).toInt();
	// Write the old values to UI
	ui->spinBoxChannel1->setValue(output_old[0]);
	ui->spinBoxChannel2->setValue(output_old[1]);
	ui->spinBoxChannel3->setValue(output_old[2]);
	ui->spinBoxChannel4->setValue(output_old[3]);
	ui->spinBoxChannel5->setValue(output_old[4]);
	ui->spinBoxChannel6->setValue(output_old[5]);
	ui->spinBoxChannel7->setValue(output_old[6]);
	ui->spinBoxChannel8->setValue(output_old[7]);
	// Connect all spin boxes (valueChanged) to the same slot
	connect(ui->spinBoxChannel1, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel2, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel3, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel4, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel5, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel6, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel7, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);
	connect(ui->spinBoxChannel8, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDac::spinBoxChannelX_valueChanged);

	//connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogDac::rejected);
	//connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogDac::accepted);
}

DialogDac::~DialogDac()
{
}

void DialogDac::spinBoxChannelX_valueChanged()
{
	uint32_t output[8] =
	{
		ui->spinBoxChannel1->value(),
		ui->spinBoxChannel2->value(),
		ui->spinBoxChannel3->value(),
		ui->spinBoxChannel4->value(),
		ui->spinBoxChannel5->value(),
		ui->spinBoxChannel6->value(),
		ui->spinBoxChannel7->value(),
		ui->spinBoxChannel8->value()
	};
	uint32_t drvno = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt();
	bool isIr = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	mainWindow->lsc.dac_setAllOutputs(drvno, output, isIr);
	return;
}

void DialogDac::on_buttonBox_rejected()
{
	// Send old values to DAC
	uint32_t drvno = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt();
	bool isIr = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	mainWindow->lsc.dac_setAllOutputs(drvno, output_old, isIr);
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
	return;
}

void DialogDac::on_pushButtonDefault_pressed()
{
	ui->spinBoxChannel3->setValue(settingSensorOffsetChannel3Default);
	ui->spinBoxChannel4->setValue(settingSensorOffsetChannel4Default);
	ui->spinBoxChannel5->setValue(settingSensorOffsetChannel5Default);
	ui->spinBoxChannel6->setValue(settingSensorOffsetChannel6Default);
	ui->spinBoxChannel7->setValue(settingSensorOffsetChannel7Default);
	ui->spinBoxChannel8->setValue(settingSensorOffsetChannel8Default);
	ui->spinBoxChannel1->setValue(settingSensorOffsetChannel1Default);
	ui->spinBoxChannel2->setValue(settingSensorOffsetChannel2Default);
	return;
}