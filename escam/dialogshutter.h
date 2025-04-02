/*****************************************************************//**
 * @file   dialogshutter.h
 * @brief  Dialog for controlling the shutters.
 * 
 * @author Florian
 * @date   06.03.2025
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
	void on_buttonBox_accepted();
	void loadSavedValues();
private:
	Ui::DialogShutterClass ui;
	QSettings settings;

};
