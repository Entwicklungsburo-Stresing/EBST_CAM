/*****************************************************************//**
 * @file		camerasettingswidget.h
 * @brief		Widget used to set camera settings.
 * @details		This widget will be displayed as many times as there are PCIe boards. It is used in dialogsettings.
 * @author		Florian Hahn
 * @date		06.02.2023
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QWidget>
#include <QSettings>
#include "ui_camerasettingswidget.h"
#include "dialogsettings.h"
#include <QDir>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class CameraSettingsWidgetClass; }
QT_END_NAMESPACE

class CameraSettingsWidget : public QWidget
{
	Q_OBJECT
public:
	CameraSettingsWidget(QWidget *parent = nullptr);
	~CameraSettingsWidget();
	uint32_t drvno = 0;
	DialogSettings* ds;
public slots:
	void on_accepted();
	void loadDefaults();
	void changeSettingsLevel(int settings_level);
	void initializeWidget();
private:
	Ui::CameraSettingsWidgetClass *ui;
	QSettings settings;
	int _settings_level = 0;
private slots:
	void on_comboBoxSti_currentIndexChanged(int index);
	void on_comboBoxBti_currentIndexChanged(int index);
	void on_comboBoxSensorType_currentIndexChanged(int index);
	void on_comboBoxCameraSystem_currentIndexChanged(int index);
	void on_checkBoxRegionsEqual_stateChanged(int arg1);
	void on_spinBoxPixel_valueChanged(int arg1);
	void on_comboBoxFftMode_currentIndexChanged(int index);
	void on_pushButtonFilePath_clicked();
	void on_checkBoxWriteDataToDisc_stateChanged(int arg1);
	void on_spinBoxNumberOfRegions_valueChanged(int value);
	void on_spinBoxLines_valueChanged(int value);
	void on_buttonGroupTimerResolution_idClicked(int id);
};
