#pragma once

#include <QDialog>
#include "ui_dialogservo.h"

namespace Ui {
	class dialogservoClass;
}

class DialogServo : public QDialog
{
	Q_OBJECT

public:
	DialogServo(QWidget *parent = nullptr);
	~DialogServo();

private slots:
	void on_spinBoxSeqLength_valueChanged(int val);
	void on_lineEditDec_textChanged();
	void on_lineEditHex_textChanged();
	void on_lineEditBin_textChanged();
private:
	Ui::dialogservoClass* ui;
	QString convertDecimalToBinary(QString decimalString);
	QString convertHexToBinary(QString hexString);
	QString convertBinaryToDecimal(QString binaryString);
	QString convertBinaryToHex(QString binaryString);
	bool checkDecimal(QString decimalString);
};
