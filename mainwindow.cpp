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

    connect(ui->horizontalSliderSample, SIGNAL(valueChanged(int)), this, SLOT(sampleChanged(int)));
    connect(ui->horizontalSliderBlock, SIGNAL(valueChanged(int)), this, SLOT(blockChanged(int)));
    connect(ui->pushButtonStart, SIGNAL(pressed()), this, SLOT(startPressed()));

    this->loadSettings();
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
    this->setChartData(series);
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
    this->lsc.returnFrame(0,sample,ui->horizontalSliderBlock->value(),0,data,576);
    this->setChartData(data,576);
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
    this->lsc.returnFrame(0,ui->horizontalSliderSample->value(),block,0,data,576);
    this->setChartData(data,576);
    return;
}

/**
 * @brief Slot for the signal pressed of pushButtonStart.
 * @return none
 */
void MainWindow::startPressed()
{
    this->lsc.initMeasurement();
    this->lsc.startMeasurement();
    uint16_t data[576];
    this->lsc.returnFrame(0,ui->horizontalSliderSample->value(),ui->horizontalSliderBlock->value(),0,data,576);
    this->setChartData(data,576);
    return;
}

/**
 * @brief This slot opens the settings dialog.
 * @return none
 */
void MainWindow::on_actionEdit_triggered()
{
    DialogSettings* ds = new DialogSettings(&this->settings);
    ds->show();
    connect(ds, SIGNAL(accepted()), this, SLOT(loadSettings()));
    return;
}

void MainWindow::loadSettings()
{
    int nos = this->settings.value(SETTING_NOS,NOS_DEFAULT).toInt();
    ui->horizontalSliderSample->setMaximum(nos);
    ui->spinBoxSample->setMaximum(nos);
    int nob = this->settings.value(SETTING_NOB,NOB_DEFAULT).toInt();
    ui->horizontalSliderBlock->setMaximum(nob);
    ui->spinBoxBlock->setMaximum(nob);
    return;
}
