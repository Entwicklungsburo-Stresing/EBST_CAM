#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "ui_mainwindow.h"
#include "lsc.h"
#include "dialogsettings.h"
#include "dialogrms.h"
#include "dialogdsc.h"

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
	Ui::MainWindow *ui;
public slots:
	void loadCameraData();
	void on_measureStart();
	void on_measureDone();
	void on_blockStart();
	void on_blockDone();
private:
	QSettings settings;
	void setChartData(QLineSeries** series, uint16_t numberOfSets);
	void setChartData(uint16_t* data, uint32_t* length, uint16_t numberOfSets);
	void showNoDriverFoundDialog();
	void showPcieBoardError();
	DialogDSC* ds_dsc = new DialogDSC( this );
	DialogRMS* ds_rms = new DialogRMS( this );
	QThread measurementThread;
	void copySettings(QSettings &dst, QSettings &src);
	QTimer* displayTimer = new QTimer(this);
	bool measureOn = false;
	void closeEvent(QCloseEvent *event);
private slots:
	void on_actionEdit_triggered();
	void on_actionRMS_triggered();
	void on_actionDSC_triggered();
	void on_actionAxes_triggered();
	void on_actionCameras_triggered();
	void on_actionReset_axes_triggered();
	void setDefaultAxes();
	void on_actionAbout_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actionDAC_triggered();
	void on_actionIO_Control_triggered();
	void on_actionShow_triggered();
	void loadSettings();
	void startPressed();
	void startContPressed(bool checked);
	void on_actionDump_board_registers_triggered();
	void abortPressed();
	void on_rubberBandChanged();
	void on_checkBoxShowCamera(bool state, int camera, uint32_t drvno);
	void on_pushButtonStart_pressed();
	void on_horizontalSliderBlock_valueChanged();
	void on_actionExport_triggered();
	void on_actionImport_triggered();
	void on_actionGamma_triggered();
	void showCurrentScan();
	void adjustLiveView();
	void on_actionVerify_data_file_triggered();
	void on_actionspecial_pixels_triggered();
};

#endif // MAINWINDOW_H
