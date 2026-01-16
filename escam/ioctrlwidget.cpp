#include "ioctrlwidget.h"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>
#include "dialogsettings.h"
#include "lsc-gui.h"

IoctrlWidget::IoctrlWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.widgetNumericConverter, &NumericConverterWidget::sequenceChanged, this, &IoctrlWidget::sendSequence);
	connect(ui.widgetNumericConverter, &NumericConverterWidget::sequenceLengthChanged, this, &IoctrlWidget::sequenceLengthChanged);
}

IoctrlWidget::~IoctrlWidget()
{}

void IoctrlWidget::loadDefaults()
{
	ui.spinBoxDelay->setValue(settingIoctrlDelayIn1nsDefault);
	ui.spinBoxWidth->setValue(settingIoctrlWidthIn1nsDefault);
	ui.widgetNumericConverter->setSequenceLength(settingIoctrlSequenceLengthDefault);
	ui.widgetNumericConverter->setHex(settingIoctrlSequenceDefault);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlDelayIn1nsPath, ui.spinBoxDelay->value());
	settings.setValue(settingIoctrlWidthIn1nsPath, ui.spinBoxWidth->value());
	settings.setValue(settingIoctrlSequencePath, ui.widgetNumericConverter->getHex());
	settings.setValue(settingIoctrlSequenceLengthPath, ui.widgetNumericConverter->getSequenceLength().toInt());
	settings.endGroup();
	return;
}

void IoctrlWidget::loadSettings(int drvno)
{
	settings.beginGroup(QString("board%1/ch%2").arg(drvno).arg(channel));
	int delay = settings.value(settingIoctrlDelayIn1nsPath, settingIoctrlDelayIn1nsDefault).toInt();
	int width = settings.value(settingIoctrlWidthIn1nsPath, settingIoctrlWidthIn1nsDefault).toInt();
	QString sequence = settings.value(settingIoctrlSequencePath, settingIoctrlSequenceDefault).toString();
	int sequenceLength = settings.value(settingIoctrlSequenceLengthPath, settingIoctrlSequenceLengthDefault).toInt();
	settings.endGroup();
	ui.spinBoxDelay->setValue(delay);
	ui.spinBoxWidth->setValue(width);
	ui.widgetNumericConverter->setHex(sequence);
	ui.widgetNumericConverter->setSequenceLength(sequenceLength);
	_drvno = drvno;
	sendAllSettings();
	return;
}

void IoctrlWidget::sequenceLengthChanged()
{
	int val = ui.widgetNumericConverter->getSequenceLength().toInt();
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorSequenceLengthPath, val);
	settings.endGroup();
	return;
}

void IoctrlWidget::sendSequence()
{
	QString hex = ui.widgetNumericConverter->getHex();

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

void IoctrlWidget::on_spinBoxDelay_valueChanged(int val)
{
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlDelayIn1nsPath, val);
	settings.endGroup();
	return;
}

void IoctrlWidget::on_spinBoxWidth_valueChanged(int val)
{
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlWidthIn1nsPath, val);
	settings.endGroup();
	return;
}

void IoctrlWidget::sendAllSettings()
{
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, ui.spinBoxWidth->value());
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, ui.spinBoxDelay->value());
	sendSequence();
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, ui.widgetNumericConverter->getSequenceLength().toInt());
	return;
}

IoctrlWidget::TimeResult IoctrlWidget::processTimeInput(const QString& input, long long maxNs)
{
	TimeResult result = { -1, "", false };
	static QRegularExpression regex("^\\s*([0-9.,]+)\\s*(s|ms|us|ns)?\\s*$"); // Allows the input of whole or decimal numbers with either "." or "," and an optional unit
	QRegularExpressionMatch match = regex.match(input.toLower().trimmed());
	
	if (!match.hasMatch()) return result;

	double originalValue = match.captured(1).replace(',', '.').toDouble();
	QString unit = match.captured(2);
	if (unit.isEmpty()) unit = "ns";
	double factor = 1.0;
	if (unit == "s") factor = 1e9;
	else if (unit == "ms") factor = 1e6;
	else if (unit == "us") factor = 1e3;
	else if (unit == "ns") factor = 1.0;
	else return result; // Indicate invalid unit

	long long nsRaw = static_cast<long long>(std::round(originalValue * factor));
	if (nsRaw > maxNs) nsRaw = maxNs; // Exceeds hardware limit
	if (nsRaw < 0) nsRaw = 0; // Negative value
	long long nsRounded = ((nsRaw + 16) / 32) * 32;
	double displayValue = static_cast<double>(nsRounded) / factor;
	result.ns = nsRounded;
	result.formatted = QString::number(displayValue, 'g', 6) + " " + unit;
	result.valid = true;
	return result;
}

void IoctrlWidget::on_lineEditDelay_editingFinished()
{
	QString input = ui.lineEditDelay->text();
	TimeResult tr = processTimeInput(input, delayLimit);
	if (tr.valid) {
		ui.lineEditDelay->blockSignals(true);
		ui.lineEditDelay->setText(tr.formatted);
		ui.lineEditDelay->blockSignals(false);
		ui.lineEditDelay->setStyleSheet("");
		delayChanged(static_cast<int>(tr.ns));
	} else {
		ui.lineEditDelay->setStyleSheet("border: 1px solid red;");
	}
	return;
}

void IoctrlWidget::delayChanged(int delay)
{
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, delay);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlDelayIn1nsPath, delay);
	settings.endGroup();
	return;
}

void IoctrlWidget::on_lineEditWidth_editingFinished()
{
	QString input = ui.lineEditWidth->text();
	TimeResult tr = processTimeInput(input, widthLimit);
	if (tr.valid) {
		ui.lineEditWidth->blockSignals(true);
		ui.lineEditWidth->setText(tr.formatted);
		ui.lineEditWidth->blockSignals(false);
		ui.lineEditWidth->setStyleSheet("");
		widthChanged(static_cast<int>(tr.ns));
	} else {
		ui.lineEditWidth->setStyleSheet("border: 1px solid red;");
	}
	return;
}

void IoctrlWidget::widthChanged(int width)
{
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, width);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlWidthIn1nsPath, width);
	settings.endGroup();
	return;
}
