/*****************************************************************//**
 * @file		dialogdac.h
 * @brief		Dialog to set the DAC values.
 * @author		Florian Hahn
 * @date		04.08.2021
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "lsc-gui.h"

namespace Ui {
	class DialogDac;
}

class DialogDac : public QDialog
{
	Q_OBJECT

public:
	DialogDac(QWidget *parent = Q_NULLPTR);
	~DialogDac();

private slots:
	void spinBoxChannel_valueChanged();
	void loadSettings();
	void on_pushButtonDefault_pressed();
	void on_pushButtonAutotune_pressed();
	void on_autotuneStateChanged();
	void reject();
	void checkTargetReached();

private:
	uint32_t output_old[8];
	Ui::DialogDac *ui;
	QSettings settings;
	// Because of the signal slot mechanism of Qt the following boolean can be called from different threads. That is the reason why they are volatile. Otherwise it was observed, that the autotune dialog gets stuck.
	volatile bool autotuneRunning = false;
	volatile bool ch1TargetReached = false, ch2TargetReached = false, ch3TargetReached = false, ch4TargetReached = false, ch5TargetReached = false, ch6TargetReached = false, ch7TargetReached = false, ch8TargetReached = false;
	void autotunePressed();
	double calculateMean(uint16_t* camera_data, int start, int end, bool isHsIr);
	bool autotuneAdjust(uint16_t* camera_data, int start, int end, QSpinBox* spinBox, bool isHsir);
};
