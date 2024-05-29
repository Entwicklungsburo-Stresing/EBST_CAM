/*
This file is part of ESLSCDLL.

ESLSCDLL is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ESLSCDLL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.If not, see < http://www.gnu.org/licenses/>.

Copyright 2020 Entwicklungsbuero G. Stresing (http://www.stresing.de/)
*/

#include "mainwindow.h"
#include "dialogaxes.h"
#include "../version.h"
#include "dialogdac.h"
#include "dialogioctrl.h"
#include "dialogspecialpixels.h"
#ifdef WIN32
#include "dialoggreyscalesettings.h"
#endif

/**
 * @brief Constructor of Class MainWindow.
 * @param parent
 */
MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose);
	connect(ui->horizontalSliderSample, &QSlider::valueChanged, this, &MainWindow::loadCameraData);
	connect(ui->horizontalSliderBlock, &QSlider::valueChanged, this, &MainWindow::loadCameraData);
	connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
	connect(&lsc, &Lsc::measureStart, this, &MainWindow::on_measureStart);
	connect(&lsc, &Lsc::measureDone, this, &MainWindow::on_measureDone);
	connect(&lsc, &Lsc::blockStart, this, &MainWindow::on_blockStart);
	connect(&lsc, &Lsc::blockDone, this, &MainWindow::on_blockDone);
	connect(&lsc, &Lsc::allBlocksDone, this, &MainWindow::on_allBlocksDone);
	connect(ui->chartView, &MyQChartView::rubberBandChanged, this, &MainWindow::on_rubberBandChanged); 
	connect(liveViewTimer, &QTimer::timeout, this, &MainWindow::showCurrentScan);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::readScanFrequencyBit);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::readBlockFrequencyBit);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::findCamera);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::on_readCameraTemp);
	connect(scanFrequencyTimer, &QTimer::timeout, this, &MainWindow::on_scanFrequencyTooHigh);
	connect(blockFrequencyTimer, &QTimer::timeout, this, &MainWindow::on_blockFrequencyTooHigh);
	connect(ui->radioButtonLiveViewFixedSample, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);
	connect(ui->radioButtonLiveViewOff, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);
	connect(ui->radioButtonLiveViewOffNewestSample, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);


	es_status_codes status = lsc.initDriver();
	if (status != es_no_error)
		showStatusCodeDialog(status);
	status = lsc.initPcieBoard();
	if (status != es_no_error)
		showStatusCodeDialog(status);
	// Check if there are settings saved on this system
	if (!settings.contains(settingBoardSelPath))
	{
		// When no settings are found, offer to import settings
		QMessageBox d(this);
		d.setWindowTitle("No settings found");
		d.setText("No application settings are found on this system. Do you want to import settings?");
		d.setIcon(QMessageBox::Information);
		d.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		connect(&d, &QMessageBox::accepted, this, &MainWindow::on_actionImport_triggered);
		d.exec();
	}
	loadSettings();
	setDefaultAxes();

	lampsTimer->start(100);
	restoreGeometry(settings.value("centralwidget/geometry").toByteArray());
	restoreState(settings.value("centralwidget/state").toByteArray());

	// move lsc to its own thread
	lsc.moveToThread(&measurementThread);
	connect(&measurementThread, &QThread::started, &lsc, &Lsc::startMeasurement);
	connect(&lsc, &Lsc::measureDone, &measurementThread, &QThread::quit);
	connect(&lsc, &Lsc::allBlocksDone, ds_dsc, &DialogDSC::updateDSC);
	connect(&lsc, &Lsc::allBlocksDone, ds_rms, &DialogRMS::updateRMS);
#ifdef __linux__
	// disable gray scale menu on Linux
	ui->menuGreyscale_Viewer->setEnabled(false);
#endif
	// disable axes menu until first finish of measurement to avoid crash
	ui->actionAxes->setEnabled(false);
}

/**
 * @brief Destructor of class MainWindow.
 */
MainWindow::~MainWindow()
{
	delete ui;
}

/**
 * @brief Sets the data of chartView. This function takes the data in the Qt format QLineSeries.
 * @param series Data as an array of QLineSeries.
 * @param numberOfSets Number of data sets which are given in the array series.
 */
void MainWindow::setChartData(QLineSeries** series, uint16_t numberOfSets)
{
	QChart *chart = ui->chartView->chart();
	chart->removeAllSeries();
	for (uint16_t set = 0; set < numberOfSets; set++)
	{
		if (settings.value(settingAxesMirrorXPath).toBool())
		{
			QVector<QPointF> points = series[set]->pointsVector();
			for (int i = 0; i < points.size() / 2; i++)
			{
				points[i].setX(points.size() - i - 1);
				points[points.size() - i - 1].setX(i);
				series[set]->replace(i, points[points.size() - i - 1]);
				series[set]->replace(points.size() - i - 1, points[i]);
			}
		}
		chart->addSeries(series[set]);
	}
	chart->createDefaultAxes();
	QList<QAbstractAxis *> axes = ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis0->setMax(ui->chartView->curr_xmax);
	axis0->setMin(ui->chartView->curr_xmin);
	axis1->setMax(ui->chartView->curr_ymax);
	axis1->setMin(ui->chartView->curr_ymin);
	return;
}

/**
 * @brief This overloaded function takes data with a C pointer and a length, converts it into QLineSeries and passes it to setChartData.
 * @param data Pointer to data.
 * @param length Length of data.
 * @param numberOfSets Number of data sets which are stored in data pointer.
 */
void MainWindow::setChartData(uint16_t* data, uint32_t* length, uint16_t numberOfSets, QList<QString> lineSeriesNamesList)
{
	// Allocate memory for the pointer array to the QlineSeries.
	QLineSeries** series = static_cast<QLineSeries**>(calloc(numberOfSets, sizeof(QLineSeries*)));
	// Iterate through all data sets.
	uint16_t* cur_data_ptr = data;
	for(uint16_t set=0; set<numberOfSets; set++)
	{
		// Set the current data set to a new empty QLineSeries.
		series[set] = new QLineSeries(this);
		series[set]->setName(lineSeriesNamesList[set]);
		// Iterate through all data points for the current data set.
		for(uint16_t i=0; i<length[set]; i++)
		{
			// Append the current data point to the current data set.
			series[set]->append(i, *(cur_data_ptr));
			cur_data_ptr++;
		}
	}
	setChartData(series, numberOfSets);
	free(series);
	return;
}

