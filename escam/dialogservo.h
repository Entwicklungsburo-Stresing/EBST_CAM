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
	void on_dec_changed();
	void on_hex_changed();
	void on_bin_changed();
private:
	Ui::dialogservoClass* ui;
	QString convertDecimalToBinary(QString decimalString);
	QString convertDecimalToHex(QString decimalString);
	QString convertHexToBinary(QString hexString);
	QString convertHexToDecimal(QString hexString);
	QString convertBinaryToDecimal(QString binaryString);
	QString convertBinaryToHex(QString binaryString);
};
