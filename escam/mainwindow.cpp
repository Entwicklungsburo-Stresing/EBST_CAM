#include "mainwindow.h"
#include "dialogaxes.h"
#include "../version.h"
#include "dialogdac.h"

/**
 * @brief Constructor of Class MainWindow.
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->horizontalSliderSample, SIGNAL(valueChanged(int)), this, SLOT(loadCameraData()));
    connect(ui->horizontalSliderBlock, SIGNAL(valueChanged(int)), this, SLOT(loadCameraData()));
	connect(ui->pushButtonStartCont, &QPushButton::toggled, this, &MainWindow::startContPressed);
	connect(ui->pushButtonAbort, SIGNAL(pressed()), this, SLOT(abortPressed()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(&lsc, SIGNAL(measureStart()), this, SLOT(on_measureStart()));
    connect(&lsc, SIGNAL(measureDone()), this, SLOT(on_measureDone()));
    connect(&lsc, SIGNAL(blockStart()), this, SLOT(on_blockStart()));
    connect(&lsc, SIGNAL(blockDone()), this, SLOT(on_blockDone()));
	connect(ui->chartView, SIGNAL(rubberBandChanged()),this, SLOT(on_mychartView_rubberBandChanged()));

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
            loadSettings();
    }
}

/**
 * @brief Destructor of class MainWindow.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Sets the data of chartView.
 * @param series Data as QLineSeries.
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
 * @brief This is an overloaded function.
 * @param data Pointer to data.
 * @param length Length of data.
 */
