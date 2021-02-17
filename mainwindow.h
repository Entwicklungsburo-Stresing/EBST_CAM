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
public slots:
    void loadCameraData();
private:
    Ui::MainWindow *ui;
    Lsc lsc;
    QSettings settings;
    void setChartData(QLineSeries* series);
    void setChartData(uint16_t* data, uint16_t length);
    void showNoDriverFoundDialog();
    void showPcieBoardError();
private slots:
    void on_actionEdit_triggered();
    void loadSettings();
    void startPressed();
    void on_actionDump_board_registers_triggered();
};

#endif // MAINWINDOW_H
