#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "lsc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, Lsc
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setChartData(QLineSeries* series);
    void setChartData(uint16_t* data, uint16_t length);
public slots:
    void sampleChanged(int);
    void blockChanged(int);
    void startPressed();
private:
    Ui::MainWindow *ui;
signals:

};

#endif // MAINWINDOW_H