/**
 * @brief Initializes Settings Struct before measurement started.
 * @return none
 */
void MainWindow::initSettings()
{
	settings_struct.board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	settings_struct.cont_pause_in_microseconds = settings.value(settingContinuousPauseInMicrosecondsPath, settingContinuousPausInMicrosecondsDefault).toDouble();
	settings_struct.continuous_measurement = ui->checkBoxLoopMeasurement->isChecked();
	settings_struct.nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	settings_struct.nob = settings.value(settingNobPath, settingNobDefault).toDouble();
	//camerasetup tab
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		settings.beginGroup("board" + QString::number(drvno));
		//measurement tab
		settings_struct.camera_settings[drvno].sti_mode = settings.value(settingStiPath, settingStiDefault).toDouble();
		settings_struct.camera_settings[drvno].bti_mode = settings.value(settingBtiPath, settingBtiDefault).toDouble();
		settings_struct.camera_settings[drvno].stime_in_microsec = settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble();
		settings_struct.camera_settings[drvno].btime_in_microsec = settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble();
		settings_struct.camera_settings[drvno].sdat_in_10ns = settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble();
		settings_struct.camera_settings[drvno].bdat_in_10ns = settings.value(settingBdat_in_10nsPath, settingBdat_in_10nsDefault).toDouble();
		settings_struct.camera_settings[drvno].sslope = settings.value(settingSslopePath, settingSslopeDefault).toDouble();
		settings_struct.camera_settings[drvno].bslope = settings.value(settingBslopePath, settingBslopeDefault).toDouble();
		settings_struct.camera_settings[drvno].xckdelay_in_10ns = settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toDouble();
		settings_struct.camera_settings[drvno].sec_in_10ns = settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toDouble();
		settings_struct.camera_settings[drvno].bec_in_10ns = settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toDouble();
		settings_struct.camera_settings[drvno].trigger_mode_integrator = settings.value(settingTriggerModeIntegratorPath, settingTriggerModeIntegratorDefault).toDouble();
		settings_struct.camera_settings[drvno].sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble();
		settings_struct.camera_settings[drvno].is_fft_legacy = settings.value(settingIsFftLegacyPath, settingIsFftlegacyDefault).toBool();
		settings_struct.camera_settings[drvno].camera_system = settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble();
		settings_struct.camera_settings[drvno].camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
		settings_struct.camera_settings[drvno].pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
		settings_struct.camera_settings[drvno].led_off = settings.value(settingLedPath, settingLedDefault).toBool();
		settings_struct.camera_settings[drvno].sensor_gain = settings.value(settingSensorGainPath, settingSensorGainDefault).toDouble();
		settings_struct.camera_settings[drvno].adc_gain = settings.value(settingAdcGainPath, settingAdcGainDefault).toDouble();
		settings_struct.camera_settings[drvno].temp_level = settings.value(settingCoolingPath, settingCoolingDefault).toDouble();
		settings_struct.camera_settings[drvno].gpx_offset = settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_impact_start_pixel = settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toDouble();
		settings_struct.camera_settings[drvno].use_software_polling = settings.value(settingsUseSoftwarePollingPath, settingsUseSoftwarePollingDefault).toBool();
		settings_struct.camera_settings[drvno].is_cooled_camera_legacy_mode = settings.value(settingIsCooledCameraLegacyModePath, settingIsCooledCameraLegacyModeDefault).toBool();
		settings_struct.camera_settings[drvno].sensor_reset_length_in_4_ns = settings.value(settingSensorResetLengthIn4nsPath, settingSensorResetLengthIn4nsDefault).toDouble();
		//fftmodes tab
		settings_struct.camera_settings[drvno].fft_lines = settings.value(settingLinesPath, settingLinesDefault).toDouble();
		settings_struct.camera_settings[drvno].vfreq = settings.value(settingVfreqPath, settingVfreqDefault).toDouble();
		settings_struct.camera_settings[drvno].fft_mode = settings.value(settingFftModePath, settingFftModeDefault).toDouble();
		settings_struct.camera_settings[drvno].lines_binning = settings.value(settingLinesBinningPath, settingLinesBinningDefault).toDouble();
		settings_struct.camera_settings[drvno].number_of_regions = settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toDouble();
		settings_struct.camera_settings[drvno].s1s2_read_delay_in_10ns = settings.value(settingS1S2ReadDelayIn10nsPath, settingS1S2ReadDelayIn10nsDefault).toDouble();;
		settings_struct.camera_settings[drvno].region_size[0] = settings.value(settingRegionSize1Path, settingRegionSize1Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[1] = settings.value(settingRegionSize2Path, settingRegionSize2Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[2] = settings.value(settingRegionSize3Path, settingRegionSize3Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[3] = settings.value(settingRegionSize4Path, settingRegionSize4Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[4] = settings.value(settingRegionSize5Path, settingRegionSize5Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[5] = settings.value(settingRegionSize6Path, settingRegionSize6Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[6] = settings.value(settingRegionSize7Path, settingRegionSize7Default).toDouble();
		settings_struct.camera_settings[drvno].region_size[7] = settings.value(settingRegionSize8Path, settingRegionSize8Default).toDouble();
		//export data tab
		settings_struct.camera_settings[drvno].write_to_disc = settings.value(settingWriteDataToDiscPath, settingWriteToDiscDefault).toBool();
		QByteArray array = settings.value(settingFilePathPath, QDir::currentPath()).toString().toLocal8Bit();
		strcpy(settings_struct.camera_settings[drvno].file_path, array.data());
		//dac
		for (int camera = 0; camera < MAXCAMCNT; camera++)
			for (int channel = 0; channel < 8; channel++)
				settings_struct.camera_settings[drvno].dac_output[camera][channel] = settings.value(settingDacCameraChannelBaseDir + QString::number(channel + 1) + "Pos" + QString::number(camera), settingDacCameraDefault).toDouble();
		//debug
		settings_struct.camera_settings[drvno].tor = settings.value(settingTorPath, settingTorDefault).toDouble();
		settings_struct.camera_settings[drvno].adc_mode = settings.value(settingAdcModePath, settingAdcModeDefault).toDouble();
		settings_struct.camera_settings[drvno].adc_custom_pattern = settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toDouble();
		settings_struct.camera_settings[drvno].bnc_out = settings.value(settingBncOutPath, settingBncOutDefault).toDouble();
		settings_struct.camera_settings[drvno].tocnt = settings.value(settingTocntPath, settingTocntDefault).toDouble();
		settings_struct.camera_settings[drvno].ticnt = settings.value(settingTicntPath, settingTicntDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[0] = settings.value(settingIOCtrlOutput1DelayIn5nsPath, settingIOCtrlOutput1DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[1] = settings.value(settingIOCtrlOutput2DelayIn5nsPath, settingIOCtrlOutput2DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[2] = settings.value(settingIOCtrlOutput3DelayIn5nsPath, settingIOCtrlOutput3DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[3] = settings.value(settingIOCtrlOutput4DelayIn5nsPath, settingIOCtrlOutput4DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[4] = settings.value(settingIOCtrlOutput5DelayIn5nsPath, settingIOCtrlOutput5DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[5] = settings.value(settingIOCtrlOutput6DelayIn5nsPath, settingIOCtrlOutput6DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[6] = settings.value(settingIOCtrlOutput7DelayIn5nsPath, settingIOCtrlOutput7DelayIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[0] = settings.value(settingIOCtrlOutput1WidthIn5nsPath, settingIOCtrlOutput1WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[1] = settings.value(settingIOCtrlOutput2WidthIn5nsPath, settingIOCtrlOutput2WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[2] = settings.value(settingIOCtrlOutput3WidthIn5nsPath, settingIOCtrlOutput3WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[3] = settings.value(settingIOCtrlOutput4WidthIn5nsPath, settingIOCtrlOutput4WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[4] = settings.value(settingIOCtrlOutput5WidthIn5nsPath, settingIOCtrlOutput5WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[5] = settings.value(settingIOCtrlOutput6WidthIn5nsPath, settingIOCtrlOutput6WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[6] = settings.value(settingIOCtrlOutput7WidthIn5nsPath, settingIOCtrlOutput7WidthIn5nsDefault).toDouble();
		settings_struct.camera_settings[drvno].ioctrl_T0_period_in_10ns = settings.value(settingIOCtrlT0PeriodIn10nsPath, settingIOCtrlT0PeriodIn10nsDefault).toDouble();
		settings.endGroup();
	}
}


/**
 * @brief Slot to start measurement. Called by on_pushButtonStartStop_pressed.
 * @return none
 */
void MainWindow::startPressed()
{
	initSettings();
	es_status_codes status = lsc.initMeasurement();
	if (status != es_no_error)
	{
		showStatusCodeDialog(status);
		QMessageBox* d = new QMessageBox(this);
		d->setWindowTitle("Camera not found");
		d->setText("Do you want to use dummy data?");
		d->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		int ret = d->exec();
		switch (ret)
		{
		case QMessageBox::Yes:
			// Yes was clicked
			for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
				lsc.fillUserBufferWithDummyData(drvno);
			break;
		default:
		case QMessageBox::No:
			// No was clicked
			// Do nothing
			break;
		}
	}
	else
		measurementThread.start();
	return;
}

/**
 * @brief This slot opens the settings dialog.
 * @return none
 */
void MainWindow::on_actionEdit_triggered()
{
	DialogSettings* ds = new DialogSettings( this );
	ds->setAttribute( Qt::WA_DeleteOnClose );
	ds->show();
	connect( ds, &DialogSettings::settings_saved, this, &MainWindow::loadSettings );
	connect( ds, &DialogSettings::settings_saved, this, &MainWindow::setDefaultAxes );
	return;
}

/**
 * @brief This slot opens the verify data file dialog.
 */
void MainWindow::on_actionVerify_data_file_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Verify data", nullptr, tr("data files (*.dat);;all files (*)"));
	if (fileName.isEmpty()) return;
	struct verify_data_parameter vd;
	strcpy(vd.filename_full, fileName.toStdString().c_str());
	QDialog* messageBox = new QDialog(this);
	messageBox->setAttribute(Qt::WA_DeleteOnClose);
	messageBox->setSizeGripEnabled(true);
	QVBoxLayout* layout = new QVBoxLayout(messageBox);
	messageBox->setLayout(layout);
	QLabel* labelDrv = new QLabel(messageBox);
	labelDrv->setTextInteractionFlags(Qt::TextSelectableByMouse);
	labelDrv->setTextFormat(Qt::RichText);
	labelDrv->setText(QString::fromStdString(lsc.getVerifiedDataDialog(&vd)));
	labelDrv->setAlignment(Qt::AlignTop);
	QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, messageBox);
	connect(dialogButtonBox, &QDialogButtonBox::accepted, messageBox, &QDialog::accept);
	layout->addWidget(labelDrv);
	layout->addWidget(dialogButtonBox);
	messageBox->setWindowTitle("Verify data");
	messageBox->show();
	return;
}

/**
 * @brief This slot exports the measurement data to HDF5.
 * @return none
 */
void MainWindow::on_actionExport_data_triggered()
{
	QString path = QFileDialog::getSaveFileName(this, "Export data", "measurement.h5", tr("HDF5 files(*.h5)"), nullptr, QFileDialog::ShowDirsOnly);
	if (path.isEmpty()) return;
	QFileInfo fi(path);
	QString filename = fi.fileName();
	QString filepath = fi.path();
	QByteArray ba = filepath.toLatin1();
	const char* pathString = ba.data();
	QByteArray ba2 = filename.toLatin1();
	const char* filenameString = ba2.data();
	es_status_codes status = mainWindow->lsc.exportMeasurementHDF5(pathString, filenameString);

	QDialog* messageBox = new QDialog(this);
	messageBox->setAttribute(Qt::WA_DeleteOnClose);
	QVBoxLayout* layout = new QVBoxLayout(messageBox);
	messageBox->setLayout(layout);
	QLabel* labelExport = new QLabel(messageBox);
	labelExport->setTextInteractionFlags(Qt::TextSelectableByMouse);
	labelExport->setTextFormat(Qt::RichText);
	labelExport->setAlignment(Qt::AlignTop);
	QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, messageBox);
	connect(dialogButtonBox, &QDialogButtonBox::accepted, messageBox, &QDialog::accept);
	layout->addWidget(labelExport);
	layout->addWidget(dialogButtonBox);
	messageBox->setWindowTitle("Export data");
	if (status != es_no_error)
	{ 
		QString errorMsg = QString::fromStdString("Error exporting data:\n");
		errorMsg.append(QString::fromStdString(ConvertErrorCodeToMsg(status)));
		labelExport->setText(errorMsg);
		messageBox->show();
		return;
	}
	labelExport->setText(QString::fromStdString("Data exported successfully."));
	messageBox->show();
	return;
}

/**
 * @brief This slot opens the RMS dialog.
 * @return none
 */
void MainWindow::on_actionRMS_triggered()
{
	ds_rms->initDialogRMS();
	ds_rms->show();
	return;
}

/**
 * @brief This slot opens the DSC dialog.
 * @return none
 */
void MainWindow::on_actionDSC_triggered()
{
	ds_dsc->initDialogDsc();
	ds_dsc->show();
	return;
}

/**
 * @brief This slot opens the settings dialog for the charts axes.
 * @return none
 */
void MainWindow::on_actionAxes_triggered()
{
	DialogAxes* messageBox = new DialogAxes(this);
	connect(ui->chartView, &MyQChartView::rubberBandChanged, messageBox, &DialogAxes::on_rubberband_valueChanged);
	connect(ui->actionReset_axes, &QAction::triggered, messageBox, &DialogAxes::on_rubberband_valueChanged);
	messageBox->setAttribute(Qt::WA_DeleteOnClose);
	messageBox->show();
	return;
}

/**
 * @brief This slot opens the settings dialog for selecting the cameras to be displayed on the chart.
 * @return none
 */
void MainWindow::on_actionCameras_triggered()
{
	QDialog* messageBox = new QDialog(this);
	messageBox->setAttribute(Qt::WA_DeleteOnClose);
	QVBoxLayout* layout = new QVBoxLayout(messageBox);
	messageBox->setLayout(layout);
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	uint32_t camcnt = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		settings.beginGroup("board" + QString::number(drvno));
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
			// If camcnt is 0, treat as camcnt 1
			if (camcnt == 0)
				camcnt = 1;
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				QCheckBox* checkbox = new QCheckBox(messageBox);
				checkbox->setText("Board " + QString::number(drvno) + ", Camera " + QString::number(cam));
				checkbox->setChecked(settings.value(settingShowCameraBaseDir + QString::number(cam), settingShowCameraDefault).toBool());
				layout->addWidget(checkbox);
				// Lambda syntax is used to pass additional argument i
				connect(checkbox, &QCheckBox::stateChanged, this, [checkbox, this, cam, drvno] {on_checkBoxShowCamera(checkbox->isChecked(), cam, drvno); loadCameraData(); });
			}
		}
		settings.endGroup();
	}
	QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, messageBox);
	connect(dialogButtonBox, &QDialogButtonBox::accepted, messageBox, &QDialog::accept);
	layout->addWidget(dialogButtonBox);
	messageBox->setWindowTitle("Cameras");
	messageBox->show();

	return;
}

void MainWindow::on_checkBoxShowCamera(bool state, int camera, uint32_t drvno)
{
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingShowCameraBaseDir + QString::number(camera), state);
	settings.endGroup();
	return;
}

void MainWindow::on_actionReset_axes_triggered()
{
	setDefaultAxes();
}

void MainWindow::on_actionContext_help_triggered()
{
	QWhatsThis::enterWhatsThisMode();
}

void MainWindow::setDefaultAxes()
{
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	qreal xmax = 0;
	qreal ymax = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			qreal pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			if (pixel - 1 > xmax)
				xmax = pixel - 1;
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble() == camera_system_3030 && ymax != 0xFFFF)
				ymax = 0x3FFF;
			else
				ymax = 0xFFFF;
			settings.endGroup();
		}
	}
	ui->chartView->curr_ymax = ymax;
	ui->chartView->curr_xmax = xmax;
	ui->chartView->curr_xmin = 0;
	ui->chartView->curr_ymin = 0;
	// retrieve axis pointer
	QList<QAbstractAxis*> axes = ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis0->setMax(ui->chartView->curr_xmax);
	axis0->setMin(ui->chartView->curr_xmin);
	axis1->setMax(ui->chartView->curr_ymax);
	axis1->setMin(ui->chartView->curr_ymin);
	return;
}

void MainWindow::on_actionAbout_triggered()
{
	QString aboutText = "This is Escam version ";
	aboutText.append(VER_FILE_VERSION_STR);
	aboutText.append("\n");
	aboutText.append(VER_COPYRIGHT_STR);
	aboutText.append("\n");
	aboutText.append(VER_COMPANY_NAME);
	aboutText.append("\n");
	aboutText.append("stresing.de");
	QMessageBox::about(this, "About", aboutText);
	return;
}

void MainWindow::on_actionAbout_Qt_triggered()
{
	QMessageBox::aboutQt(this, "About Qt");
	return;
}

void MainWindow::on_actionDAC_triggered()
{
	DialogDac* dialogDac = new DialogDac(this);
	dialogDac->setAttribute(Qt::WA_DeleteOnClose);
	dialogDac->show();
	return;
}

/**
 * @brief Load settings and apply to UI.
 */
void MainWindow::loadSettings()
{
	bool coolingOn = false;
	bool isOvertempCam = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint8_t tor = static_cast<uint8_t>(settings.value(settingTorPath, settingTorDefault).toDouble());
			bool coolingOnBoard = false;
			if (settings.value(settingCoolingPath, settingCoolingDefault).toDouble() > 0)
				coolingOnBoard = true;
			int cameraSystem = settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble();
			if (cameraSystem == camera_system_3030) isOvertempCam = true;
			coolingOn |= coolingOnBoard;
			settings.endGroup();
			lsc.setTorOut(drvno, tor);
		}
	}
	if (isOvertempCam) 
	{
		ui->widgetOvertempParent->setVisible(true);
	}
	else
	{
		ui->widgetOvertempParent->setVisible(coolingOn);
	}
	int nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	ui->horizontalSliderSample->setMaximum(nos);
	ui->spinBoxSample->setMaximum(nos);
	int nob = settings.value(settingNobPath, settingNobDefault).toDouble();
	ui->horizontalSliderBlock->setMaximum(nob);
	ui->spinBoxBlock->setMaximum(nob);
	ui->horizontalSliderSample->setValue(nos);
	ui->horizontalSliderBlock->setValue(nob);
	QString theme = settings.value(settingThemePath, settingThemeDefault).toString();
	QApplication::setStyle(QStyleFactory::create(theme));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
	QStyleHints* qstyle = QApplication::styleHints();
	if(qstyle->colorScheme() == Qt::ColorScheme::Dark && theme != "windowsvista")
		ui->chartView->chart()->setTheme(QChart::ChartThemeDark);
	else
		ui->chartView->chart()->setTheme(QChart::ChartThemeLight);
#endif
	return;
}

void MainWindow::on_actionDump_board_registers_triggered()
{
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QDialog* messageBox = new QDialog(this);
	messageBox->setAttribute(Qt::WA_DeleteOnClose);
	messageBox->setSizeGripEnabled(true);
	QVBoxLayout* layout = new QVBoxLayout(messageBox);
	messageBox->setLayout(layout);
	QTabWidget* tabWidget = new QTabWidget(messageBox);
	tabWidget->setDocumentMode(true);
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		QScrollArea* scrollDrv = new QScrollArea(tabWidget);
		scrollDrv->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelDrv = new QLabel(scrollDrv);
		labelDrv->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelDrv->setTextFormat(Qt::RichText);
		labelDrv->setText(QString::fromStdString(lsc.__AboutDrv(drvno)));
		labelDrv->setAlignment(Qt::AlignTop);
		scrollDrv->setWidget(labelDrv);
		tabWidget->addTab(scrollDrv, "About driver board " + QString::number(drvno));
		//QLabel* labelGPX = new QLabel(tabWidget);
		//labelGPX->setTextInteractionFlags(Qt::TextSelectableByMouse);
		//labelGPX->setTextFormat(Qt::RichText);
		//labelGPX->setText(QString::fromStdString(lsc.__AboutGPX(drvno)));
		//labelGPX->setAlignment(Qt::AlignTop);
		//tabWidget->addTab(labelGPX, "About GPX board " + QString::number(drvno));
		QScrollArea* scrollS0 = new QScrollArea(tabWidget);
		scrollS0->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelS0 = new QLabel(scrollS0);
		labelS0->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelS0->setTextFormat(Qt::RichText);
		labelS0->setText(QString::fromStdString(lsc._dumpS0Registers(drvno)));
		labelS0->setAlignment(Qt::AlignTop);
		scrollS0->setWidget(labelS0);
		tabWidget->addTab(scrollS0, "S0 registers board " + QString::number(drvno));
		QScrollArea* scrollHRS0 = new QScrollArea(tabWidget);
		scrollHRS0->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelHRS0 = new QLabel(scrollHRS0);
		labelHRS0->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelHRS0->setTextFormat(Qt::RichText);
		labelHRS0->setText(QString::fromStdString(lsc._dumpHumanReadableS0Registers(drvno)));
		labelHRS0->setAlignment(Qt::AlignTop);
		scrollHRS0->setWidget(labelHRS0);
		tabWidget->addTab(scrollHRS0, "Hum Read S0 board " + QString::number(drvno));
		QScrollArea* scrollDma = new QScrollArea(tabWidget);
		scrollDma->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelDma = new QLabel(scrollDma);
		labelDma->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelDma->setTextFormat(Qt::RichText);
		labelDma->setText(QString::fromStdString(lsc._dumpDmaRegisters(drvno)));
		labelDma->setAlignment(Qt::AlignTop);
		scrollDma->setWidget(labelDma);
		tabWidget->addTab(scrollDma, "DMA registers board " + QString::number(drvno));
		QScrollArea* scrollTlp = new QScrollArea(tabWidget);
		scrollTlp->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelTlp = new QLabel(scrollTlp);
		labelTlp->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelTlp->setTextFormat(Qt::RichText);
		labelTlp->setText(QString::fromStdString(lsc._dumpTlp(drvno)));
		labelTlp->setAlignment(Qt::AlignTop);
		scrollTlp->setWidget(labelTlp);
		tabWidget->addTab(scrollTlp, "TLP size board " + QString::number(drvno));
		QScrollArea* scrollPci = new QScrollArea(tabWidget);
		scrollPci->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelPci = new QLabel(scrollPci);
		labelPci->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelPci->setTextFormat(Qt::RichText);
		labelPci->setText(QString::fromStdString(lsc._dumpPciRegisters(drvno)));
		labelPci->setAlignment(Qt::AlignTop);
		scrollPci->setWidget(labelPci);
		tabWidget->addTab(scrollPci, "PCI registers board " + QString::number(drvno));
		QScrollArea* scrollSettings = new QScrollArea(tabWidget);
		scrollSettings->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		QLabel* labelSettings = new QLabel(scrollPci);
		labelSettings->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelSettings->setTextFormat(Qt::RichText);
		labelSettings->setText(QString::fromStdString(lsc._dumpCameraSettings(drvno)));
		labelSettings->setAlignment(Qt::AlignTop);
		scrollSettings->setWidget(labelSettings);
		tabWidget->addTab(scrollSettings, "Camera settings board " + QString::number(drvno));
	}
	QScrollArea* scrollSettings = new QScrollArea(tabWidget);
	scrollSettings->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QLabel* labelSettings = new QLabel(scrollSettings);
	labelSettings->setTextInteractionFlags(Qt::TextSelectableByMouse);
	labelSettings->setTextFormat(Qt::RichText);
	labelSettings->setText(QString::fromStdString(lsc._dumpMeasurementSettings()));
	labelSettings->setAlignment(Qt::AlignTop);
	scrollSettings->setWidget(labelSettings);
	tabWidget->addTab(scrollSettings, "Measurement settings");
	layout->addWidget(tabWidget);
	QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, tabWidget);
	connect(dialogButtonBox, &QDialogButtonBox::accepted, messageBox, &QDialog::accept);
	layout->addWidget(dialogButtonBox);
	messageBox->setWindowTitle("Register dump");
	messageBox->show();
	QGuiApplication::restoreOverrideCursor();
	return;
}

void MainWindow::loadCameraData()
{
	uint32_t pixel = 0;
	// Save pixel values in an array for the case when there are different pixel numbers on different PCIe boards.
	uint32_t pixel_array[MAXPCIECARDS];
	// camcnt is the count of all cameras
	uint32_t camcnt = 0;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	// showCamcnt is the count of all cameras to be shown on the chart
	// = sum of all true settingShowCameraBaseDir settings
	uint32_t showCamcnt = 0;
	size_t data_array_size = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
			// if camcnt is 0, treat as 1
			if (camcnt == 0)
				camcnt = 1;
			pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				bool showCurrentCam = settings.value(settingShowCameraBaseDir + QString::number(cam), settingShowCameraDefault).toBool();
				if (showCurrentCam)
				{
					showCamcnt++;
					data_array_size += pixel;
				}
			}
			settings.endGroup();
		}
	}
	uint16_t* data = static_cast<uint16_t*>(malloc(data_array_size * sizeof(uint16_t)));
	uint16_t* cur_data_ptr = data;
	uint32_t block = static_cast<uint32_t>(ui->horizontalSliderBlock->value() - 1);
	uint32_t sample = static_cast<uint32_t>(ui->horizontalSliderSample->value() - 1);
	// showedCam counts the number of cameras which are shown on the chart
	uint32_t showedCam = 0;
	QList<QString> lineSeriesNamesList;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
			// if camcnt is 0, treat as 1
			if (camcnt == 0)
				camcnt = 1;
			pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				bool showCurrentCam = settings.value(settingShowCameraBaseDir + QString::number(cam), settingShowCameraDefault).toBool();
				if (showCurrentCam)
				{
					es_status_codes status = lsc.returnFrame(drvno, sample, block, cam, pixel, cur_data_ptr);
					if (status != es_no_error) break;
					pixel_array[showedCam] = pixel;
					cur_data_ptr += pixel;
					showedCam++;
					lineSeriesNamesList.append(QString("Y%1 Board: %2; Camera: %3").arg(showedCam - 1).arg(static_cast<int>(drvno)).arg(static_cast<int>(cam)));
				}
			}
			settings.endGroup();
		}
	}
	if(showedCam)
		setChartData(data, pixel_array, static_cast<uint16_t>(showCamcnt), lineSeriesNamesList);

	// Deactivate legend, because it is blinking. Activate it, when a solution is found
	//if (showedCam > 1)
	//	ui->chartView->chart()->legend()->setVisible(true);
	//else
	//	ui->chartView->chart()->legend()->setVisible(false);

	free(data);
	return;
}

