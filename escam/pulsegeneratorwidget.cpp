#include "pulsegeneratorwidget.h"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>
#include "dialogsettings.h"
#include "lsc-gui.h"

PulseGeneratorWidget::PulseGeneratorWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::PulseGeneratorWidgetClass())
{
	ui->setupUi(this);
	ui->lineEditDec->setText("0");
	QRegularExpression decRegex("^[0-9]*$");
	QValidator* decValidator = new QRegularExpressionValidator(decRegex, this);
	ui->lineEditDec->setValidator(decValidator);

	QRegularExpression hexRegex("^[0-9A-Fa-f]*$");
	QValidator* hexValidator = new QRegularExpressionValidator(hexRegex, this);
	ui->lineEditHex->setValidator(hexValidator);

	QRegularExpression binRegex("^[01]*$");
	QValidator* binValidator = new QRegularExpressionValidator(binRegex, this);
	ui->lineEditBin->setValidator(binValidator);
	on_checkBoxSequenceOn_checkStateChanged(Qt::Unchecked);
}

PulseGeneratorWidget::~PulseGeneratorWidget()
{
	delete ui;
}

void PulseGeneratorWidget::loadDefaults()
{
	double delayInMicroseconds = settingPulseGeneratorDelayIn1nsDefault / 1000;
	double widthInMicroseconds = settingPulseGeneratorWidthIn1nsDefault / 1000;
	ui->doubleSpinBoxDelayMicroseconds->setValue(delayInMicroseconds);
	ui->doubleSpinBoxWidthMicroseconds->setValue(widthInMicroseconds);
	ui->spinBoxSeqLength->setValue(settingPulseGeneratorSequenceLengthDefault);
	ui->lineEditHex->setText(settingPulseGeneratorSequenceDefault);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorDelayIn1nsPath, ui->doubleSpinBoxDelayMicroseconds->value() * 1000);
	settings.setValue(settingPulseGeneratorWidthIn1nsPath, ui->doubleSpinBoxWidthMicroseconds->value() * 1000);
	settings.setValue(settingPulseGeneratorSequencePath, ui->lineEditHex->text());
	settings.setValue(settingPulseGeneratorSequenceLengthPath, ui->spinBoxSeqLength->value());
	settings.endGroup();
	return;
}

void PulseGeneratorWidget::loadSettings(int drvno)
{
	settings.beginGroup(QString("board%1/ch%2").arg(drvno).arg(channel));
	double delayInMicroseconds = settings.value(settingPulseGeneratorDelayIn1nsPath, settingPulseGeneratorDelayIn1nsDefault).toInt() / 1000;
	double widthInMicroseconds = settings.value(settingPulseGeneratorWidthIn1nsPath, settingPulseGeneratorWidthIn1nsDefault).toInt() / 1000;
	QString sequence = settings.value(settingPulseGeneratorSequencePath, settingPulseGeneratorSequenceDefault).toString();
	int sequenceLength = settings.value(settingPulseGeneratorSequenceLengthPath, settingPulseGeneratorSequenceLengthDefault).toInt();
	settings.endGroup();
	ui->doubleSpinBoxDelayMicroseconds->setValue(delayInMicroseconds);
	ui->doubleSpinBoxWidthMicroseconds->setValue(widthInMicroseconds);
	ui->lineEditHex->setText(sequence);
	ui->spinBoxSeqLength->setValue(sequenceLength);
	_drvno = drvno;
	sendAllSettings();
	return;
}

void PulseGeneratorWidget::on_spinBoxSeqLength_valueChanged(int val)
{
	if (ui->lineEditBin->text().length() > val)
	{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
		ui->lineEditBin->setText(ui->lineEditBin->text().right(val));
#else
		ui->lineEditBin->setText(ui->lineEditBin->text().last(val));
#endif
	}
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorSequenceLengthPath, val);
	settings.endGroup();
}

