/*****************************************************************//**
 * @file   mainwindow.cpp
 * @copydoc mainwindow.h
 *********************************************************************/

#include "mainwindow.h"
#include "../version.h"
#include "dialogdac.h"
#include "dialogioctrl.h"
#include "dialogspecialpixels.h"
#include "dialogtriggerinfo.h"
#include "dialogchartsettings.h"
#include "dialogshutter.h"
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
	connect(&lsc, &Lsc::allBlocksDone, this, &MainWindow::on_allBlocksDone);
	connect(liveViewTimer, &QTimer::timeout, this, &MainWindow::showCurrentScan);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::readScanFrequencyBit);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::readBlockFrequencyBit);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::findCamera);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::on_readCameraTemp);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::setBlockOnLamp);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::setScanTriggerDetected);
	connect(lampsTimer, &QTimer::timeout, this, &MainWindow::setBlockTriggerDetected);
	connect(scanFrequencyTimer, &QTimer::timeout, this, &MainWindow::on_scanFrequencyTooHigh);
	connect(blockFrequencyTimer, &QTimer::timeout, this, &MainWindow::on_blockFrequencyTooHigh);
	connect(ui->radioButtonLiveViewFixedSample, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);
	connect(ui->radioButtonLiveViewOff, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);
	connect(ui->radioButtonLiveViewOffNewestSample, &QRadioButton::toggled, this, &MainWindow::adjustLiveView);


	es_status_codes status = lsc.initDriver();
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
	ui->chartView->setDefaultAxes();

	lampsTimer->start(100);
	restoreGeometry(settings.value("centralwidget/geometry").toByteArray());
	restoreState(settings.value("centralwidget/state").toByteArray());

	// move lsc to its own thread
	lsc.moveToThread(&measurementThread);
	connect(&measurementThread, &QThread::started, &lsc, &Lsc::startMeasurement);
	connect(&lsc, &Lsc::measureDone, &measurementThread, &QThread::quit);
#ifdef __linux__
	// disable gray scale menu on Linux
	ui->menuGreyscale_Viewer->setEnabled(false);
#endif
	// disable axes menu until first finish of measurement to avoid crash
	ui->actionAxes->setEnabled(false);
	// Hide the verify data file action in release mode
#ifndef _DEBUG
	ui->actionVerify_data_file->setVisible(false);
	ui->actionVerify_data_file->setEnabled(false);
#endif
}

/**
 * @brief Destructor of class MainWindow.
 */
MainWindow::~MainWindow()
{
	delete ui;
}

/**
 * @brief Slot to start measurement. Called by on_pushButtonStartStop_pressed.
 * @return none
 */
