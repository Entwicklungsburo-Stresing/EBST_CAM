/*****************************************************************//**
 * @file		dialogioctrl.h
 * @brief		Dialog for setting the IO control values.
 * @author		Florian Hahn
 * @date		02.09.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "lsc-gui.h"

namespace Ui {
	class DialogIoctrlLegacy;
}

class DialogIoctrlLegacy : public QDialog
{
	Q_OBJECT

public:
	DialogIoctrlLegacy(QWidget *parent = Q_NULLPTR);
	~DialogIoctrlLegacy();

private slots:
	void setOutput(uint8_t outputNumber, uint16_t width_in_5ns, uint16_t delay_in_5ns);
	void setT0(uint32_t period_in_10ns);
	//void on_buttonBox_rejected();
	//void on_buttonBox_accepted();
	void on_pushButtonDefault_pressed();
	void on_spinBoxBoard_valueChanged();
	void spinBox_valueChanged();

private:
	Ui::DialogIoctrlLegacy *ui;
	QSettings settings;
	uint16_t outputWidth_old[7];
	uint16_t outputDelay_old[7];
	uint32_t t0_old;
};