void PulseGeneratorWidget::on_lineEditDec_textChanged()
{
	QString dec = ui->lineEditDec->text();
	if (dec.isEmpty()) {
		return;
	}
	QString maxDec = convertBinaryToDecimal(QString("1").repeated(ui->spinBoxSeqLength->value()));
	if (dec.length() > maxDec.length()) {
		ui->lineEditDec->setText(maxDec);
		dec = maxDec;
	}
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
	QString bin = convertDecimalToBinary(dec);
	QString hex = convertBinaryToHex(bin);

	ui->lineEditHex->blockSignals(true);
	ui->lineEditBin->blockSignals(true);

	ui->lineEditHex->setText(hex);
	ui->lineEditBin->setText(addLeadingZerosToBin(bin));

	ui->lineEditHex->blockSignals(false);
	ui->lineEditBin->blockSignals(false);
	sendSequence();
	return;
}

void PulseGeneratorWidget::on_lineEditHex_textChanged()
{
	QString hex = ui->lineEditHex->text();
	QString maxHex = convertBinaryToHex(QString("1").repeated(ui->spinBoxSeqLength->value()));
	if (hex.isEmpty()) {
		return;
	}
	if (hex.length() > maxHex.length()) {
		ui->lineEditHex->setText(maxHex);
		hex = maxHex;
	}
	else if (hex.length() == maxHex.length()) {
		if (QString(maxHex[0]).toInt(nullptr, 16) < QString(hex[0]).toInt(nullptr, 16)) {
			ui->lineEditHex->setText(maxHex);
			hex = maxHex;
		}
	}

	QString bin = convertHexToBinary(hex);
	QString dec = convertBinaryToDecimal(bin);

	ui->lineEditDec->blockSignals(true);
	ui->lineEditBin->blockSignals(true);

	ui->lineEditDec->setText(dec);
	ui->lineEditBin->setText(addLeadingZerosToBin(bin));

	ui->lineEditDec->blockSignals(false);
	ui->lineEditBin->blockSignals(false);
	sendSequence();
	return;
}

void PulseGeneratorWidget::on_lineEditBin_textChanged()
{

	if (ui->lineEditBin->text().isEmpty()) {
		return;
	}
	int cursorPosition = ui->lineEditBin->cursorPosition();
	if (ui->lineEditBin->text().length() >= ui->spinBoxSeqLength->value()) {
		ui->lineEditBin->setText(ui->lineEditBin->text().right(ui->spinBoxSeqLength->value()));
	}
	// Saves the cursor position before changing the text to prevent it from jumping to the end. - 1 because it is 0-based index
	ui->lineEditBin->setCursorPosition(cursorPosition - 1);
	QString bin = ui->lineEditBin->text();
	QString dec = convertBinaryToDecimal(bin);
	QString hex = convertBinaryToHex(bin);

	ui->lineEditDec->blockSignals(true);
	ui->lineEditHex->blockSignals(true);

	ui->lineEditDec->setText(dec);
	ui->lineEditHex->setText(hex);

	ui->lineEditDec->blockSignals(false);
	ui->lineEditHex->blockSignals(false);
	sendSequence();
	return;
}

void PulseGeneratorWidget::on_lineEditBin_editingFinished() {
	ui->lineEditBin->blockSignals(true);
	QString bin = ui->lineEditBin->text();
	ui->lineEditBin->setText(addLeadingZerosToBin(bin));
	ui->lineEditBin->blockSignals(false);
}

QString PulseGeneratorWidget::convertDecimalToBinary(QString decimal)
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

QString PulseGeneratorWidget::convertHexToBinary(QString hex)
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

QString PulseGeneratorWidget::convertBinaryToDecimal(QString binary)
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

QString PulseGeneratorWidget::convertBinaryToHex(QString binary)
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
	return result;
}

QString PulseGeneratorWidget::addLeadingZerosToBin(QString bin)
{
	int length = ui->spinBoxSeqLength->value();
	if (bin.length() < length) {
		bin.prepend(QString(length - bin.length(), '0'));
	}
	return bin;
}

