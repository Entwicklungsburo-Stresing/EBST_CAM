#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "ui_mainwindow.h"
#include "lsc.h"
#include "dialogsettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Lsc lsc;
public slots:
    void loadCameraData();
    void on_measureStart();
    void on_measureDone();
    void on_blockStart();
    void on_blockDone();
private:
    Ui::MainWindow *ui;
    QSettings settings;
    void setChartData(QLineSeries** series, uint16_t numberOfSets);
    void setChartData(uint16_t* data, uint16_t length, uint16_t numberOfSets);
    void showNoDriverFoundDialog();
    void showPcieBoardError();
private slots:
    void on_actionEdit_triggered();
    void loadSettings();
    void startPressed();
    void on_actionDump_board_registers_triggered();
    void abortPressed();
};

#endif // MAINWINDOW_H