void MainWindow::on_measureStart()
{
	measureOn = true;
	//set measureOn lamp on
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::green);
	ui->widgetMeasureOn->setPalette(pal);
	//disable start button
	ui->pushButtonStartStop->setText("Stop");
	adjustLiveView();
	return;
}

void MainWindow::on_measureDone()
{
	measureOn = false;
	//set measureOn lamp off
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::darkGreen);
	ui->widgetMeasureOn->setPalette(pal);
	//Set correct value for sample and block slider
	if (ui->radioButtonLiveViewOffNewestSample->isChecked()) {
		int64_t sample = 0;
		int64_t block = 0;
		uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
		for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
		{
			// Check if the drvno'th bit is set
			if ((board_sel >> drvno) & 1)
			{
				lsc.getCurrentScanNumber(drvno, &sample, &block);
			}
		}
		ui->horizontalSliderSample->setValue(sample + 1);
		ui->horizontalSliderBlock->setValue(block + 1);
	}
	//set blockOn lamp off
	ui->widgetBlockOn->setPalette(pal);
	//enable start button
	ui->pushButtonStartStop->setText("Start");
	liveViewTimer->stop();
	//enable controls
	ui->spinBoxBlock->setEnabled(true);
	ui->spinBoxSample->setEnabled(true);
	ui->horizontalSliderBlock->setEnabled(true);
	ui->horizontalSliderSample->setEnabled(true);
	ui->actionAxes->setEnabled(true);
	return;
}