void PulseGeneratorWidget::sendSequence()
{
	QString hex = ui->lineEditHex->text();

	// Build 8-element uint16_t sequence (128 bits = 32 hex chars).
	// Pad the user hex to the low-order bits and zero-extend to 32 hex digits.
	const int totalHexDigits = 32; // 8 * 4 hex digits = 128 bits
	int userHexDigits = hex.length();
	// ensure hex uses uppercase and no prefix
	QString hexNormalized = hex.toUpper();

	// left-pad the user's hex so it represents the least-significant bits of the 128-bit word
	// first ensure the userHexDigits are right-justified (should already be), then pad to full width
	if (userHexDigits < totalHexDigits) {
		hexNormalized = QString(totalHexDigits - userHexDigits, '0') + hexNormalized;
	}

	uint16_t sequence[8] = { 0 };
	for (int i = 0; i < 8; ++i) {
		// each sequence element covers 4 hex digits (16 bits)
		QString seg = hexNormalized.mid(i * 4, 4);
		bool ok = false;
		uint32_t val = seg.toUInt(&ok, 16);
		if (!ok) val = 0;
		sequence[i] = static_cast<uint16_t>(val & 0xFFFFu);
	}

	mainWindow->lsc.camIOCtrl_setSequence(_drvno, channel, sequence);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlSequencePath, hex);
	settings.endGroup();
	return;
}

void PulseGeneratorWidget::on_doubleSpinBoxDelaySeconds_valueChanged(double val)
{
	// Take the given value, convert to milliseconds and microseconds, and set the other spin boxes accordingly.
	double microseconds = val * 1000000;
	double milliseconds = val * 1000;
	ui->doubleSpinBoxDelayMicroseconds->blockSignals(true);
	ui->doubleSpinBoxDelayMilliseconds->blockSignals(true);
	ui->doubleSpinBoxDelayMicroseconds->setValue(microseconds);
	ui->doubleSpinBoxDelayMilliseconds->setValue(milliseconds);
	ui->doubleSpinBoxDelayMicroseconds->blockSignals(false);
	ui->doubleSpinBoxDelayMilliseconds->blockSignals(false);
	int nanoseconds = val * 1000000000;
	delayChanged(nanoseconds);
	return;
}

void PulseGeneratorWidget::on_doubleSpinBoxDelayMilliseconds_valueChanged(double val)
{
	// Take the given value, convert to seconds and microseconds, and set the other spin boxes accordingly.
	double microseconds = val * 1000;
	double seconds = val / 1000;
	ui->doubleSpinBoxDelayMicroseconds->blockSignals(true);
	ui->doubleSpinBoxDelaySeconds->blockSignals(true);
	ui->doubleSpinBoxDelayMicroseconds->setValue(microseconds);
	ui->doubleSpinBoxDelaySeconds->setValue(seconds);
	ui->doubleSpinBoxDelayMicroseconds->blockSignals(false);
	ui->doubleSpinBoxDelaySeconds->blockSignals(false);
	int nanoseconds = val * 1000000;
	delayChanged(nanoseconds);
	return;
}

void PulseGeneratorWidget::on_doubleSpinBoxDelayMicroseconds_valueChanged(double val)
{
	// Take the given value, convert to seconds and milliseconds, and set the other spin boxes accordingly.
	double milliseconds = val / 1000;
	double seconds = val / 1000000;
	ui->doubleSpinBoxDelayMilliseconds->blockSignals(true);
	ui->doubleSpinBoxDelaySeconds->blockSignals(true);
	ui->doubleSpinBoxDelayMilliseconds->setValue(milliseconds);
	ui->doubleSpinBoxDelaySeconds->setValue(seconds);
	ui->doubleSpinBoxDelayMilliseconds->blockSignals(false);
	ui->doubleSpinBoxDelaySeconds->blockSignals(false);
	int nanoseconds = val * 1000;
	delayChanged(nanoseconds);
	return;
}

