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
	uint32_t greyscale_viewer_board = 0;
	uint32_t greyscale_viewer_camera = 0;
public slots:
	void loadCameraData();
	void on_measureStart();
	void on_measureDone();
	void on_allBlocksDone();
	void startPressed();
private:
	QSettings settings;
	QThread measurementThread;
	void copySettings(QSettings &dst, QSettings &src);
	QTimer* liveViewTimer = new QTimer(this);
	QTimer* lampsTimer = new QTimer(this);
	QTimer* scanFrequencyTimer = new QTimer(this);
	QTimer* blockFrequencyTimer = new QTimer(this);
	bool measureOn = false;
	void closeEvent(QCloseEvent *event);
	void showStatusCodeDialog(es_status_codes status);
private slots:
	void on_actionEdit_triggered();
	void on_actionRMS_triggered();
	void on_actionDSC_triggered();
	void on_actionTrigger_info_triggered();
	void on_actionCameras_triggered();
	void on_actionReset_axes_triggered();
	void on_actionContext_help_triggered();
	void on_actionAbout_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actionDAC_triggered();
	void on_actionIO_Control_triggered();
	void on_actionShow_triggered();
	void loadSettings();
	void on_actionDump_board_registers_triggered();
	void on_checkBoxShowCamera(bool state, int camera, uint32_t drvno);
	void on_pushButtonStartStop_pressed();
	void on_horizontalSliderBlock_valueChanged();
	void on_actionExport_triggered();
	void on_actionImport_triggered();
	void showCurrentScan();
	void adjustLiveView();
	void on_actionVerify_data_file_triggered();
	void on_actionExport_data_triggered();
	void on_actionspecial_pixels_triggered();
	void on_actionGreyscaleSettings_triggered();
	void on_checkBoxLoopMeasurement_stateChanged(int state);
	void readScanFrequencyBit();
	void on_scanFrequencyTooHigh();
	void readBlockFrequencyBit();
	void on_blockFrequencyTooHigh();
	void findCamera();
	void setScanTriggerDetected();
	void setBlockTriggerDetected();
	void on_readCameraTemp();
	void on_actionChartSettings_triggered();
	void setBlockOnLamp();
};

#endif // MAINWINDOW_H