void MainWindow::setChartData(uint16_t* data, uint16_t length, uint16_t numberOfSets)
{
    QLineSeries** series = (QLineSeries**)calloc(numberOfSets, sizeof(QLineSeries*));
    for(uint16_t set=0; set<numberOfSets; set++)
    {
        series[set] = new QLineSeries(this);
		//@flo bitte kommentieren
        for(uint16_t i=0; i<length; i++)
        {
            series[set]->append(i, *(data + i + (length * set)));
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
    //measurement tab
    settings_struct.nos = settings.value(settingNosPath, settingNosDefault).toInt();
    settings_struct.nob = settings.value(settingNobPath, settingNobDefault).toInt();
    settings_struct.sti_mode = settings.value(settingStiPath, settingStiDefault).toInt();
    settings_struct.bti_mode = settings.value(settingBtiPath, settingBtiDefault).toInt();
    settings_struct.stime_in_microsec = settings.value(settingStime_in_microseconds_Path, settingStime_in_microseconds_Default).toDouble();
    settings_struct.btime_in_microsec = settings.value(settingBtime_in_microseconds_Path, settingBtime_in_microseconds_Default).toDouble();
    settings_struct.sdat_in_10ns = settings.value(settingSdat_in_10nsPath, settingSdat_in_10nsDefault).toInt();
    settings_struct.bdat_in_10ns = settings.value(settingBdat_in_10nsPath, settingSdat_in_10nsDefault).toInt();
    settings_struct.sslope = settings.value(settingSslopePath, settingSslopeDefault).toInt();
    settings_struct.bslope = settings.value(settingBslopePath, settingBslopeDefault).toInt();
    settings_struct.xckdelay_in_10ns = settings.value(settingXckdelayIn10nsPath, settingXckdelayIn10nsDefault).toInt();
    settings_struct.ShutterExpTimeIn10ns = settings.value(settingShutterExpTimeIn10nsPath, settingShutterExpTimeIn10nsDefault).toInt();
    settings_struct.trigger_mode_cc = settings.value(settingTriggerCcPath, settingTriggerCcDefault).toInt();
    //camerasetup tab
    settings_struct.sensor_type = settings.value(settingSensorTypePath, settingSensorTypeDefault).toInt();
    settings_struct.camera_system = settings.value(settingCameraSystemPath, settingCameraSystemDefault).toInt();
    settings_struct.camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toInt();
    settings_struct.pixel = settings.value(settingPixelPath, settingPixelDefault).toInt();
    settings_struct.mshut = settings.value(settingMshutPath, settingMshutDefault).toBool();
    settings_struct.led_off = settings.value(settingLedPath, settingLedDefault).toBool();
    settings_struct.gain_switch = settings.value(settingGain3010Path, settingGain3010Default).toBool();
    settings_struct.gain_3030 = settings.value(settingGain3030Path, settingGain3030Default).toInt();
    settings_struct.Temp_level = settings.value(settingCoolingPath, settingCoolingDefault).toInt();
    settings_struct.dac = settings.value(settingDacPath, settingDacDefault).toBool();
    settings_struct.enable_gpx = settings.value(settingGpxPath, settingGpxDefault).toBool();
    settings_struct.gpx_offset = settings.value(settingGpxOffsetPath, settingGpxOffsetDefault).toInt();
    //fftmodes tab
    settings_struct.FFTLines = settings.value(settingLinesPath, settingLinesDefault).toInt();
    settings_struct.Vfreq = settings.value(settingVfreqPath, settingVfreqDefault).toInt();
    settings_struct.FFTMode = settings.value(settingFftModePath, settingFftModeDefault).toInt();
    settings_struct.lines_binning = settings.value(settingLinesBinningPath, settingLinesBinningDefault).toInt();
    settings_struct.number_of_regions = settings.value(settingNumberOfRegionsPath, settingNumberOfRegionsDefault).toInt();
    settings_struct.keep_first = 0;//TODO implement in gui
    if (settings.value(settingRegionSizeEqualPath, settingRegionSizeEqualDefault).toBool() == 0)
        *(settings_struct.region_size) = 0;
    else
        settings_struct.region_size[0] = settings.value(settingRegionSize1Path, settingRegionSize1Default).toInt();
    settings_struct.region_size[1] = settings.value(settingRegionSize2Path, settingRegionSize2Default).toInt();
    settings_struct.region_size[2] = settings.value(settingRegionSize3Path, settingRegionSize3Default).toInt();
    settings_struct.region_size[3] = settings.value(settingRegionSize4Path, settingRegionSize4Default).toInt();
    settings_struct.region_size[4] = settings.value(settingRegionSize5Path, settingRegionSize5Default).toInt();
    settings_struct.region_size[5] = settings.value(settingRegionSize6Path, settingRegionSize6Default).toInt();
    settings_struct.region_size[6] = settings.value(settingRegionSize7Path, settingRegionSize7Default).toInt();
    settings_struct.region_size[7] = settings.value(settingRegionSize8Path, settingRegionSize8Default).toInt();
    settings_struct.dac_output[0][0] = settings.value(settingSensorOffsetChannel1Path, settingSensorOffsetChannel1Default).toInt();
    settings_struct.dac_output[0][1] = settings.value(settingSensorOffsetChannel2Path, settingSensorOffsetChannel2Default).toInt();
    settings_struct.dac_output[0][2] = settings.value(settingSensorOffsetChannel3Path, settingSensorOffsetChannel3Default).toInt();
    settings_struct.dac_output[0][3] = settings.value(settingSensorOffsetChannel4Path, settingSensorOffsetChannel4Default).toInt();
    settings_struct.dac_output[0][4] = settings.value(settingSensorOffsetChannel5Path, settingSensorOffsetChannel5Default).toInt();
    settings_struct.dac_output[0][5] = settings.value(settingSensorOffsetChannel6Path, settingSensorOffsetChannel6Default).toInt();
    settings_struct.dac_output[0][6] = settings.value(settingSensorOffsetChannel7Path, settingSensorOffsetChannel7Default).toInt();
    settings_struct.dac_output[0][7] = settings.value(settingSensorOffsetChannel8Path, settingSensorOffsetChannel8Default).toInt();
	settings_struct.dac_output[1][0] = settings.value(settingSensorOffsetChannel1Path, settingSensorOffsetChannel1Default).toInt();
    settings_struct.dac_output[1][1] = settings.value(settingSensorOffsetChannel2Path, settingSensorOffsetChannel2Default).toInt();
    settings_struct.dac_output[1][2] = settings.value(settingSensorOffsetChannel3Path, settingSensorOffsetChannel3Default).toInt();
    settings_struct.dac_output[1][3] = settings.value(settingSensorOffsetChannel4Path, settingSensorOffsetChannel4Default).toInt();
    settings_struct.dac_output[1][4] = settings.value(settingSensorOffsetChannel5Path, settingSensorOffsetChannel5Default).toInt();
    settings_struct.dac_output[1][5] = settings.value(settingSensorOffsetChannel6Path, settingSensorOffsetChannel6Default).toInt();
    settings_struct.dac_output[1][6] = settings.value(settingSensorOffsetChannel7Path, settingSensorOffsetChannel7Default).toInt();
    settings_struct.dac_output[1][7] = settings.value(settingSensorOffsetChannel8Path, settingSensorOffsetChannel8Default).toInt();
    settings_struct.TORmodus = settings.value(settingTorPath, settingTorDefault).toInt();
    settings_struct.ADC_Mode = settings.value(settingAdcModePath, settingAdcModeDefault).toInt();
    settings_struct.ADC_custom_pattern = settings.value(settingAdcCustomValuePath, settingAdcCustomValueDefault).toInt();
    settings_struct.gpx_offset = 0;//TODO
	settings_struct.bec_in_10ns = 0; //TODO
	settings_struct.isIr = settings.value(settingIsIrPath, settingIsIrDefault).toBool();
	//settings_struct.cont_pause = settings.value(settingContPause, settingAdcCustomValueDefault).toInt();
	settings_struct.board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt() + 1;
	es_status_codes status = lsc.initMeasurement();
	if (status != es_no_error) {
		QErrorMessage* d = new QErrorMessage(this);
		d->setWindowTitle("Error");
		d->setWindowModality(Qt::ApplicationModal);
		d->showMessage(tr((char*)ConvertErrorCodeToMsg(status)));
		return;
	}
	QThread* measurementThread = new QThread;
	//Before assigning lsc to measurementThread first assign lsc to main thread. This only works when it is not assigned to any thread. This is the case when initMeasurement was called before and measurementThread finished. Moving lsc to the main thread is needed because you cannot move this object from nowhere land or another thread to measurementThread.
	lsc.moveToThread(QApplication::instance()->thread());
	lsc.moveToThread(measurementThread);
	connect(measurementThread, SIGNAL(started()), &lsc, SLOT(startMeasurement()));
	connect(&lsc, SIGNAL(measureDone()), measurementThread, SLOT(quit()));
	connect(measurementThread, SIGNAL(finished()), measurementThread, SLOT(deleteLater()));
	measurementThread->start();
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
		settings_struct.cont_pause = 1;// settings.value(settingContPause, settingAdcCustomValueDefault).toInt();
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
	connect( ds, SIGNAL( settings_saved() ), this, SLOT( loadSettings() ) );
	return;
}
/**
 * @brief This slot opens the TDC dialog.
 * @return none
 */
void MainWindow::on_actionTDC_triggered()
{
	//ds->setAttribute( Qt::WA_DeleteOnClose );
	ds_tdc->show();
	//connect( &lsc, SIGNAL( measureDone() ), ds, SLOT( updateTDC() ) );
	return;
}

/**
 * @brief This slot opens the RMS dialog.
 * @return none
 */
void MainWindow::on_actionRMS_triggered()
{
	//DialogRMS* ds = new DialogRMS(this);
	//ds->setAttribute(Qt::WA_DeleteOnClose);
	ds_rms->show();
	//TODO
	//connect(ds, SIGNAL(settings_saved()), this, SLOT(loadSettings()));
	return;
}

/**
 * @brief This slot opens the DSC dialog.
 * @return none
 */
void MainWindow::on_actionDSC_triggered()
{
	//DialogDSC* ds = new DialogDSC(this);
	//ds_dsc->setAttribute(Qt::WA_DeleteOnClose);
	ds_dsc->show();
	//TODO
	//connect(ds, SIGNAL(settings_saved()), this, SLOT(loadSettings()));
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
    for (int i = 0; i < settings.value(settingCamcntPath, settingCamcntDefault).toInt() * used_number_of_boards; i++)
    {
        QCheckBox* checkbox = new QCheckBox(messageBox);
        checkbox->setText("Camera "+QString::number(i+1));
        checkbox->setChecked(settings.value(settingShowCameraBaseDir + QString::number(i), settingShowCameraDefault).toBool());
        layout->addWidget(checkbox);
        connect(checkbox, &QCheckBox::stateChanged, this, [checkbox, this, i] {on_checkBoxShowCamera_stateChanged(checkbox->isChecked(), i); loadCameraData(); });
    }
    QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, messageBox);
    connect(dialogButtonBox, SIGNAL(accepted()), messageBox, SLOT(accept()));
    layout->addWidget(dialogButtonBox);
    messageBox->setWindowTitle("Cameras");
    messageBox->show();
    return;
}

void MainWindow::on_checkBoxShowCamera_stateChanged(bool state, int camera)
{
    settings.setValue(settingShowCameraBaseDir + QString::number(camera), state);
    return;
}

void MainWindow::on_actionReset_axes_triggered()
{
	// retrieve axis pointer
	QList<QAbstractAxis *> axes = ui->chartView->chart()->axes();
    if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	ui->chartView->curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toReal();
	ui->chartView->curr_xmin = 0;
	ui->chartView->curr_ymax = 0xFFFF;
	ui->chartView->curr_ymin = 0;
	axis0->setMax(settings.value(settingPixelPath, settingPixelDefault).toReal());
	axis0->setMin(0);
	axis1->setMax(0xFFFF);
	axis1->setMin(0);
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
    int nos = settings.value(settingNosPath,settingNosDefault).toInt();
    ui->horizontalSliderSample->setMaximum(nos);
    ui->spinBoxSample->setMaximum(nos);
    int nob = settings.value(settingNobPath,settingNobPath).toInt();
    ui->horizontalSliderBlock->setMaximum(nob);
    ui->spinBoxBlock->setMaximum(nob);
    int tor = settings.value(settingTorPath,settingTorDefault).toInt();
	for(uint32_t drvno=1; drvno<=number_of_boards; drvno++)
		lsc.setTorOut(drvno, tor);
    int board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt();
    if(board_sel == 2)
        used_number_of_boards = 2;
    else
        used_number_of_boards = 1;
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
    d->open(this,SLOT(close()));
    return;
}

void MainWindow::showPcieBoardError()
{
    QMessageBox* d = new QMessageBox(this);
    d->setWindowTitle("Fatal error");
    d->setWindowModality(Qt::ApplicationModal);
    d->setText("Error while opening PCIe board.");
    d->setIcon(QMessageBox::Critical);
    d->open(this,SLOT(close()));
    return;
}

void MainWindow::on_actionDump_board_registers_triggered()
{
    QDialog* messageBox = new QDialog(this);
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout* layout = new QVBoxLayout(messageBox);
    messageBox->setLayout(layout);
    QTabWidget* tabWidget = new QTabWidget(messageBox);
    tabWidget->setDocumentMode(true);
	for (uint32_t drvno = 1; drvno <= number_of_boards; drvno++)
	{
		QLabel* labelS0 = new QLabel(tabWidget);
		labelS0->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelS0->setTextFormat(Qt::RichText);
		labelS0->setText(QString::fromStdString(lsc._dumpS0Registers(drvno)));
		labelS0->setAlignment(Qt::AlignTop);
		tabWidget->addTab(labelS0, "S0 registers board " + QString::number(drvno));
		QLabel* labelDma = new QLabel(tabWidget);
		labelDma->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelDma->setTextFormat(Qt::RichText);
		labelDma->setText(QString::fromStdString(lsc._dumpDmaRegisters(drvno)));
		labelDma->setAlignment(Qt::AlignTop);
		tabWidget->addTab(labelDma, "DMA registers board " + QString::number(drvno));
		QLabel* labelTlp = new QLabel(tabWidget);
		labelTlp->setTextInteractionFlags(Qt::TextSelectableByMouse);
		labelTlp->setTextFormat(Qt::RichText);
		labelTlp->setText(QString::fromStdString(lsc._dumpTlp(drvno)));
		labelTlp->setAlignment(Qt::AlignTop);
		tabWidget->addTab(labelTlp, "TLP size board" + QString::number(drvno));
	}
    QLabel* labelSettings = new QLabel(tabWidget);
    labelSettings->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelSettings->setTextFormat(Qt::RichText);
    labelSettings->setText(QString::fromStdString(lsc._dumpGlobalSettings()));
    labelSettings->setAlignment(Qt::AlignTop);
    tabWidget->addTab(labelSettings, "Settings");
    layout->addWidget(tabWidget);
    QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok, tabWidget);
    connect(dialogButtonBox, SIGNAL(accepted()), messageBox, SLOT(accept()));
    layout->addWidget(dialogButtonBox);
    messageBox->setWindowTitle("Register dump");
    messageBox->show();
    return;
}

