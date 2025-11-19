#include "dialogreferencemeasurement.h"

DialogReferenceMeasurement::DialogReferenceMeasurement(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogReferenceMeasurementClass())
{
	ui->setupUi(this);
	initDialog();
}

DialogReferenceMeasurement::~DialogReferenceMeasurement()
{
	delete ui;
}

/**
 * @brief Initialize dialog elements based on the number of connected boards.
 * @return none
 */
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

/**
 * @brief Update camera selection and reference button state, when selected board changes.
 * @return none
 */
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
	loadReferenceButtonState();
	return;
}

/**
 * @brief Update reference button state, when selected camera changes.
 * @return none
 */
void DialogReferenceMeasurement::on_spinBoxCamera_valueChanged()
{
	loadReferenceButtonState();
	return;
}

/**
 * @brief This slot saves or clears the reference measurement 1.
 * @return none
 */
void DialogReferenceMeasurement::on_pushButtonHandleReference1_pressed()
{
	handleReference(QString("1"));
	return;
}

/**
 * @brief This slot saves or clears the reference measurement 2.
 * @return none
 */
void DialogReferenceMeasurement::on_pushButtonHandleReference2_pressed()
{
	handleReference(QString("2"));
	return;
}

/**
 * @brief This slot saves or clears the reference measurement.
 * Reference name format: reference_series_<board>_<camera>_<id>
 * @param id Identifier for the reference measurement (e.g., "1" or "2")
 * @return none
 */
void DialogReferenceMeasurement::handleReference(QString id)
{ 
	uint32_t drvno = ui->spinBoxBoard->value();
	uint16_t camera = ui->spinBoxCamera->value();
	QString name = "reference_series_" + QString::number(drvno) + "_" + QString::number(camera) + "_" + id;
	QPushButton* referenceButton = (id == "1") ? ui->pushButtonHandleReference1 : ui->pushButtonHandleReference2;

	if (referenceButton->text() == "Save")
	{
		saveReference(name);
		referenceButton->setText("Clear");
	}
	else
	{
		clearReference(name);
		referenceButton->setText("Save");
	}
	return;
}

/**
 * @brief Saves the currently selected measurement as a QLineSeries with the given name to the chart.
 * @param seriesName Name of the reference series to be saved
 * @return none
 */
void DialogReferenceMeasurement::saveReference(QString seriesName)
{
	uint32_t drvno = ui->spinBoxBoard->value();
	settings.beginGroup("board" + QString::number(drvno));
	uint32_t sample = mainWindow->ui->horizontalSliderSample->value() - 1;
	uint32_t block = mainWindow->ui->horizontalSliderBlock->value() - 1;
	uint16_t camera = QString::number(ui->spinBoxCamera->value()).toInt();
	uint32_t pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
	settings.endGroup();
	size_t data_array_size = 0;
	data_array_size += pixel;
	uint16_t* camera_data = static_cast<uint16_t*>(malloc(data_array_size * sizeof(uint16_t)));
	es_status_codes status = mainWindow->lsc.copyOneSample(drvno, sample, block, camera, camera_data);
	//std::vector<uint16_t> camera_data(pixel);
	//es_status_codes status = mainWindow->lsc.copyOneSample(drvno, sample, block, camera, camera_data.data());

	if (status != es_no_error)
	{
		free(camera_data);
		return;
	}
	
	QLineSeries* referenceSeries = new QLineSeries();
	referenceSeries->setName(seriesName);
	for (uint32_t i = 0; i < pixel; i++)
	{
		referenceSeries->append(static_cast<qreal>(i), static_cast<qreal>(camera_data[i]));
	}
	free(camera_data);
	mainWindow->ui->chartView->chart()->addSeries(referenceSeries);
	referenceSeriesList.append(referenceSeries);
	return;
}

/**
 * @brief Clears the reference measurement with the given name from the chart.
 * @param seriesName Name of the reference series to be cleared
 * @return none
 */
void DialogReferenceMeasurement::clearReference(QString seriesName)
{
	for (QLineSeries* series : referenceSeriesList)
	{
		if (series->name() == seriesName)
		{
			mainWindow->ui->chartView->chart()->removeSeries(series);
			delete series;
			referenceSeriesList.removeOne(series);
			return;
		}
	}
	return;
}

/**
 * @brief Creates the name of the currently selected camera, and checks if reference measurements exist for it. Updates the button text accordingly.
 * @return none
 */
void DialogReferenceMeasurement::loadReferenceButtonState()
{
	uint32_t drvno = ui->spinBoxBoard->value();
	uint16_t camera = ui->spinBoxCamera->value();
	QString referenceName1 = "reference_series_" + QString::number(drvno) + "_" + QString::number(camera) + "_1";
	QString referenceName2 = "reference_series_" + QString::number(drvno) + "_" + QString::number(camera) + "_2";
	bool reference1Exists = false;
	bool reference2Exists = false;
	for (QLineSeries* series : referenceSeriesList)
	{
		if (series->name() == referenceName1) reference1Exists = true;
		if (series->name() == referenceName2) reference2Exists = true;
		if (reference1Exists && reference2Exists) break;
	}
	reference1Exists ? ui->pushButtonHandleReference1->setText("Clear") : ui->pushButtonHandleReference1->setText("Save");
	reference2Exists ? ui->pushButtonHandleReference2->setText("Clear") : ui->pushButtonHandleReference2->setText("Save");
	return;
}