void MainWindow::on_blockStart()
{
	//set blockOn lamp on
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::green);
	ui->widgetBlockOn->setPalette(pal);
	return;
}

void MainWindow::on_blockDone()
{
	//set blockOn lamp off
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::darkGreen);
	ui->widgetBlockOn->setPalette(pal);
	return;
}

void MainWindow::on_allBlocksDone()
{
	//display camera data
	loadCameraData();
#ifdef WIN32
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			settings.endGroup();
			uint32_t nos = settings.value(settingNosPath, settingNosDefault).toDouble();
			uint32_t block = ui->horizontalSliderBlock->value() - 1;
			lsc.showNewBitmap(drvno, block, 0, pixelcount, nos);
		}
	}
#endif
	showCurrentScan();
	return;
}

void MainWindow::readScanFrequencyBit()
{
	bool isScanFrequencyTooHigh = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool freqTooHigh = false;
			es_status_codes status = lsc.readScanFrequencyBit(drvno, &freqTooHigh);
			isScanFrequencyTooHigh |= freqTooHigh;
			if (isScanFrequencyTooHigh) lsc.resetScanFrequencyBit(drvno);
		}
	}
	if (isScanFrequencyTooHigh) {
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::red);
		ui->widgetScanFreqHigh->setPalette(pal);
		scanFrequencyTimer->start(1000);
	}
	return;
}