void MainWindow::loadCameraData()
{
    uint32_t pixel = settings.value(settingPixelPath,settingPixelDefault).toInt();
	// camcnt is the count of all cameras
    uint32_t camcnt = settings.value(settingCamcntPath,settingCamcntDefault).toInt();
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toInt() + 1;
	// showCamcnt is the count of all cameras to be shown on the chart
	// = sum of all true settingShowCameraBaseDir settings
    uint32_t showCamcnt = 0;
	for (uint32_t drvno = 1; drvno <= number_of_boards; drvno++)
	{
		if((board_sel == 1 || board_sel == 3) && drvno == 1 || (board_sel == 2 || board_sel == 3) && drvno == 2)
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				int currCam = cam + ((drvno - 1) * camcnt);
				bool showCurrentCam = settings.value(settingShowCameraBaseDir + QString::number(currCam), settingShowCameraDefault).toBool();
					if (showCurrentCam)
						showCamcnt++;
			}
	}
    uint16_t* data = (uint16_t*)malloc(pixel * showCamcnt * sizeof(uint16_t));
    int block = ui->horizontalSliderBlock->value() - 1;
    int sample = ui->horizontalSliderSample->value() - 1;
	// showedCam counts the number of cameras which are shown on the chart
    uint32_t showedCam = 0;
	for (uint32_t drvno = 1; drvno <= number_of_boards; drvno++)
	{
		if ((board_sel == 1 || board_sel == 3) && drvno == 1 || (board_sel == 2 || board_sel == 3) && drvno == 2)
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				int currCam = cam + ((drvno - 1) * camcnt);
				bool showCurrentCam = settings.value(settingShowCameraBaseDir + QString::number(currCam), settingShowCameraDefault).toBool();
				if (showCurrentCam)
				{
					lsc.returnFrame(drvno, sample, block, cam, data + showedCam * pixel, pixel);
					showedCam++;
				}
			}
	}
    setChartData(data, pixel, showCamcnt);
	//send pxel 6 and 7 to the tdc window
	//pixel 6low/7high of tdc1 and 8low/9high of tdc2 to tdc view
	ds_tdc->updateTDC( *(uint32_t*)(data + 6), *(uint32_t*)(data + 8) );
	ds_dsc->updateDSC();
	ds_rms->updateRMS();

    free(data);
    return;
}

