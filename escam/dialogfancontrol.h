/*****************************************************************//**
 * @file		dialogfancontrol.h
 * @brief		Dialog to control the fan.
 * @author		Dennis Vollenweider
 * @date		27.10.2025
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "ui_dialogfancontrol.h"
#include "lsc-gui.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogFanControlClass; };
QT_END_NAMESPACE

class DialogFanControl : public QDialog
{
	Q_OBJECT

public:
	DialogFanControl(QWidget *parent = Q_NULLPTR);
	~DialogFanControl();

private slots:
	void on_spinBoxBoard_valueChanged(int index);
	void on_comboBoxMonitor_currentIndexChanged(int index);
	void on_checkBoxFanOn_stateChanged(int state);

private:
	Ui::DialogFanControlClass *ui;
	QSettings settings;
	void initDialogFanControl();
	void updateFanCheckboxState(int monitorIndex);
};

