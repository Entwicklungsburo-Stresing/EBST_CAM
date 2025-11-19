#include "dialogreferencemeasurement.h"

DialogReferenceMeasurement::DialogReferenceMeasurement(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogReferenceMeasurementClass())
{
	ui->setupUi(this);
	initDialog();
	connect(ui->pushButtonSaveReference1, &QPushButton::pressed, [this]() { on_pushButtonSaveReference_pressed(0); });
	connect(ui->pushButtonSaveReference2, &QPushButton::pressed, [this]() { on_pushButtonSaveReference_pressed(1); });
	connect(ui->pushButtonClearReference1, &QPushButton::pressed, [this]() { on_pushButtonClearReference_pressed(0); });
	connect(ui->pushButtonClearReference2, &QPushButton::pressed, [this]() { on_pushButtonClearReference_pressed(1); });
}

DialogReferenceMeasurement::~DialogReferenceMeasurement()
{
	delete ui;
}

void DialogReferenceMeasurement::initDialog()
{
	if (mainWindow->lsc.numberOfBoards > 1)
		ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	on_spinBoxBoard_valueChanged();
}

void DialogReferenceMeasurement::on_spinBoxBoard_valueChanged()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	uint32_t camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
	settings.endGroup();
	if (camcnt > 1)
		ui->spinBoxCamera->setMaximum(camcnt - 1);
	else
	{
		ui->spinBoxCamera->setVisible(false);
		ui->labelCamera->setVisible(false);
	}
	return;
}

void DialogReferenceMeasurement::on_pushButtonSaveReference_pressed(int referenceIndex)
{
	checkIfReferenceExistsAndDelete(referenceIndex);
	uint32_t drvno = QString::number(ui->spinBoxBoard->value()).toInt();
	settings.beginGroup("board" + QString::number(drvno));
	uint32_t sample = 20;
	uint32_t block = settings.value(settingNobPath, settingNobDefault).toDouble() - 1;
	uint16_t camera = QString::number(ui->spinBoxCamera->value()).toInt();
	uint32_t pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
	int sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toInt();
	settings.endGroup();
	size_t data_array_size = 0;
	data_array_size += pixel;
	uint16_t* camera_data = static_cast<uint16_t*>(malloc(data_array_size * sizeof(uint16_t)));
	es_status_codes status = mainWindow->lsc.copyOneSample(drvno, sample, block, camera, camera_data);
	if (status != es_no_error)
	{
		free(camera_data);
		return;
	}
	QLineSeries* referenceSeries = new QLineSeries();
	referenceSeries->setName("reference_series" + QString::number(referenceIndex));
	for (uint32_t i = 0; i < pixel; i++)
	{
		referenceSeries->append(static_cast<qreal>(i), static_cast<qreal>(camera_data[i]));
	}
	free(camera_data);
	mainWindow->ui->chartView->chart()->addSeries(referenceSeries);
	referenceSeriesList.append(referenceSeries);
	return;
}

void DialogReferenceMeasurement::on_pushButtonClearReference_pressed(int referenceIndex)
{
	checkIfReferenceExistsAndDelete(referenceIndex);
	mainWindow->ui->chartView->repaint();
	return;
}

void DialogReferenceMeasurement::checkIfReferenceExistsAndDelete(int referenceIndex)
{
	for (QLineSeries* series : referenceSeriesList)
	{
		if (series->name() == "reference_series" + QString::number(referenceIndex))
		{
			mainWindow->ui->chartView->chart()->removeSeries(series);
			delete series;
			referenceSeriesList.removeOne(series);
			return;
		}
	}
	return;
}

