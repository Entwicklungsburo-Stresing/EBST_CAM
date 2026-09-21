/*****************************************************************//**
 * @file		dialogservo.h
 * @brief		Dialog for controlling the servo unit.
 * @author		Florian Hahn
 * @date		16.04.2025
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released under the GPL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "lsc-gui.h"
#include "lsc.h"
#include "ui_dialogservo.h"

namespace Ui {
	class dialogservoClass;
}

class DialogServo : public QDialog
{
	Q_OBJECT

public:
	DialogServo(QWidget* parent = nullptr);
	~DialogServo();

private slots:
	void on_spinBoxBoard_valueChanged();
	void on_spinBoxSeqLength_valueChanged(int val);
	void on_lineEditDec_textChanged();
	void on_lineEditHex_textChanged();
	void on_lineEditBin_textChanged();
	void on_lineEditBin_editingFinished();
	void on_spinBoxPos1_valueChanged(int value);
	void on_spinBoxPos2_valueChanged(int value);
	void on_comboBoxTriggerSource_currentIndexChanged();
	void on_pushButtonManualTrig_clicked();
	void on_radioButtonCalibratePos1_toggled();
	void on_radioButtonCalibratePos2_toggled();
	void on_radioButtonTrigSeq_toggled();
	void on_pushButtonDefault_clicked();
	void on_spinBoxStepPeriod_valueChanged(int value);
	void on_pushButtonSendAll_clicked();
	void on_spinBoxSeqDelay_valueChanged(int value);
	void on_spinBoxSeqOffset_valueChanged(int value);

private:
	Ui::dialogservoClass* ui;
	QSettings settings;
	void loadSettings();
	QString convertDecimalToBinary(QString decimalString);
	QString convertHexToBinary(QString hexString);
	QString convertBinaryToDecimal(QString binaryString);
	QString convertBinaryToHex(QString binaryString);
	QString addLeadingZerosToBin(QString bin);
};
