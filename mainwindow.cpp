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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setChartData(QLineSeries* series)
{
    QChart *chart = ui->chartView->chart();
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
