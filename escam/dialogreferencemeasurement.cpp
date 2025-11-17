#include "dialogreferencemeasurement.h"

DialogReferenceMeasurement::DialogReferenceMeasurement(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogReferenceMeasurementClass())
{
	ui->setupUi(this);
}

DialogReferenceMeasurement::~DialogReferenceMeasurement()
{
	delete ui;
}