void MainWindow::on_scanFrequencyTooHigh()
{
	scanFrequencyTimer->stop();
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::darkRed);
	ui->widgetScanFreqHigh->setPalette(pal);
	return;
}

void MainWindow::readBlockFrequencyBit()
{
	bool isBlockFrequencyTooHigh = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool freqTooHigh = false;
			es_status_codes status = lsc.readBlockFrequencyBit(drvno, &freqTooHigh);
			isBlockFrequencyTooHigh |= freqTooHigh;
			if (isBlockFrequencyTooHigh) lsc.resetBlockFrequencyBit(drvno);
		}
	}
	if (isBlockFrequencyTooHigh) {
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::red);
		ui->widgetBlockFreqHigh->setPalette(pal);
		blockFrequencyTimer->start(1000);
	}
	return;
}

void MainWindow::findCamera()
{
	bool allCamerasFound;
	if (number_of_boards)
		allCamerasFound = true;
	else
		allCamerasFound = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool cameraFound;
			es_status_codes status = lsc.findCam(drvno);
			if (status != es_no_error)
				cameraFound = false;
			else
				cameraFound = true;
			allCamerasFound &= cameraFound;
		}
	}
	if (allCamerasFound)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::green);
		ui->widgetCameraFound->setPalette(pal);
	}
	else
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::darkGreen);
		ui->widgetCameraFound->setPalette(pal);
	}
	return;
}

