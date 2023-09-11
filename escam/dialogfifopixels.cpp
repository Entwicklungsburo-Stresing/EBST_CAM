#include "dialogfifopixels.h"
#include "lsc-gui.h"

DialogFifoPixels::DialogFifoPixels(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::DialogFifoPixelsClass)
{
	ui->setupUi(this);
	connect(ui->spinBoxBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogFifoPixels::updateValues);

	if (number_of_boards > 1)
		ui->spinBoxBoard->setMaximum(number_of_boards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	updateValues();
}

DialogFifoPixels::~DialogFifoPixels()
{
}

void DialogFifoPixels::updateValues()
{
	bool valid = false, overflow = false, empty = false, full = false;

	es_status_codes status = mainWindow->lsc.checkFifoValid(ui->spinBoxBoard->value(), &valid);
	if (status != es_no_error) return;
	status = mainWindow->lsc.checkFifoOverflow(ui->spinBoxBoard->value(), &overflow);
	if (status != es_no_error) return;
	status = mainWindow->lsc.checkFifoEmpty(ui->spinBoxBoard->value(), &empty);
	if (status != es_no_error) return;
	status = mainWindow->lsc.checkFifoFull(ui->spinBoxBoard->value(), &full);
	if (status != es_no_error) return;

	ui->labelFifoValidValue->setText(QString::number(valid));
	ui->labelFifoOverflowValue->setText(QString::number(overflow));
	ui->labelFifoEmptyValue->setText(QString::number(empty));
	ui->labelFifoFullValue->setText(QString::number(full));

	return;
}