void PulseGeneratorWidget::delayChanged(int val)
{
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorDelayIn1nsPath, val);
	settings.endGroup();
	return;
}

void PulseGeneratorWidget::on_doubleSpinBoxWidthSeconds_valueChanged(double val)
{
	// Take the given value, convert to milliseconds and microseconds, and set the other spin boxes accordingly.
	double microseconds = val * 1000000;
	double milliseconds = val * 1000;
	ui->doubleSpinBoxWidthMicroseconds->blockSignals(true);
	ui->doubleSpinBoxWidthMilliseconds->blockSignals(true);
	ui->doubleSpinBoxWidthMicroseconds->setValue(microseconds);
	ui->doubleSpinBoxWidthMilliseconds->setValue(milliseconds);
	ui->doubleSpinBoxWidthMicroseconds->blockSignals(false);
	ui->doubleSpinBoxWidthMilliseconds->blockSignals(false);
	int nanoseconds = val * 1000000000;
	widthChanged(nanoseconds);
	return;
}

void PulseGeneratorWidget::on_doubleSpinBoxWidthMilliseconds_valueChanged(double val)
{
	// Take the given value, convert to seconds and microseconds, and set the other spin boxes accordingly.
	double microseconds = val * 1000.00;
	double seconds = val / 1000.00;
	ui->doubleSpinBoxWidthMicroseconds->blockSignals(true);
	ui->doubleSpinBoxWidthSeconds->blockSignals(true);
	ui->doubleSpinBoxWidthMicroseconds->setValue(microseconds);
	ui->doubleSpinBoxWidthSeconds->setValue(seconds);
	ui->doubleSpinBoxWidthMicroseconds->blockSignals(false);
	ui->doubleSpinBoxWidthSeconds->blockSignals(false);
	int nanoseconds = val * 1000000;
	widthChanged(nanoseconds);
	return;
}

void PulseGeneratorWidget::on_doubleSpinBoxWidthMicroseconds_valueChanged(double val)
{
	// Take the given value, convert to seconds and milliseconds, and set the other spin boxes accordingly.
	double milliseconds = val / 1000;
	double seconds = val / 1000000;
	ui->doubleSpinBoxWidthMilliseconds->blockSignals(true);
	ui->doubleSpinBoxWidthSeconds->blockSignals(true);
	ui->doubleSpinBoxWidthMilliseconds->setValue(milliseconds);
	ui->doubleSpinBoxWidthSeconds->setValue(seconds);
	ui->doubleSpinBoxWidthMilliseconds->blockSignals(false);
	ui->doubleSpinBoxWidthSeconds->blockSignals(false);
	int nanoseconds = val * 1000;
	widthChanged(nanoseconds);
	return;
}

void PulseGeneratorWidget::widthChanged(int val)
{
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorWidthIn1nsPath, val);
	settings.endGroup();
	return;
}

void PulseGeneratorWidget::sendAllSettings()
{
	int delayValue = ui->doubleSpinBoxDelayMicroseconds->value() * 1000;
	int widthValue = ui->doubleSpinBoxDelayMicroseconds->value() * 1000;
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, delayValue);
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, widthValue);
	on_lineEditHex_textChanged();
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, ui->spinBoxSeqLength->value());
	return;
}

void PulseGeneratorWidget::on_checkBoxSequenceOn_checkStateChanged(Qt::CheckState checked)
{
	ui->labelDec->setVisible(checked);
	ui->lineEditDec->setVisible(checked);
	ui->labelHex->setVisible(checked);
	ui->lineEditHex->setVisible(checked);
	ui->labelBin->setVisible(checked);
	ui->lineEditBin->setVisible(checked);
	ui->labelSeqLength->setVisible(checked);
	ui->spinBoxSeqLength->setVisible(checked);
	return;
}
