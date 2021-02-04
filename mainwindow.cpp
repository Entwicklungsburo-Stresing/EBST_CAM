#include "mainwindow.h"

/**
 * @brief Constructor of Class MainWindow.
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QChart *chart = ui->chartView->chart();
    chart->legend()->hide();
    chart->setTitle("Camera 1");
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    if(!lsc.initDriver()) showNoDriverFoundDialog();
    if(lsc.initPcieBoard() < 0) showPcieBoardError();

    connect(ui->horizontalSliderSample, SIGNAL(valueChanged(int)), this, SLOT(sampleChanged(int)));
    connect(ui->horizontalSliderBlock, SIGNAL(valueChanged(int)), this, SLOT(blockChanged(int)));
    connect(ui->pushButtonStart, SIGNAL(pressed()), this, SLOT(startPressed()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    loadSettings();
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
void MainWindow::setChartData(QLineSeries* series)
{
    QChart *chart = ui->chartView->chart();
    chart->removeAllSeries();
    chart->addSeries(series);
    chart->createDefaultAxes();
    return;
}

/**
 * @brief This is an overloaded function.
 * @param data Pointer to data.
 * @param length Length of data.
 */
void MainWindow::setChartData(uint16_t* data, uint16_t length)
{
    QLineSeries* series = new QLineSeries();
    for(uint16_t i=0; i<length; i++)
    {
        series->append(i, *(data+i));
    }
    setChartData(series);
    return;
}

/**
 * @brief Slot for the signal valueChanged of horizontalSliderSample.
 * @param sample Current value of slider.
 */
void MainWindow::sampleChanged(int sample)
{
    uint16_t data[576];
    sample--;
    int block = ui->horizontalSliderBlock->value() - 1;
    lsc.returnFrame(0,sample,block,0,data,576);
    setChartData(data,576);
    return;
}


/**
 * @brief Slot for the signal valueChanged of horizontalSliderBlock.
 * @param block Current value of slider.
 */
void MainWindow::blockChanged(int block)
{
    uint16_t data[576];
    block--;
    int sample = ui->horizontalSliderSample->value() - 1 ;
    lsc.returnFrame(0,sample,block,0,data,576);
    setChartData(data,576);
    return;
}

/**
 * @brief Slot for the signal pressed of pushButtonStart.
 * @return none
 */
void MainWindow::startPressed()
{
    lsc.initMeasurement();
    lsc.startMeasurement();
    uint16_t data[576];
    int block = ui->horizontalSliderBlock->value() - 1;
    int sample = ui->horizontalSliderSample->value() - 1 ;
    lsc.returnFrame(0,sample,block,0,data,576);
    setChartData(data,576);
    return;
}

/**
 * @brief This slot opens the settings dialog.
 * @return none
 */
void MainWindow::on_actionEdit_triggered()
{
    DialogSettings* ds = new DialogSettings(&settings);
    ds->show();
    connect(ds, SIGNAL(accepted()), this, SLOT(loadSettings()));
    return;
}

/**
 * @brief Load settings and apply to UI.
 */
void MainWindow::loadSettings()
{
    int nos = settings.value(SETTING_NOS,NOS_DEFAULT).toInt();
    ui->horizontalSliderSample->setMaximum(nos);
    ui->spinBoxSample->setMaximum(nos);
    int nob = settings.value(SETTING_NOB,NOB_DEFAULT).toInt();
    ui->horizontalSliderBlock->setMaximum(nob);
    ui->spinBoxBlock->setMaximum(nob);
    return;
}

void MainWindow::showNoDriverFoundDialog()
{
    QMessageBox* d = new QMessageBox;
    d->setWindowTitle("Fatal error");
    d->setWindowModality(Qt::ApplicationModal);
    d->setText("Driver or PCIe board not found.");
    d->setIcon(QMessageBox::Critical);
    d->setDetailedText(lsc.driverInstructions);
    d->open(this,SLOT(close()));
    return;
}

void MainWindow::showPcieBoardError()
{
    QMessageBox* d = new QMessageBox;
    d->setWindowTitle("Fatal error");
    d->setWindowModality(Qt::ApplicationModal);
    d->setText("Error while opening PCIe board.");
    d->setIcon(QMessageBox::Critical);
    d->open(this,SLOT(close()));
    return;
}
