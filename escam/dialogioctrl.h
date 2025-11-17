/*****************************************************************//**
 * @file		dialogioctrl.h
 * @brief		Dialog for setting the IO control values.
 * @author		Florian Hahn
 * @date		02.09.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "lsc-gui.h"

namespace Ui {
	class DialogIoctrl;
}

class DialogIoctrl : public QDialog
{
	Q_OBJECT

public:
	DialogIoctrl(QWidget *parent = Q_NULLPTR);
	~DialogIoctrl();
signals:
	void settingsLoaded(int drvno);
	void defaults_loaded();
private:
	Ui::DialogIoctrl *ui;
	QSettings settings;
private slots:
	void on_comboBoxTrigSource_currentIndexChanged(int index);
	void loadSettings();
	void on_spinBoxBoard_valueChanged(int value);
};