void MainWindow::on_measureStart()
{
    //set measureOn lamp on
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::green);
    ui->widgetMeasureOn->setPalette(pal);
    //disable start button
	ui->pushButtonStart->setDisabled(true);
	//enable abort button
    ui->pushButtonAbort->setEnabled(true);
    //enable controls
    ui->spinBoxBlock->setEnabled(true);
    ui->spinBoxSample->setEnabled(true);
    ui->horizontalSliderBlock->setEnabled(true);
    ui->horizontalSliderSample->setEnabled(true);
	//enable chart menu
	ui->menuChart->setEnabled(true);
    return;
}

void MainWindow::on_measureDone()
{
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
	for(uint32_t drvno=1; drvno<=number_of_boards; drvno++)
		lsc.abortMeasurement(drvno);
    return;
}

void MainWindow::on_mychartView_rubberBandChanged()
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
	// apply bounderies on axes
	if(axis0->max() > settings.value(settingPixelPath, settingPixelDefault).toInt())
	{
		ui->chartView->curr_xmax = settings.value(settingPixelPath, settingPixelDefault).toInt();
		axis0->setMax(ui->chartView->curr_xmax);
	}
	if(axis0->min() < 0)
	{
		ui->chartView->curr_xmin = 0;
		axis0->setMin(0);
	}
	if(axis1->max() > 0xFFFF)
	{
		ui->chartView->curr_ymax = 0xFFFF;
		axis1->setMax(0xFFFF);
	}
	if(axis1->min() < 0)
	{
		ui->chartView->curr_ymin = 0;
		axis1->setMin(0);
	}
}