void MainWindow::on_blockFrequencyTooHigh()
{
	blockFrequencyTimer->stop();
	QPalette pal = palette();
	pal.setColor(QPalette::Window, Qt::darkRed);
	ui->widgetBlockFreqHigh->setPalette(pal);
	return;
}

/**
 * @brief Reads the camera overtemp and the camera temp good bit, and sets the LED to it's according color.
 * RED if Overtemp bit is set
 * GREEN is temp good bit is set
 * DARK GRAY if no bit is set
 */
void MainWindow::on_readCameraTemp()
{
	bool isOvertemp = false;
	bool isCooled = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	int64_t sample = 0;
	int64_t block = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++) 
	{
		// Check if the drvno'th bis is set
		if ((board_sel >> drvno) & 1)
		{
			lsc.getCurrentScanNumber(drvno, &sample, &block);
			if (sample >= 0 && aCAMCNT[drvno] > 0)
			{
				bool cameraBoardOvertemp = false;
				bool cameraBoardCooled = false;
				for (uint16_t camera_pos = 0; camera_pos < aCAMCNT[drvno]; camera_pos++)
				{
					bool cameraOvertemp = false;
					es_status_codes status = lsc.getCameraStatusOverTemp(drvno, static_cast<uint32_t>(sample), static_cast<uint32_t>(block), camera_pos, &cameraOvertemp);
					cameraBoardOvertemp |= cameraOvertemp;
					bool cameraCooled = false;
					status = lsc.getCameraStatusTempGood(drvno, static_cast<uint32_t>(sample), static_cast<uint32_t>(block), camera_pos, &cameraCooled);
					cameraBoardCooled |= cameraCooled;
				}
				isOvertemp |= cameraBoardOvertemp;
				isCooled |= cameraBoardCooled;
			}
		}
	}

	if (isOvertemp)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::red);
		ui->widgetOvertempParent->setToolTip("Camera temperature too high");
		ui->widgetOvertemp->setPalette(pal);
	}
	else if (isCooled)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::green);
		ui->widgetOvertempParent->setToolTip("Camera temperature good");
		ui->widgetOvertemp->setPalette(pal);
	}
	else
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::darkGray);
		ui->widgetOvertempParent->setToolTip("Camera temperature ok");
		ui->widgetOvertemp->setPalette(pal);
	}
	return;

}

