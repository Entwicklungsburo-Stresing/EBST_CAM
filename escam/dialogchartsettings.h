/*****************************************************************//**
 * @file		dialogchartsettings.h
 * @brief		Dialog to set the chart settings.
 * @author		Florian Hahn
 * @date		02.07.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "ui_dialogchartsettings.h"
#include "lsc-gui.h"

class DialogChartSettings : public QDialog
{
	Q_OBJECT

public:
	DialogChartSettings(QWidget *parent = nullptr);
	~DialogChartSettings();

	void on_rubberband_valueChanged();
signals:
	void spinBoxAxes_valueChanged();
private slots:
	void on_buttonBox_rejected();
	void on_spinBoxXmin_valueChanged(int arg1);
	void on_spinBoxXmax_valueChanged(int arg1);
	void on_spinBoxYmin_valueChanged(int arg1);
	void on_spinBoxYmax_valueChanged(int arg1);
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
	void on_checkBoxMirrorX_stateChanged(int state);
	void on_checkBoxShowCrosshair_stateChanged(int state);
#else
	void on_checkBoxMirrorX_checkStateChanged(Qt::CheckState state);
	void on_checkBoxShowCrosshair_checkStateChanged(Qt::CheckState state);
#endif

private:
	Ui::DialogChartSettingsClass ui;
	QSettings settings;
	qreal xmax_old = 0;
	qreal xmin_old = 0;
	qreal ymax_old = 0;
	qreal ymin_old = 0;
};