void MainWindow::startPressed()
{
	// The order is the same as in struct.h
	struct measurement_settings library_settings;
	library_settings.board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	library_settings.nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	library_settings.nob = settings.value(settingNobPath, settingNobDefault).toDouble();
	library_settings.continuous_measurement = ui->checkBoxLoopMeasurement->isChecked();
	library_settings.cont_pause_in_microseconds = settings.value(settingContinuousPauseInMicrosecondsPath, settingContinuousPausInMicrosecondsDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		settings.beginGroup("board" + QString::number(drvno));

		library_settings.camera_settings[drvno].use_software_polling = settings.value(settingsUseSoftwarePollingPath, settingsUseSoftwarePollingDefault).toBool();
		library_settings.camera_settings[drvno].sti_mode = settings.value(settingStiPath, settingStiDefault).toDouble();
		library_settings.camera_settings[drvno].bti_mode = settings.value(settingBtiPath, settingBtiDefault).toDouble();
		library_settings.camera_settings[drvno].stime = settings.value(settingStimePath, settingStime_Default).toDouble();
		library_settings.camera_settings[drvno].btime_in_microsec = settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble();
		library_settings.camera_settings[drvno].sdat_in_10ns = settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toDouble();
		library_settings.camera_settings[drvno].bdat_in_10ns = settings.value(settingBdat_in_10nsPath, settingBdat_in_10nsDefault).toDouble();
		library_settings.camera_settings[drvno].sslope = settings.value(settingSslopePath, settingSslopeDefault).toDouble();
		library_settings.camera_settings[drvno].bslope = settings.value(settingBslopePath, settingBslopeDefault).toDouble();
		library_settings.camera_settings[drvno].xckdelay_in_10ns = settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toDouble();
		library_settings.camera_settings[drvno].sec_in_10ns = settings.value(settingShutterSecIn10nsPath, settingShutterSecIn10nsDefault).toDouble();
		library_settings.camera_settings[drvno].trigger_mode_integrator = settings.value(settingTriggerModeIntegratorPath, settingTriggerModeIntegratorDefault).toDouble();
		library_settings.camera_settings[drvno].sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toDouble();
		library_settings.camera_settings[drvno].camera_system = settings.value(settingCameraSystemPath, settingCameraSystemDefault).toDouble();
		library_settings.camera_settings[drvno].camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
		library_settings.camera_settings[drvno].pixel = settings.value(settingPixelPath, settingPixelDefault).toDouble();
		library_settings.camera_settings[drvno].is_fft_legacy = settings.value(settingIsFftLegacyPath, settingIsFftlegacyDefault).toBool();
		library_settings.camera_settings[drvno].led_off = settings.value(settingLedPath, settingLedDefault).toBool();
		library_settings.camera_settings[drvno].sensor_gain = settings.value(settingSensorGainPath, settingSensorGainDefault).toDouble();
		library_settings.camera_settings[drvno].adc_gain = settings.value(settingAdcGainPath, settingAdcGainDefault).toDouble();
		library_settings.camera_settings[drvno].temp_level = settings.value(settingCoolingPath, settingCoolingDefault).toDouble();
		library_settings.camera_settings[drvno].bticnt = settings.value(settingBticntPath, settingBticntDefault).toDouble();
		library_settings.camera_settings[drvno].gpx_offset = settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toDouble();
		library_settings.camera_settings[drvno].fft_lines = settings.value(settingLinesPath, settingLinesDefault).toDouble();
		library_settings.camera_settings[drvno].vfreq = settings.value(settingVfreqPath, settingVfreqDefault).toDouble();
		library_settings.camera_settings[drvno].fft_mode = settings.value(settingFftModePath, settingFftModeDefault).toDouble();
		library_settings.camera_settings[drvno].lines_binning = settingLinesBinningDefault;
		library_settings.camera_settings[drvno].number_of_regions = settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toDouble();
		library_settings.camera_settings[drvno].s1s2_read_delay_in_10ns = settings.value(settingS1S2ReadDelayIn10nsPath, settingS1S2ReadDelayIn10nsDefault).toDouble();
		library_settings.camera_settings[drvno].region_size[0] = settings.value(settingRegionSize1Path, settingRegionSize1Default).toDouble();
		library_settings.camera_settings[drvno].region_size[1] = settings.value(settingRegionSize2Path, settingRegionSize2Default).toDouble();
		library_settings.camera_settings[drvno].region_size[2] = settings.value(settingRegionSize3Path, settingRegionSize3Default).toDouble();
		library_settings.camera_settings[drvno].region_size[3] = settings.value(settingRegionSize4Path, settingRegionSize4Default).toDouble();
		library_settings.camera_settings[drvno].region_size[4] = settings.value(settingRegionSize5Path, settingRegionSize5Default).toDouble();
		library_settings.camera_settings[drvno].region_size[5] = settings.value(settingRegionSize6Path, settingRegionSize6Default).toDouble();
		library_settings.camera_settings[drvno].region_size[6] = settings.value(settingRegionSize7Path, settingRegionSize7Default).toDouble();
		library_settings.camera_settings[drvno].region_size[7] = settings.value(settingRegionSize8Path, settingRegionSize8Default).toDouble();
		for (int camera = 0; camera < MAXCAMCNT; camera++)
			for (int channel = 0; channel < DACCOUNT; channel++)
				library_settings.camera_settings[drvno].dac_output[camera][channel] = settings.value(settingDacCameraChannelBaseDir + QString::number(channel + 1) + "Pos" + QString::number(camera), settingDacCameraDefault).toDouble();
		library_settings.camera_settings[drvno].tor = settings.value(settingTorPath, settingTorDefault).toDouble();
		library_settings.camera_settings[drvno].adc_mode = settings.value(settingAdcModePath, settingAdcModeDefault).toDouble();
		library_settings.camera_settings[drvno].adc_custom_pattern = settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toDouble();
		library_settings.camera_settings[drvno].bec_in_10ns = settings.value(settingShutterBecIn10nsPath, settingShutterBecIn10nsDefault).toDouble();
		library_settings.camera_settings[drvno].channel_select = settings.value(settingChannelSelectPath, settingChannelSelectDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_impact_start_pixel = settings.value(settingIOCtrlImpactStartPixelPath, settingIOCtrlImpactStartPixelDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[0] = settings.value(settingIOCtrlOutput1WidthIn5nsPath, settingIOCtrlOutput1WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[1] = settings.value(settingIOCtrlOutput2WidthIn5nsPath, settingIOCtrlOutput2WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[2] = settings.value(settingIOCtrlOutput3WidthIn5nsPath, settingIOCtrlOutput3WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[3] = settings.value(settingIOCtrlOutput4WidthIn5nsPath, settingIOCtrlOutput4WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[4] = settings.value(settingIOCtrlOutput5WidthIn5nsPath, settingIOCtrlOutput5WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[5] = settings.value(settingIOCtrlOutput6WidthIn5nsPath, settingIOCtrlOutput6WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_width_in_5ns[6] = settings.value(settingIOCtrlOutput7WidthIn5nsPath, settingIOCtrlOutput7WidthIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[0] = settings.value(settingIOCtrlOutput1DelayIn5nsPath, settingIOCtrlOutput1DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[1] = settings.value(settingIOCtrlOutput2DelayIn5nsPath, settingIOCtrlOutput2DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[2] = settings.value(settingIOCtrlOutput3DelayIn5nsPath, settingIOCtrlOutput3DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[3] = settings.value(settingIOCtrlOutput4DelayIn5nsPath, settingIOCtrlOutput4DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[4] = settings.value(settingIOCtrlOutput5DelayIn5nsPath, settingIOCtrlOutput5DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[5] = settings.value(settingIOCtrlOutput6DelayIn5nsPath, settingIOCtrlOutput6DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_output_delay_in_5ns[6] = settings.value(settingIOCtrlOutput7DelayIn5nsPath, settingIOCtrlOutput7DelayIn5nsDefault).toDouble();
		library_settings.camera_settings[drvno].ioctrl_T0_period_in_10ns = settings.value(settingIOCtrlT0PeriodIn10nsPath, settingIOCtrlT0PeriodIn10nsDefault).toDouble();
		library_settings.camera_settings[drvno].dma_buffer_size_in_scans = 1000;
		library_settings.camera_settings[drvno].tocnt = settings.value(settingTocntPath, settingTocntDefault).toDouble();
		library_settings.camera_settings[drvno].sticnt = settings.value(settingSticntPath, settingSticntDefault).toDouble();
		library_settings.camera_settings[drvno].sensor_reset_or_hsir_ec = settings.value(settingSensorResetOrHsirEcPath, settingSensorResetOrHsIrDefault).toDouble();
		library_settings.camera_settings[drvno].write_to_disc = settings.value(settingWriteDataToDiscPath, settingWriteToDiscDefault).toBool();
		QByteArray array = settings.value(settingFilePathPath, QDir::currentPath()).toString().toLocal8Bit();
#ifdef WIN32
		strcpy_s(library_settings.camera_settings[drvno].file_path, file_path_size, array.data());
#else
		strcpy(library_settings.camera_settings[drvno].file_path, array.data());
#endif
		library_settings.camera_settings[drvno].shift_s1s2_to_next_scan = settings.value(settingShiftS1S2ToNextScanPath, settingShiftS1S2ToNextScanDefault).toBool();
		library_settings.camera_settings[drvno].is_cooled_camera_legacy_mode = settings.value(settingIsCooledCameraLegacyModePath, settingIsCooledCameraLegacyModeDefault).toBool();
		library_settings.camera_settings[drvno].monitor = settings.value(settingMonitorPath, settingMonitorDefault).toDouble();
		library_settings.camera_settings[drvno].manipulate_data_mode = settings.value(settingManipulateDataModePath, settingManipulateDataModeDefault).toDouble();
		library_settings.camera_settings[drvno].manipulate_data_custom_factor = settings.value(settingManipulateDataCustomFactorPath, settingManipulateDataCustomFactorDefault).toDouble();
		library_settings.camera_settings[drvno].ec_legacy_mode = settings.value(settingEcLegacyModePath, settingEcLegacyModeDefault).toBool();
		library_settings.camera_settings[drvno].stime_resolution_mode = settings.value(settingStimeResolutionModePath, settingStimeResolutionModeDefault).toDouble();
		settings.endGroup();
	}
	es_status_codes status = lsc.initMeasurement(library_settings);
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
			lsc.fillUserBufferWithDummyData();
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
	connect( ds, &DialogSettings::settings_saved, ui->chartView, &MyQChartView::setDefaultAxes );
	return;
}

/**
 * @brief This slot opens the verify data file dialog.
 */
void MainWindow::on_actionVerify_data_file_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Verify data", nullptr, "binary files (*.bin);;all files (*)");
	if (fileName.isEmpty()) return;
	struct verify_data_parameter vd;
#ifdef WIN32
	strcpy_s(vd.filename_full, file_filename_full_size, fileName.toStdString().c_str());
#else
	strcpy(vd.filename_full, fileName.toStdString().c_str());
#endif
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
	QString path = QFileDialog::getSaveFileName(this, "Export data", "measurement.h5", "HDF5 files(*.h5);; binary files (*.bin)", nullptr, QFileDialog::ShowDirsOnly);
	if (path.isEmpty()) return;
	QByteArray fileName_byteArray = path.toLatin1();
	const char* fileName_char = fileName_byteArray.data();
	es_status_codes status = mainWindow->lsc.SaveMeasurementDataToFile(fileName_char);

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
 * @brief This slot imports measurement data from a HDF5 file or a binary file.
 * @return none
 */
void MainWindow::on_actionImport_data_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Import data", "", "binary files (*.bin)"); // HDF5 files(*.h5) not yet allowed, because it isn't implemented yet
	if (fileName.isEmpty()) return;
	QByteArray fileName_byteArray = fileName.toLocal8Bit();
	const char* fileName_char = fileName_byteArray.data();
	es_status_codes status = mainWindow->lsc.importMeasurementDataFromFile(fileName_char);
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
	messageBox->setWindowTitle("Import data");
	if (status != es_no_error)
	{
		QString errorMsg = QString::fromStdString("Error importing data:\n");
		errorMsg.append(QString::fromStdString(ConvertErrorCodeToMsg(status)));
		labelExport->setText(errorMsg);
		messageBox->show();
		return;
	}
	labelExport->setText(QString::fromStdString("Data imported successfully."));
	messageBox->show();
	emit lsc.measureStart();
	emit lsc.blockStart();
	emit lsc.blockDone();
	emit lsc.allBlocksDone();
	emit lsc.measureDone();
	return;
}

/**
 * @brief This slot opens the RMS dialog.
 * @return none
 */
void MainWindow::on_actionRMS_triggered()
{
	DialogRMS* ds_rms = new DialogRMS(this);
	connect(&lsc, &Lsc::allBlocksDone, ds_rms, &DialogRMS::updateRMS);
	ds_rms->setAttribute(Qt::WA_DeleteOnClose);
	ds_rms->initDialogRMS();
	ds_rms->show();
	return;
}

/**
 * @brief This slot opens the shutter dialog.
 * @return none
 */
void MainWindow::on_actionShutter_triggered()
{
	DialogShutter* ds = new DialogShutter(this);
	ds->setAttribute(Qt::WA_DeleteOnClose);
	ds->show();
	return;
}

/**
 * @brief This slot opens the Trigger info dialog.
 * @return none
 */
void MainWindow::on_actionTrigger_info_triggered()
{
	DialogTriggerInfo* dti = new DialogTriggerInfo(this);
	connect(&lsc, &Lsc::measureDone, dti, &DialogTriggerInfo::on_measureDone);
	dti->setAttribute(Qt::WA_DeleteOnClose);
	dti->show();
	return;
}

/**
 * @brief This slot opens the DSC dialog.
 * @return none
 */
void MainWindow::on_actionDSC_triggered()
{
	DialogDSC* ds_dsc = new DialogDSC(this);
	connect(&lsc, &Lsc::allBlocksDone, ds_dsc, &DialogDSC::updateDSC);
	ds_dsc->setAttribute(Qt::WA_DeleteOnClose);
	ds_dsc->initDialogDsc();
	ds_dsc->show();
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
				connect(checkbox, &QCheckBox::stateChanged, this, [checkbox, this, cam, drvno] {on_checkBoxShowCamera(checkbox->isChecked(), cam, drvno); loadCameraData(); });
#else
				connect(checkbox, &QCheckBox::checkStateChanged, this, [checkbox, this, cam, drvno] {on_checkBoxShowCamera(checkbox->isChecked(), cam, drvno); loadCameraData(); });
#endif
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
	ui->chartView->setDefaultAxes();
}

void MainWindow::on_actionContext_help_triggered()
{
	QWhatsThis::enterWhatsThisMode();
}

void MainWindow::on_actionAbout_triggered()
{
	QString aboutText = "This is Escam version ";
	aboutText.append(VER_FILE_VERSION_STR"\n");
	aboutText.append(VER_COPYRIGHT_STR"\n");
	aboutText.append("This software is released under the LPGL-3.0\n");
	aboutText.append("https://stresing.de");
	QMessageBox::about(this, "About Escam", aboutText);
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
	qDebug() << "Loading settings...";
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool coolingOn = false;
	bool isOvertempCam = false;
	bool showManipulateDataWarning = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
			if (settings.value(settingManipulateDataModePath, settingManipulateDataModeDefault).toDouble() != manipulate_data_mode_none)
				showManipulateDataWarning = true;
			settings.endGroup();
			lsc.setTorOut(drvno, tor);
		}
	}
	if (showManipulateDataWarning)
		statusBar()->showMessage("Data manipulation is enabled.");
	else
		statusBar()->clearMessage();
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
	if (nos < ui->horizontalSliderSample->value())
		ui->horizontalSliderSample->setValue(nos);
	int nob = settings.value(settingNobPath, settingNobDefault).toDouble();
	ui->horizontalSliderBlock->setMaximum(nob);
	ui->spinBoxBlock->setMaximum(nob);
	if(nob < ui->horizontalSliderBlock->value())
		ui->horizontalSliderBlock->setValue(nob);
	QString theme = settings.value(settingThemePath, settingThemeDefault).toString();
	QApplication::setStyle(QStyleFactory::create(theme));
	QStyleHints* qstyle = QApplication::styleHints();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
	qstyle->setColorScheme(Qt::ColorScheme(settings.value(settingColorSchemePath, settingColorSchemeDefault).toDouble()));
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
	if(qstyle->colorScheme() == Qt::ColorScheme::Dark)
		ui->chartView->chart()->setTheme(QChart::ChartThemeDark);
	else
		ui->chartView->chart()->setTheme(QChart::ChartThemeLight);
#else
	(void)qstyle;
#endif

	QGuiApplication::restoreOverrideCursor();
	qDebug() << "Loading settings done";
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
	uint16_t* camera_data = static_cast<uint16_t*>(malloc(data_array_size * sizeof(uint16_t)));
	uint16_t* cur_data_ptr = camera_data;
	uint32_t block = static_cast<uint32_t>(ui->horizontalSliderBlock->value() - 1);
	uint32_t sample = static_cast<uint32_t>(ui->horizontalSliderSample->value() - 1);
	// showedCam counts the number of cameras which are shown on the chart
	uint32_t showedCam = 0;
	QList<QString> lineSeriesNamesList;
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
					es_status_codes status = lsc.copyOneSample(drvno, sample, block, cam, cur_data_ptr);
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
		ui->chartView->setChartData(camera_data, pixel_array, static_cast<uint16_t>(showCamcnt), lineSeriesNamesList);

	// Deactivate legend, because it is blinking. Activate it, when a solution is found
	//if (showedCam > 1)
	//	ui->chartView->chart()->legend()->setVisible(true);
	//else
	//	ui->chartView->chart()->legend()->setVisible(false);
	ui->chartView->updateLabelMouseCoordinates(ui->chartView->mapFromGlobal(QCursor::pos()));
	free(camera_data);
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
		for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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

void MainWindow::setBlockOnLamp()
{
	bool block_on_all_boards = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool block_on = false;
			lsc.getBlockOn(drvno, &block_on);
			block_on_all_boards |= block_on;
		}
	}
	if (block_on_all_boards)
	{
		//set blockOn lamp on
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::green);
		ui->widgetBlockOn->setPalette(pal);
	}
	else
	{
		//set blockOn lamp off
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::darkGreen);
		ui->widgetBlockOn->setPalette(pal);
	}
	return;
}

void MainWindow::on_allBlocksDone()
{
	//display camera data
	loadCameraData();
#ifdef WIN32
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool freqTooHigh = false;
			lsc.readScanFrequencyBit(drvno, &freqTooHigh);
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool freqTooHigh = false;
			lsc.readBlockFrequencyBit(drvno, &freqTooHigh);
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
	if (lsc.numberOfBoards)
		allCamerasFound = true;
	else
		allCamerasFound = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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

void MainWindow::setScanTriggerDetected()
{
	bool allScanTriggerDetected;
	if (lsc.numberOfBoards)
		allScanTriggerDetected = true;
	else
		allScanTriggerDetected = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool scanTriggerDetected = false;
			lsc.getScanTriggerDetected(drvno, &scanTriggerDetected);
			allScanTriggerDetected &= scanTriggerDetected;
		}
	}
	if (allScanTriggerDetected)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::green);
		ui->widgetScanTriggerDetected->setPalette(pal);
		for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
			// Check if the drvno'th bit is set
			if ((board_sel >> drvno) & 1)
				lsc.resetScanTriggerDetected(drvno);
	}
	else
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::darkGreen);
		ui->widgetScanTriggerDetected->setPalette(pal);
	}
	return;
}

void MainWindow::setBlockTriggerDetected()
{
	bool allBlockTriggerDetected;
	if (lsc.numberOfBoards)
		allBlockTriggerDetected = true;
	else
		allBlockTriggerDetected = false;
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		// Check if the drvno'th bit is set
		if ((board_sel >> drvno) & 1)
		{
			bool blockTriggerDetected = false;
			lsc.getBlockTriggerDetected(drvno, &blockTriggerDetected);
			allBlockTriggerDetected &= blockTriggerDetected;
		}
	}
	if (allBlockTriggerDetected)
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::green);
		ui->widgetBlockTriggerDetected->setPalette(pal);
		for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
			// Check if the drvno'th bit is set
			if ((board_sel >> drvno) & 1)
				lsc.resetBlockTriggerDetected(drvno);
	}
	else
	{
		QPalette pal = palette();
		pal.setColor(QPalette::Window, Qt::darkGreen);
		ui->widgetBlockTriggerDetected->setPalette(pal);
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
	{
		// Check if the drvno'th bis is set
		if ((board_sel >> drvno) & 1)
		{
			lsc.getCurrentScanNumber(drvno, &sample, &block);
			if (sample >= 0 && lsc.getVirtualCamcnt(drvno) > 0)
			{
				bool cameraBoardOvertemp = false;
				bool cameraBoardCooled = false;
				for (uint16_t camera_pos = 0; camera_pos < lsc.getVirtualCamcnt(drvno); camera_pos++)
				{
					bool cameraOvertemp = false;
					lsc.getCameraStatusOverTemp(drvno, static_cast<uint32_t>(sample), static_cast<uint32_t>(block), camera_pos, &cameraOvertemp);
					cameraBoardOvertemp |= cameraOvertemp;
					bool cameraCooled = false;
					lsc.getCameraStatusTempGood(drvno, static_cast<uint32_t>(sample), static_cast<uint32_t>(block), camera_pos, &cameraCooled);
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
 * @brief This is a helper function to copy settings.
 * 
 * @param dst
 * @param src
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
	for (uint32_t drvno = 0; drvno < lsc.numberOfBoards; drvno++)
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
	// 06/24: Commented out these lines. This might be a good idea when nos and/or nob are big numbers but leads to problems, when nos and nob are small.
	//if (sample <= 2 && block <= 0)
	//{
	//	if (measurement_cnt > 1)
	//	{
	//		ui->horizontalSliderSample->setValue(ui->horizontalSliderSample->maximum());
	//		ui->horizontalSliderBlock->setValue(ui->horizontalSliderBlock->maximum());
	//	}
	//	return;
	//}
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
	if(lsc.IsRunning())
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

void MainWindow::on_actionChartSettings_triggered()
{
	DialogChartSettings* dialog = new DialogChartSettings(this);
	connect(ui->chartView, &MyQChartView::rubberBandChanged, dialog, &DialogChartSettings::on_rubberband_valueChanged);
	connect(ui->actionReset_axes, &QAction::triggered, dialog, &DialogChartSettings::on_rubberband_valueChanged);
	connect(dialog, &DialogChartSettings::spinBoxAxes_valueChanged, ui->chartView, &MyQChartView::on_axes_changed);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
	return;
}