void MainWindow::on_rubberBandChanged()
{
	// retrieve axis pointer
	QList<QAbstractAxis*> axes = ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	// save current axis configuration
	ui->chartView->curr_xmax = axis0->max();
	ui->chartView->curr_xmin = axis0->min();
	ui->chartView->curr_ymax = axis1->max();
	ui->chartView->curr_ymin = axis1->min();
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	qreal ymax = 0;
	uint max_pixel = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint cur_pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
			if (max_pixel < cur_pixel)
				max_pixel = cur_pixel;
			qreal cur_ymax;
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble() == 2)
				cur_ymax = 0x3FFF;
			else
				cur_ymax = 0xFFFF;
			if (ymax < cur_ymax)
				ymax = cur_ymax;
			settings.endGroup();
		}
	}
	// apply boundaries on axes
	if (axis0->max() > max_pixel)
	{
		ui->chartView->curr_xmax = max_pixel - 1;
		axis0->setMax(ui->chartView->curr_xmax);
	}
	if (axis0->min() < 0)
	{
		ui->chartView->curr_xmin = 0;
		axis0->setMin(0);
	}
	if (axis1->max() > ymax)
	{
		ui->chartView->curr_ymax = ymax;
		axis1->setMax(ymax);
	}
	if (axis1->min() < 0)
	{
		ui->chartView->curr_ymin = 0;
		axis1->setMin(0);
	}
	return;
}


/**
 * @brief This slot opens the IO Control dialog.
 * @return none
 */
void MainWindow::on_actionIO_Control_triggered()
{
	DialogIoctrl* dialogIoctrl = new DialogIoctrl(this);
	dialogIoctrl->setAttribute(Qt::WA_DeleteOnClose);
	dialogIoctrl->show();
	return;
}

void MainWindow::on_actionspecial_pixels_triggered()
{
	DialogSpecialPixels* dialogSpecialPixels = new DialogSpecialPixels(this);
	dialogSpecialPixels->setAttribute(Qt::WA_DeleteOnClose);
	connect(ui->horizontalSliderSample, &QSlider::valueChanged, dialogSpecialPixels, &DialogSpecialPixels::updateSample);
	connect(ui->horizontalSliderBlock, &QSlider::valueChanged, dialogSpecialPixels, &DialogSpecialPixels::updateBlock);
	connect(&lsc, &Lsc::allBlocksDone, dialogSpecialPixels, &DialogSpecialPixels::updateValues);
	dialogSpecialPixels->updateSample(ui->horizontalSliderSample->value());
	dialogSpecialPixels->updateBlock(ui->horizontalSliderBlock->value());
	dialogSpecialPixels->show();
	return;
}

/**
 * @brief This slot opens the gray scale viewer. Only on windows.
 * @return none
 */
void MainWindow::on_actionShow_triggered()
{
#ifdef WIN32
	settings.beginGroup("board" + QString::number(greyscale_viewer_board));
	uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toDouble();
	settings.endGroup();
	uint32_t nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	uint32_t block = ui->horizontalSliderBlock->value() - 1;
	lsc.start2dViewer(greyscale_viewer_board, block, greyscale_viewer_camera, pixelcount, nos);
#endif
	return;
}

