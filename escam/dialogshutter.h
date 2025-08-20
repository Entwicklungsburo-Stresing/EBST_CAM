/*****************************************************************//**
 * @file		dialogshutter.h
 * @brief		Dialog for controlling the shutters.
 * @author		Florian
 * @date		06.03.2025
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "ui_dialogshutter.h"
#include "lsc-gui.h"

class DialogShutter : public QDialog
{
	Q_OBJECT

public:
	DialogShutter(QWidget *parent = nullptr);
	~DialogShutter();
private slots:
	void on_checkBoxMshut_checkStateChanged(Qt::CheckState checkState);
	void on_checkBoxShutterX_checkStateChanged();
	void on_buttonBox_rejected();
	void loadSavedValues();
private:
	Ui::DialogShutterClass ui;
	QSettings settings;

};
