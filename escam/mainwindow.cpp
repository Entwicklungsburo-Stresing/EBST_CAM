#include "mainwindow.h"
#include "dialogaxes.h"
#include "../version.h"
#include "dialogdac.h"
#include "dialogioctrl.h"
#ifdef WIN32
#include "dialoggamma.h"
#include "shared_src/ESLSCDLL_pro.h"
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
	connect(ui->pushButtonStartCont, &QPushButton::toggled, this, &MainWindow::startContPressed);
	connect(ui->pushButtonAbort, &QPushButton::pressed, this, &MainWindow::abortPressed);
	connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
	connect(&lsc, &Lsc::measureStart, this, &MainWindow::on_measureStart);
	connect(&lsc, &Lsc::measureDone, this, &MainWindow::on_measureDone);
	connect(&lsc, &Lsc::blockStart, this, &MainWindow::on_blockStart);
	connect(&lsc, &Lsc::blockDone, this, &MainWindow::on_blockDone);
	connect(ui->chartView, &MyQChartView::rubberBandChanged, this, &MainWindow::on_rubberBandChanged);
	connect(displayTimer, &QTimer::timeout, this, &MainWindow::showCurrentScan);
	connect(ui->radioButtonLiveViewFixedSample, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);
	connect(ui->radioButtonLiveViewOff, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);
	connect(ui->radioButtonLiveViewOffNewestSample, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);


	es_status_codes status = lsc.initDriver();
	if (status != es_no_error)
	{
		showNoDriverFoundDialog();
	}
	else
	{
		status = lsc.initPcieBoard();
		if (status != es_no_error)
			showPcieBoardError();
		else
		{
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
		}
	}

	// move lsc to its own thread
	lsc.moveToThread(&measurementThread);
	connect(&measurementThread, &QThread::started, &lsc, &Lsc::startMeasurement);
	connect(&lsc, &Lsc::measureDone, &measurementThread, &QThread::quit);
	connect(&lsc, &Lsc::measureDone, ds_dsc, &DialogDSC::updateDSC);
	connect(&lsc, &Lsc::measureDone, ds_rms, &DialogRMS::updateRMS);
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
		chart->addSeries(series[set]);
	chart->createDefaultAxes();
	QList<QAbstractAxis *> axes = ui->chartView->chart()->axes();
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
void MainWindow::setChartData(uint16_t* data, uint32_t* length, uint16_t numberOfSets)
{
	// Allocate memory for the pointer array to the QlineSeries.
	QLineSeries** series = static_cast<QLineSeries**>(calloc(numberOfSets, sizeof(QLineSeries*)));
	// Iterate through all data sets.
	uint16_t* cur_data_ptr = data;
	for(uint16_t set=0; set<numberOfSets; set++)
	{
		// Set the current data set to a new empty QLineSeries.
		series[set] = new QLineSeries(this);
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
 * @brief Slot for the signal pressed of pushButtonStart.
 * @return none
 */
void MainWindow::on_pushButtonStart_pressed()
{
	ui->pushButtonStartCont->setDisabled(true);
	startPressed();
	return;
}

/**
 * @brief Slot to start measurement. Called by on_pushButtonStart_pressed and startContPressed.
 * @return none
 */
void MainWindow::startPressed()
{
	settings_struct.board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	settings_struct.cont_pause_in_microseconds = settings.value(settingContinuousPauseInMicrosecondsPath, settingContinuousPausInMicrosecondsDefault).toUInt();
	settings_struct.contiuous_measurement = ui->pushButtonStartCont->isChecked();
	settings_struct.nos = settings.value(settingNosPath, settingNosDefault).toUInt();
	settings_struct.nob = settings.value(settingNobPath, settingNobDefault).toUInt();
	//camerasetup tab
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		settings.beginGroup("board" + QString::number(drvno));
		//measurement tab
		settings_struct.camera_settings[drvno].sti_mode = settings.value(settingStiPath, settingStiDefault).toUInt();
		settings_struct.camera_settings[drvno].bti_mode = settings.value(settingBtiPath, settingBtiDefault).toUInt();
		settings_struct.camera_settings[drvno].stime_in_microsec = settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toUInt();
		settings_struct.camera_settings[drvno].btime_in_microsec = settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toUInt();
		settings_struct.camera_settings[drvno].sdat_in_10ns = settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toUInt();
		settings_struct.camera_settings[drvno].bdat_in_10ns = settings.value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toUInt();
		settings_struct.camera_settings[drvno].sslope = settings.value(settingSslopePath, settingSslopeDefault).toUInt();
		settings_struct.camera_settings[drvno].bslope = settings.value(settingBslopePath, settingBslopeDefault).toUInt();
		settings_struct.camera_settings[drvno].xckdelay_in_10ns = settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toUInt();
		settings_struct.camera_settings[drvno].sec_in_10ns = settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toUInt();
		settings_struct.camera_settings[drvno].bec_in_10ns = settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toUInt();
		settings_struct.camera_settings[drvno].trigger_mode_cc = settings.value(settingTriggerCcPath, settingTriggerCcDefault).toUInt();
		settings_struct.camera_settings[drvno].sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toUInt();
		settings_struct.camera_settings[drvno].camera_system = settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt();
		settings_struct.camera_settings[drvno].camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
		settings_struct.camera_settings[drvno].pixel = settings.value(settingPixelPath, settingPixelDefault).toUInt();
		settings_struct.camera_settings[drvno].mshut = settings.value(settingMshutPath, settingMshutDefault).toBool();
		settings_struct.camera_settings[drvno].led_off = settings.value(settingLedPath, settingLedDefault).toBool();
		settings_struct.camera_settings[drvno].sensor_gain = settings.value(settingSensorGainPath, settingSensorGainDefault).toUInt();
		settings_struct.camera_settings[drvno].adc_gain = settings.value(settingAdcGainPath, settingAdcGainDefault).toUInt();
		settings_struct.camera_settings[drvno].temp_level = settings.value(settingCoolingPath, settingCoolingDefault).toUInt();
		settings_struct.camera_settings[drvno].gpx_offset = settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toUInt();
		settings_struct.camera_settings[drvno].is_hs_ir = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
		settings_struct.camera_settings[drvno].ioctrl_impact_start_pixel = settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toUInt();
		settings_struct.camera_settings[drvno].use_software_polling = settings.value(settingsUseSoftwarePollingPath, settingsUseSoftwarePollingDefault).toBool();
		settings_struct.camera_settings[drvno].shortrs = settings.value(settingShortrsPath, settingShortrsDefault).toBool();
		settings_struct.camera_settings[drvno].is_cooled_cam = settings.value(settingIsCooledCamPath, settingIsCooledCamDefault).toBool();
		//fftmodes tab
		settings_struct.camera_settings[drvno].fft_lines = settings.value(settingLinesPath, settingLinesDefault).toUInt();
		settings_struct.camera_settings[drvno].vfreq = settings.value(settingVfreqPath, settingVfreqDefault).toUInt();
		settings_struct.camera_settings[drvno].fft_mode = settings.value(settingFftModePath, settingFftModeDefault).toUInt();
		settings_struct.camera_settings[drvno].lines_binning = settings.value(settingLinesBinningPath, settingLinesBinningDefault).toUInt();
		settings_struct.camera_settings[drvno].number_of_regions = settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toUInt();
		settings_struct.camera_settings[drvno].keep = settingKeepDefault;
		if (settings.value(settingRegionSizeEqualPath, settingRegionSizeEqualDefault).toBool() == true)
			*(settings_struct.camera_settings[drvno].region_size) = 0;
		else
		{
			settings_struct.camera_settings[drvno].region_size[0] = settings.value(settingRegionSize1Path, settingRegionSize1Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[1] = settings.value(settingRegionSize2Path, settingRegionSize2Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[2] = settings.value(settingRegionSize3Path, settingRegionSize3Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[3] = settings.value(settingRegionSize4Path, settingRegionSize4Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[4] = settings.value(settingRegionSize5Path, settingRegionSize5Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[5] = settings.value(settingRegionSize6Path, settingRegionSize6Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[6] = settings.value(settingRegionSize7Path, settingRegionSize7Default).toUInt();
			settings_struct.camera_settings[drvno].region_size[7] = settings.value(settingRegionSize8Path, settingRegionSize8Default).toUInt();
		}
		//export data tab
		settings_struct.camera_settings[drvno].write_to_disc = settings.value(settingWriteDataToDiscPath, settingWriteToDiscDefault).toBool();
		QByteArray array = settings.value(settingFilePathPath, QDir::currentPath()).toString().toLocal8Bit();
		strcpy(settings_struct.camera_settings[drvno].file_path, array.data());
		//dac
		settings_struct.camera_settings[drvno].dac_output[0] = settings.value(settingDacCameraChannel1Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[1] = settings.value(settingDacCameraChannel2Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[2] = settings.value(settingDacCameraChannel3Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[3] = settings.value(settingDacCameraChannel4Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[4] = settings.value(settingDacCameraChannel5Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[5] = settings.value(settingDacCameraChannel6Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[6] = settings.value(settingDacCameraChannel7Path, settingDacCameraDefault).toUInt();
		settings_struct.camera_settings[drvno].dac_output[7] = settings.value(settingDacCameraChannel8Path, settingDacCameraDefault).toUInt();
		//debug
		settings_struct.camera_settings[drvno].tor = settings.value(settingTorPath, settingTorDefault).toUInt();
		settings_struct.camera_settings[drvno].adc_mode = settings.value(settingAdcModePath, settingAdcModeDefault).toUInt();
		settings_struct.camera_settings[drvno].adc_custom_pattern = settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toUInt();
		settings_struct.camera_settings[drvno].gpx_offset = 0;//TODO

		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[0] = settings.value(settingIOCtrlOutput1DelayIn5nsPath, settingIOCtrlOutput1DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[1] = settings.value(settingIOCtrlOutput2DelayIn5nsPath, settingIOCtrlOutput2DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[2] = settings.value(settingIOCtrlOutput3DelayIn5nsPath, settingIOCtrlOutput3DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[3] = settings.value(settingIOCtrlOutput4DelayIn5nsPath, settingIOCtrlOutput4DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[4] = settings.value(settingIOCtrlOutput5DelayIn5nsPath, settingIOCtrlOutput5DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[5] = settings.value(settingIOCtrlOutput6DelayIn5nsPath, settingIOCtrlOutput6DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_delay_in_5ns[6] = settings.value(settingIOCtrlOutput7DelayIn5nsPath, settingIOCtrlOutput7DelayIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[0] = settings.value(settingIOCtrlOutput1WidthIn5nsPath, settingIOCtrlOutput1WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[1] = settings.value(settingIOCtrlOutput2WidthIn5nsPath, settingIOCtrlOutput2WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[2] = settings.value(settingIOCtrlOutput3WidthIn5nsPath, settingIOCtrlOutput3WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[3] = settings.value(settingIOCtrlOutput4WidthIn5nsPath, settingIOCtrlOutput4WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[4] = settings.value(settingIOCtrlOutput5WidthIn5nsPath, settingIOCtrlOutput5WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[5] = settings.value(settingIOCtrlOutput6WidthIn5nsPath, settingIOCtrlOutput6WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_output_width_in_5ns[6] = settings.value(settingIOCtrlOutput7WidthIn5nsPath, settingIOCtrlOutput7WidthIn5nsDefault).toUInt();
		settings_struct.camera_settings[drvno].ioctrl_T0_period_in_10ns = settings.value(settingIOCtrlT0PeriodIn10nsPath, settingIOCtrlT0PeriodIn10nsDefault).toUInt();
		settings.endGroup();
	}

	es_status_codes status = lsc.initMeasurement();
	if (status == es_camera_not_found)
	{
		QMessageBox* d = new QMessageBox(this);
		d->setWindowTitle("Camera not found");
		d->setText("No Camera found. Do you want to use dummy data?");
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
	else if (status != es_no_error)
	{
		QErrorMessage* d = new QErrorMessage(this);
		d->setWindowTitle("Error");
		d->setWindowModality(Qt::ApplicationModal);
		d->showMessage(tr(ConvertErrorCodeToMsg(status)));
	}
	else
		measurementThread.start();
	return;
}
/**
 * @brief Slot for the signal pressed of pushButtonStartCont.
 * @return none
 */
void MainWindow::startContPressed(bool checked)
{
	if (checked)
	{
		continiousMeasurementFlag = true;
		ui->pushButtonStartCont->setText("Stop continuous");
		startPressed();
	}
	else
	{
		continiousMeasurementFlag = false;
		ui->pushButtonStartCont->setText("Start continuous");
	}
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
 * @brief This slot opens the TDC dialog.
 * @return none
 */
void MainWindow::on_actionTDC_triggered()
{
	ds_tdc->initDialogTdc();
	ds_tdc->show();
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
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	uint32_t camcnt = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		settings.beginGroup("board" + QString::number(drvno));
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
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
	// retrieve axis pointer
	QList<QAbstractAxis *> axes = ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			ui->chartView->curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toReal();
			ui->chartView->curr_xmin = 0;
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt() == 2)
				ui->chartView->curr_ymax = 0x3FFF;
			else
				ui->chartView->curr_ymax = 0xFFFF;
			ui->chartView->curr_ymin = 0;
			axis0->setMax(settings.value(settingPixelPath, settingPixelDefault).toReal());
			axis0->setMin(0);
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt() == 2)
				axis1->setMax(0x3FFF);
			else
				axis1->setMax(0xFFFF);
			axis1->setMin(0);
			settings.endGroup();
		}
	}
	return;
}

void MainWindow::on_actionAbout_triggered()
{
	QString aboutText = "This is line scan camera version ";
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
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint8_t tor = static_cast<uint8_t>(settings.value(settingTorPath, settingTorDefault).toUInt());
			settings.endGroup();
			lsc.setTorOut(drvno, tor);
			int nos = settings.value(settingNosPath, settingNosDefault).toUInt();
			ui->horizontalSliderSample->setMaximum(nos);
			ui->spinBoxSample->setMaximum(nos);
			int nob = settings.value(settingNobPath, settingNobDefault).toUInt();
			ui->horizontalSliderBlock->setMaximum(nob);
			ui->spinBoxBlock->setMaximum(nob);
		}
	}
	int theme = settings.value(settingThemePath,settingThemeDefault).toInt();
	switch(theme)
	{
	default:
	case 0:
		qApp->setStyleSheet("");
		ui->chartView->chart()->setTheme(QChart::ChartThemeLight);
		break;
	case 1:
		QFile f(":qdarkstyle/style.qss");
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
		ui->chartView->chart()->setTheme(QChart::ChartThemeDark);
		break;
	}
	return;
}

void MainWindow::showNoDriverFoundDialog()
{
	QMessageBox* d = new QMessageBox(this);
	d->setWindowTitle("Fatal error");
	d->setWindowModality(Qt::ApplicationModal);
	d->setText("Driver or PCIe board not found.");
	d->setIcon(QMessageBox::Critical);
	d->setDetailedText(QString::fromStdString(lsc.driverInstructions));
	d->open(this, SLOT(close()));
	return;
}

void MainWindow::showPcieBoardError()
{
	QMessageBox* d = new QMessageBox(this);
	d->setWindowTitle("Fatal error");
	d->setWindowModality(Qt::ApplicationModal);
	d->setText("Error while opening PCIe board.");
	d->setIcon(QMessageBox::Critical);
	d->open(this, SLOT(close()));
	return;
}

void MainWindow::on_actionDump_board_registers_triggered()
{
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
	return;
}

void MainWindow::loadCameraData()
{
	uint32_t pixel = 0;
	// Save pixel values in an array for the case when there are different pixel numbers on different PCIe boards.
	uint32_t pixel_array[MAXPCIECARDS];
	// camcnt is the count of all cameras
	uint32_t camcnt = 0;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
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
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
			// if camcnt is 0, treat as 1
			if (camcnt == 0)
				camcnt = 1;
			pixel = settings.value(settingPixelPath, settingPixelDefault).toUInt();
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
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
			// if camcnt is 0, treat as 1
			if (camcnt == 0)
				camcnt = 1;
			pixel = settings.value(settingPixelPath, settingPixelDefault).toUInt();
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				bool showCurrentCam = settings.value(settingShowCameraBaseDir + QString::number(cam), settingShowCameraDefault).toBool();
				if (showCurrentCam)
				{
					lsc.returnFrame(drvno, sample, block, cam, cur_data_ptr, pixel);
					pixel_array[showedCam] = pixel;
					cur_data_ptr += pixel;
					showedCam++;
				}
			}
			settings.endGroup();
		}
	}
	setChartData(data, pixel_array, static_cast<uint16_t>(showCamcnt));
	ds_tdc->updateTDC();
	free(data);
	return;
}

void MainWindow::on_measureStart()
{
	measureOn = true;
	//set measureOn lamp on
	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::green);
	ui->widgetMeasureOn->setPalette(pal);
	//disable start button
	ui->pushButtonStart->setDisabled(true);
	//enable abort button
	ui->pushButtonAbort->setEnabled(true);
	adjustLiveView();
	return;
}

void MainWindow::on_measureDone()
{
	measureOn = false;
	//set measureOn lamp off
	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::darkGreen);
	ui->widgetMeasureOn->setPalette(pal);
	//display camera data
	loadCameraData();
	//enable start button
	ui->pushButtonStart->setEnabled(true);
	ui->pushButtonStartCont->setEnabled(true);
	//disable abort button
	ui->pushButtonAbort->setDisabled(true);
#ifdef WIN32
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toUInt();
			settings.endGroup();
			uint32_t nos = settings.value(settingNosPath, settingNosDefault).toUInt();
			uint32_t block = ui->horizontalSliderBlock->value() - 1;
			DLLShowNewBitmap_drvno(drvno, block, 0, pixelcount, nos);
		}
	}
#endif
	displayTimer->stop();
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
	pal.setColor(QPalette::Background, Qt::green);
	ui->widgetBlockOn->setPalette(pal);
	return;
}

void MainWindow::on_blockDone()
{
	//set blockOn lamp off
	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::darkGreen);
	ui->widgetBlockOn->setPalette(pal);
	return;
}

void MainWindow::abortPressed()
{
	lsc.abortMeasurement();
	return;
}

void MainWindow::on_rubberBandChanged()
{
	// retrieve axis pointer
	QList<QAbstractAxis *> axes = ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	// save current axis configuration
	ui->chartView->curr_xmax = axis0->max();
	ui->chartView->curr_xmin = axis0->min();
	ui->chartView->curr_ymax = axis1->max();
	ui->chartView->curr_ymin = axis1->min();
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	qreal ymax = 0;
	uint max_pixel = 0;
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint cur_pixel = settings.value(settingPixelPath, settingPixelDefault).toUInt();
			if (max_pixel < cur_pixel)
				max_pixel = cur_pixel;
			qreal cur_ymax;
			if (settings.value(settingCameraSystemPath, settingCameraSystemDefault).toUInt() == 2)
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
		ui->chartView->curr_xmax = max_pixel;
		axis0->setMax(ui->chartView->curr_xmax);
	}
	if (axis0->min() < 0)
	{
		ui->chartView->curr_xmin = 0;
		axis0->setMin(0);
	}
	if(axis1->max() > ymax)
	{
		ui->chartView->curr_ymax = ymax;
		axis1->setMax(ymax);
	}
	if(axis1->min() < 0)
	{
		ui->chartView->curr_ymin = 0;
		axis1->setMin(0);
	}
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

/**
 * @brief This slot opens the gray scale viewer. Only on windows.
 * @return none
 */
void MainWindow::on_actionShow_triggered()
{
#ifdef WIN32
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toUInt();
			settings.endGroup();
			uint32_t nos = settings.value(settingNosPath, settingNosDefault).toUInt();
			uint32_t block = ui->horizontalSliderBlock->value() - 1;
			DLLStart2dViewer_drvno(drvno, 0, block, pixelcount, nos);
		}
	}
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
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			settings.beginGroup("board" + QString::number(drvno));
			uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toUInt();
			settings.endGroup();
			uint32_t nos = settings.value(settingNosPath, settingNosDefault).toUInt();
			uint32_t block = ui->horizontalSliderBlock->value() - 1;
			DLLShowNewBitmap_drvno(drvno, block, 0, pixelcount, nos);
		}
	}
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

/**
 * @brief This slot opens the gamma dialog.
 * @return none
 */
void MainWindow::on_actionGamma_triggered()
{
#ifdef WIN32
	DialogGamma* dialogGamma = new DialogGamma(this);
	dialogGamma->setAttribute(Qt::WA_DeleteOnClose);
	dialogGamma->show();
#endif
	return;
}

void MainWindow::showCurrentScan()
{
	int64_t sample = 0;
	int64_t block = 0;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	for (uint32_t drvno = 0; drvno < number_of_boards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			lsc.getCurrentScanNumber(drvno, &sample, &block);
		}
	}
	int radioState = 0;
	if (ui->radioButtonLiveViewOff->isChecked()) radioState = 0;
	else if (ui->radioButtonLiveViewFixedSample->isChecked()) radioState = 1;
	else if (ui->radioButtonLiveViewOffNewestSample->isChecked()) radioState = 2;
	switch (radioState)
	{
	case 2:
		ui->horizontalSliderSample->setValue(static_cast<int32_t>(sample + 1));
	// This fall through from case 2 to case 1 is intended
	case 1:
		ui->horizontalSliderBlock->setValue(static_cast<int32_t>(block + 1));
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
			displayTimer->stop();
			break;
		case 1:
			//disable only block slider
			ui->spinBoxBlock->setEnabled(false);
			ui->spinBoxSample->setEnabled(true);
			ui->horizontalSliderBlock->setEnabled(false);
			ui->horizontalSliderSample->setEnabled(true);
			displayTimer->start(100);
			break;
		case 2:
			//disable controls
			ui->spinBoxBlock->setEnabled(false);
			ui->spinBoxSample->setEnabled(false);
			ui->horizontalSliderBlock->setEnabled(false);
			ui->horizontalSliderSample->setEnabled(false);
			displayTimer->start(100);
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
		lsc.abortMeasurement();
		ExitDriver();
		QMainWindow::closeEvent(event);
	}
}