/**
 * @brief This slot sends the new block to gray scale viewer.
 * @return none
 */
void MainWindow::on_horizontalSliderBlock_valueChanged()
{
#ifdef WIN32
	settings.beginGroup("board" + QString::number(greyscale_viewer_board));
	uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toDouble();
	settings.endGroup();
	uint32_t nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	uint32_t block = ui->horizontalSliderBlock->value() - 1;
	lsc.showNewBitmap(greyscale_viewer_board, block, greyscale_viewer_camera, pixelcount, nos);
#endif
	return;
}

/**
 * @brief This slot opens a save file dialog to export the settings.
 * @return none
 */
void MainWindow::on_actionExport_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Export Settings", "config.ini", tr("config files (*.ini)"));
	QSettings destSettings(fileName, QSettings::IniFormat);
	copySettings(destSettings, settings);
	return;
}

/**
 * @brief This slot opens a load file dialog to import the settings.
 * @return none
 */
void MainWindow::on_actionImport_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Import Settings", nullptr, tr("config files (*.ini);;all files (*)"));
	QSettings srcSettings(fileName, QSettings::IniFormat);
	copySettings(settings, srcSettings);
	return;
}

/**
 * \brief This is a helper function to copy settings.
 * 
 * \param dst
 * \param src
 */
void MainWindow::copySettings(QSettings &dst, QSettings &src)
{
	QStringList keys = src.allKeys();
	for (QStringList::iterator i = keys.begin(); i != keys.end(); i++)
	{
		dst.setValue(*i, src.value(*i));
	}
}

void MainWindow::showCurrentScan()
{
	int64_t sample = 0;
	int64_t block = 0;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			lsc.getCurrentScanNumber(drvno, &sample, &block);
			// This break breaks the for loop after the first drvno is selected by board_sel, because the display only can show one sample at a time.
			break;
		}
	}
	// If the current scan number is one of the first 3 scans in the measurement, abort the displaying.
	// This is here to prevent showing the first over exposed scans when in software polling mode.
	// This also prevents showing a zero line, when no scans have been written to the user buffer yet and getCurrentScanNumber gives -1 or 0 for sample and block.
	if (sample <= 2 && block <= 0)
	{
		if (measurement_cnt > 1)
		{
			ui->horizontalSliderSample->setValue(ui->horizontalSliderSample->maximum());
			ui->horizontalSliderBlock->setValue(ui->horizontalSliderSample->maximum());
		}
		return;
	}
	int radioState = 0;
	if (ui->radioButtonLiveViewOff->isChecked()) radioState = 0;
	else if (ui->radioButtonLiveViewFixedSample->isChecked()) radioState = 1;
	else if (ui->radioButtonLiveViewOffNewestSample->isChecked()) radioState = 2;
	switch (radioState)
	{
	default:
	case 0:
		// live view off: do nothing
		break;
	case 1:
		// fixed sample: show last completed block. The last completed block and not the current block is shown to vsync the picture for area sensors.
		if (sample != settings.value(settingNosPath, settingNosDefault).toDouble() - 1 && block > 0)
			block--;
		ui->horizontalSliderBlock->setValue(static_cast<int32_t>(block + 1));
		break;
	case 2:
		// newest sample: refresh sample and block slider
		ui->horizontalSliderBlock->setValue(static_cast<int32_t>(block + 1));
		ui->horizontalSliderSample->setValue(static_cast<int32_t>(sample + 1));
		break;
	}
	return;
}

void MainWindow::adjustLiveView()
{
	int radioState = 0;
	if (ui->radioButtonLiveViewOff->isChecked()) radioState = 0;
	else if (ui->radioButtonLiveViewFixedSample->isChecked()) radioState = 1;
	else if (ui->radioButtonLiveViewOffNewestSample->isChecked()) radioState = 2;
	if (measureOn)
	{
		switch (radioState)
		{
		case 0:
			//enable controls
			ui->spinBoxBlock->setEnabled(true);
			ui->spinBoxSample->setEnabled(true);
			ui->horizontalSliderBlock->setEnabled(true);
			ui->horizontalSliderSample->setEnabled(true);
			liveViewTimer->stop();
			break;
		case 1:
			//disable only block slider
			ui->spinBoxBlock->setEnabled(false);
			ui->spinBoxSample->setEnabled(true);
			ui->horizontalSliderBlock->setEnabled(false);
			ui->horizontalSliderSample->setEnabled(true);
			liveViewTimer->start(100);
			break;
		case 2:
			//disable controls
			ui->spinBoxBlock->setEnabled(false);
			ui->spinBoxSample->setEnabled(false);
			ui->horizontalSliderBlock->setEnabled(false);
			ui->horizontalSliderSample->setEnabled(false);
			liveViewTimer->start(100);
			break;
		}
	}
	return;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (lsc.IsRunning())
	{
		QErrorMessage* m = new QErrorMessage(this);
		m->setWindowTitle("Error");
		m->showMessage("Stop measurement before closing");
		event->ignore();
	}
	else
	{
		lampsTimer->stop();
		liveViewTimer->stop();
		lsc.abortMeasurement();
		lsc.exitDriver();
		settings.setValue("centralwidget/geometry", saveGeometry());
		settings.setValue("centralwidget/state", saveState());
		QMainWindow::closeEvent(event);
	}
}

/**
 * @brief Slot for the signal pressed of pushButtonStartStop.
 * @return none
 */
void MainWindow::on_pushButtonStartStop_pressed()
{
	if(isRunning)
		lsc.abortMeasurement();
	else
		startPressed();
	return;
}

void MainWindow::on_checkBoxLoopMeasurement_stateChanged(int state)
{
	lsc.setContinuousMeasurement(state);
	return;
}

void MainWindow::on_actionGreyscaleSettings_triggered()
{
#ifdef WIN32
	DialogGreyscaleSettings* dialog = new DialogGreyscaleSettings(this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
	return;
#endif
}

void MainWindow::showStatusCodeDialog(es_status_codes status)
{
	QMessageBox* d = new QMessageBox(this);
	d->setWindowTitle("Error");
	d->setText(ConvertErrorCodeToMsg(status));
	d->setIcon(QMessageBox::Critical);
	d->exec();
	return;
}
