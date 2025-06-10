#include "dialogservo.h"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>

DialogServo::DialogServo(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::dialogservoClass)
{
	ui->setupUi(this);

	QRegularExpression decRegex("^[0-9]*$");
	QValidator *decValidator = new QRegularExpressionValidator(decRegex, this);
	ui->lineEditDec->setValidator(decValidator);

	QRegularExpression hexRegex("^[0-9A-Fa-f]*$");
	QValidator *hexValidator = new QRegularExpressionValidator(hexRegex, this);
	ui->lineEditHex->setValidator(hexValidator);

	QRegularExpression binRegex("^[01]*$");
	QValidator *binValidator = new QRegularExpressionValidator(binRegex, this);
	ui->lineEditBin->setValidator(binValidator);
}

DialogServo::~DialogServo()
{
	delete ui;
}

void DialogServo::on_spinBoxSeqLength_valueChanged(int val) {
	ui->lineEditBin->setMaxLength(val);
	if (ui->lineEditBin->text().length() > val) {
		ui->lineEditBin->setText(ui->lineEditBin->text().left(val));
	}
}

void DialogServo::on_lineEditDec_textChanged()
{
	QString dec = ui->lineEditDec->text();
	QString maxDec = convertBinaryToDecimal(QString("1").repeated(ui->spinBoxSeqLength->value()));
	if (dec.length() == maxDec.length()) {
		for (int i = 0; i < maxDec.length(); ++i) {
			if (maxDec[i].digitValue() < dec[i].digitValue()) {
				ui->lineEditDec->setText(maxDec);
				dec = maxDec;
				break;
			}
			else if (maxDec[i].digitValue() > dec[i].digitValue()) {
				break;
			}
		}
	}
	QString binAsString = convertDecimalToBinary(dec);
	QString hexAsString = convertBinaryToHex(binAsString);

	ui->lineEditHex->blockSignals(true);
	ui->lineEditBin->blockSignals(true);

	ui->lineEditHex->setText(hexAsString);
	ui->lineEditBin->setText(binAsString);

	ui->lineEditHex->blockSignals(false);
	ui->lineEditBin->blockSignals(false);
	return;
}

void DialogServo::on_lineEditHex_textChanged()
{
	QString hex = ui->lineEditHex->text();
	QString maxHex = convertBinaryToHex(QString("1").repeated(ui->spinBoxSeqLength->value()));
	
	QString binAsString = convertHexToBinary(hex);
	QString decAsString = convertBinaryToDecimal(binAsString);

	ui->lineEditDec->blockSignals(true);
	ui->lineEditBin->blockSignals(true);

	ui->lineEditDec->setText(decAsString);
	ui->lineEditBin->setText(binAsString);

	ui->lineEditDec->blockSignals(false);
	ui->lineEditBin->blockSignals(false);
	return;
}

void DialogServo::on_lineEditBin_textChanged()
{
	QString bin = ui->lineEditBin->text();
	QString decAsString = convertBinaryToDecimal(bin);
	QString hexAsString = convertBinaryToHex(bin);

	ui->lineEditDec->blockSignals(true);
	ui->lineEditHex->blockSignals(true);

	ui->lineEditDec->setText(decAsString);
	ui->lineEditHex->setText(hexAsString);

	ui->lineEditDec->blockSignals(false);
	ui->lineEditHex->blockSignals(false);
	return;
}

QString DialogServo::convertDecimalToBinary(QString decimal)
{
	std::string decimalAsStdString = decimal.toStdString();

	if (decimalAsStdString.empty() || decimalAsStdString == "0") {
		return QString("0");
	}

	std::string binary = "";
	std::string temp = decimalAsStdString;

	while (temp != "0") {
		int remainder = 0;
		std::string dividedNumberAsString = "";
		for (const char digit : temp) {
			int current = remainder * 10 + (digit - '0');

			remainder = current % 2;
			dividedNumberAsString += (current / 2) + '0';
		}

		binary += (remainder + '0');
		size_t firstNonZero = dividedNumberAsString.find_first_not_of('0');
		if (firstNonZero != std::string::npos) {
			temp = dividedNumberAsString.substr(firstNonZero);
		}
		else {
			temp = "0";
		}
	}
	std::reverse(binary.begin(), binary.end());
	while (binary.length() > 1 && binary[0] == '0') {
		binary.erase(0, 1);
	}

	return QString::fromStdString(binary);
}

QString DialogServo::convertHexToBinary(QString hex)
{
	std::string hexAsStdString = hex.toUpper().toStdString();
	std::string result;
	
	if (hexAsStdString.empty()) {
		return QString("0");
	}

	std::string binaryAsStdString = "";
	for (const char hexChar : hexAsStdString) {
		int decimalValue = 0;
		if (hexChar >= '0' && hexChar <= '9') {
			decimalValue = hexChar - '0';
		}
		else if (hexChar >= 'A' && hexChar <= 'F') {
			decimalValue = hexChar - 'A' + 10;
		}

		std::bitset<4> binarySet(decimalValue);
		binaryAsStdString += binarySet.to_string();
	}
	
	while (binaryAsStdString.length() > 1 && binaryAsStdString[0] == '0') {
		binaryAsStdString.erase(0, 1);
	}

	return QString::fromStdString(binaryAsStdString);
}

QString DialogServo::convertBinaryToDecimal(QString binary)
{
	std::string binaryAsStdString = binary.toStdString();
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

QString DialogServo::convertBinaryToHex(QString binary)
{
	if (binary.isEmpty()) {
		return QString("0");
	}

	int remainder = binary.length() % 4;
	if (remainder != 0) {
		QString padding = QString(4 - remainder, '0');
		binary.prepend(padding);
	}

	QString result = "";
	for (int i = 0; i < binary.length(); i += 4) {
		QString chunk = binary.mid(i, 4);
		bool ok;
		int decimalValue = chunk.toInt(&ok, 2);

		if (decimalValue < 10) {
			result.append(QString::number(decimalValue));
		}
		else {
			result.append(QChar('A' + decimalValue - 10));
		}
	}

	/*while (result.length() > 1 && result.startsWith('0')) {
		result.remove(0, 1);
	}*/
	return result;
}
