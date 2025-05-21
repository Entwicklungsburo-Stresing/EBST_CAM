#include "dialogservo.h"
#include <algorithm>

DialogServo::DialogServo(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::dialogservoClass)
{
	ui->setupUi(this);
	//connect(ui->lineEditDec, &QLineEdit::textChanged, this, &DialogServo::on_dec_changed);
	connect(ui->lineEditHex, &QLineEdit::textChanged, this, &DialogServo::on_hex_changed);
	connect(ui->lineEditBin, &QLineEdit::textChanged, this, &DialogServo::on_bin_changed);

	QRegularExpression decRegex("[0-9]{1,39}$");
	QValidator *decValidator = new QRegularExpressionValidator(decRegex, this);
	ui->lineEditDec->setValidator(decValidator);

	QRegularExpression hexRegex("[0-9A-Fa-f]{1,32}$");
	QValidator *hexValidator = new QRegularExpressionValidator(hexRegex, this);
	ui->lineEditHex->setValidator(hexValidator);

	QRegularExpression binRegex("[01]{1,128}$");
	QValidator *binValidator = new QRegularExpressionValidator(binRegex, this);
	ui->lineEditBin->setValidator(binValidator);
}

DialogServo::~DialogServo()
{
	delete ui;
}


void DialogServo::on_dec_changed() 
{
	QString dec = ui->lineEditDec->text();
	QString hexAsString = QString::number(dec.toInt(), 16);
	QString binAsString = QString::number(dec.toInt(), 2);
	ui->lineEditHex->setText(hexAsString);
	ui->lineEditBin->setText(binAsString);
	return;
}

void DialogServo::on_hex_changed()
{
	QString hex = ui->lineEditHex->text();
	QString binAsString = convertHexToBinary(hex);
	QString decAsString = convertBinaryToDecimal(binAsString);
	
	ui->lineEditDec->setText(decAsString);
	ui->lineEditBin->setText(binAsString);
	return;
}

void DialogServo::on_bin_changed()
{
	QString bin = ui->lineEditBin->text();
	QString decAsString = convertBinaryToDecimal(bin);
	QString hexAsString = convertBinaryToHex(bin);
	ui->lineEditDec->setText(decAsString);
	ui->lineEditHex->setText(hexAsString);
	return;
}

QString DialogServo::convertDecimalToBinary(QString decimalString)
{
	return QString();
}

QString DialogServo::convertDecimalToHex(QString decimalString)
{
	return QString();
}

QString DialogServo::convertHexToBinary(QString hexString)
{
	hexString = hexString.toUpper();
	if (hexString.isEmpty()) {
		return QString("0");
	}

	QString result = "";
	QString binaryChunk = "";
	int decimalValue = -1;
	for (QChar hexChar : hexString) {
		if (hexChar >= '0' && hexChar <= '9') {
			decimalValue = hexChar.unicode() - '0';
		}
		else if (hexChar >= 'A' && hexChar <= 'F') {
			decimalValue = hexChar.unicode() - 'A' + 10;
		}
		else {
			return QString("0");
		}

		for (int i = 3; i >= 0; --i) {
			binaryChunk.append((decimalValue & (1 << i)) ? '1' : '0');
		}

		result.append(binaryChunk);
	}

	return result;
}

QString DialogServo::convertHexToDecimal(QString hexString)
{
	return QString();
}

QString DialogServo::convertBinaryToDecimal(QString binaryString)
{
	std::string binaryAsStdString = binaryString.toStdString();
	constexpr unsigned int numberBase{ 10 };
	std::string result;

	do {
		unsigned int remainder = 0;
		std::string dividedNumberAsString = "";
		for (const char bit : binaryAsStdString) {
			remainder = remainder * 2 + (bit - '0');

			if (remainder >= numberBase) {
				remainder -= numberBase;
				dividedNumberAsString += '1';
			}
			else {
				dividedNumberAsString += '0';
			}
		}
		binaryAsStdString = dividedNumberAsString;
		result.insert(0, 1, '0' + remainder);
	} while (std::count(binaryAsStdString.begin(), binaryAsStdString.end(), '1'));

	return QString::fromStdString(result);
}

QString DialogServo::convertBinaryToHex(QString binaryString)
{
	if (binaryString.isEmpty()) {
		return QString("0");
	}
	
	bool binaryIsZero = true;
	for (QChar bit : binaryString) {
		if (bit != '0') {
			binaryIsZero = false;
			break;
		}
	}

	if (binaryIsZero) {
		return "0";
	}

	int remainder = binaryString.length() % 4;
	if (remainder != 0) {
		QString padding = QString(4 - remainder, '0');
		binaryString.prepend(padding);
	}

	QString result = "";
	for (int i = 0; i < binaryString.length(); i += 4) {
		QString chunk = binaryString.mid(i, 4);
		bool ok;
		int decimalValue = chunk.toInt(&ok, 2);

		if (decimalValue < 10) {
			result.append(QString::number(decimalValue));
		}
		else {
			result.append(QChar('A' + decimalValue - 10));
		}
	}

	// Remove leading zeros
	while (result.length() > 1 && result.startsWith('0')) {
		result.remove(0, 1);
	}
	return result;
}

