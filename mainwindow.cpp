#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QChart *chart = ui->chartView->chart();
    chart->legend()->hide();
    chart->setTitle("Camera 1");
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    connect(ui->horizontalSliderSample, QOverload<int>::of(&QSlider::valueChanged), this, &MainWindow::sampleChanged);
    connect(ui->horizontalSliderBlock, QOverload<int>::of(&QSlider::valueChanged), this, &MainWindow::blockChanged);
    connect(ui->pushButton, QOverload<>::of(&QPushButton::pressed), this, &MainWindow::startPressed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setChartData(QLineSeries* series)
{
    QChart *chart = ui->chartView->chart();
    chart->removeAllSeries();
    chart->addSeries(series);
    chart->createDefaultAxes();
    return;
}

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

void MainWindow::sampleChanged(int sample)
{
    uint16_t data[576];
    this->returnFrame(0,sample,ui->horizontalSliderBlock->value(),0,data,576);
    this->setChartData(data,576);
    return;
}

void MainWindow::blockChanged(int block)
{
    uint16_t data[576];
    this->returnFrame(0,ui->horizontalSliderSample->value(),block,0,data,576);
    this->setChartData(data,576);
    return;
}

void MainWindow::startPressed()
{
    this->initMeasurement();
    this->startMeasurement();
    uint16_t data[576];
    this->returnFrame(0,0,0,0,data,576);
    this->setChartData(data,576);
    return;
}
