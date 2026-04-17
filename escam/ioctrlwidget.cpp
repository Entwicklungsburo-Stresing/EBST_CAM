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

/**
* @brief Load default settings into the widget and save them to QSettings.
* @return none
*/
void IoctrlWidget::loadDefaults()
{
	ui.lineEditDelay->setText(formatNsToHumanReadable(settingIoctrlDelayIn1nsDefault));
	ui.lineEditWidth->setText(formatNsToHumanReadable(settingIoctrlWidthIn1nsDefault));
	ui.widgetNumericConverter->setSequenceLength(settingIoctrlSequenceLengthDefault);
	ui.widgetNumericConverter->setHex(settingIoctrlSequenceDefault);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlDelayIn1nsPath, (int)settingIoctrlDelayIn1nsDefault);
	settings.setValue(settingIoctrlWidthIn1nsPath, (int)settingIoctrlWidthIn1nsDefault);
	settings.setValue(settingIoctrlSequencePath, ui.widgetNumericConverter->getHex());
	settings.setValue(settingIoctrlSequenceLengthPath, ui.widgetNumericConverter->getSequenceLength().toInt());
	settings.endGroup();
	return;
}

/**
* @brief Load saved settings from QSettings into the widget.
* @param drvno The driver number (board number) to load settings for.
* @return none
*/
void IoctrlWidget::loadSettings(int drvno)
{
	settings.beginGroup(QString("board%1/ch%2").arg(drvno).arg(channel));
	int delay = settings.value(settingIoctrlDelayIn1nsPath, settingIoctrlDelayIn1nsDefault).toInt();
	int width = settings.value(settingIoctrlWidthIn1nsPath, settingIoctrlWidthIn1nsDefault).toInt();
	QString sequence = settings.value(settingIoctrlSequencePath, settingIoctrlSequenceDefault).toString();
	int sequenceLength = settings.value(settingIoctrlSequenceLengthPath, settingIoctrlSequenceLengthDefault).toInt();
	settings.endGroup();
	ui.widgetNumericConverter->setHex(sequence);
	ui.widgetNumericConverter->setSequenceLength(sequenceLength);
	_drvno = drvno;
	sendAllSettings();
	return;
}

/**
* @brief Slot called when the sequence length is changed in the NumericConverterWidget.
* @return none
*/
void IoctrlWidget::sequenceLengthChanged()
{
	int val = ui.widgetNumericConverter->getSequenceLength().toInt();
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorSequenceLengthPath, val);
	settings.endGroup();
	return;
}

/**
* @brief Send the current sequence from the NumericConverterWidget to the hardware and save it to QSettings.
* @return none
*/
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

/**
* @brief Send all current settings (delay, width, sequence, sequence length) to the hardware and save them to QSettings.
* @return none
*/
void IoctrlWidget::sendAllSettings()
{
	TimeResult trDelay = processTimeInput(ui.lineEditDelay->text(), delayLimit);
	TimeResult trWidth = processTimeInput(ui.lineEditWidth->text(), widthLimit);
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, trWidth.ns);
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, trDelay.ns);
	sendSequence();
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, ui.widgetNumericConverter->getSequenceLength().toInt());
	return;
}

/**
* @brief Process a time input string, converting it to nanoseconds and formatting it.
* @param input The input time string (e.g., "1.5 ms", "200 us").
* @param maxNs The maximum allowed time in nanoseconds.
* @return A TimeResult struct containing the nanoseconds value, formatted string, and validity flag.
*/
IoctrlWidget::TimeResult IoctrlWidget::processTimeInput(const QString& input, long long maxNs)
{
	TimeResult result = { -1, "", false };
	/*
	 * Regex Breakdown: ^\s*(\d+(?:[.,]\d*)?|[.,]\d+)\s*(s|ms|us|\u00B5s|ns)?\s*$
	 * ----------------------------------------------------------------------------
	 * ^\s*				: Start of string, allow optional leading whitespace.
	 * (                : Start of Group 1 (Value):
	 * \d+				: Match one or more digits...
	 * (?:[.,]\d*)?		: ...optionally followed by a decimal separator (. or ,) and zero or more digits.
	 * |				: OR
	 * [.,]\d+			: Match a decimal separator followed by at least one digit (e.g., ".5").
	 * )                : End of Group 1.
	 * \s*				: Allow optional whitespace between value and unit.
	 * (s|ms|us|\u00B5s|ns)? : Group 2 (Unit): Optional match of s, ms, us, µs (\u00B5s), or ns.
	 * \s*$             : Allow optional trailing whitespace and require end of string.
	 */
	static QRegularExpression regex("^\\s*(\\d+(?:[.,]\\d*)?|[.,]\\d+)\\s*(s|ms|us|\u00B5s|ns)?\\s*$");
	QRegularExpressionMatch match = regex.match(input.toLower().trimmed());
	
	if (!match.hasMatch()) return result;

	// Extract captured groups
	QString numberPart = match.captured(1).replace(',', '.');
	QString unit = match.captured(2);

	// Convert number part to double
	bool ok = false;
	double originalValue = match.captured(1).replace(',', '.').toDouble(&ok);
	if (!ok) return result;

	if (unit.isEmpty()) unit = "ns";
	
	// Find correct factor based on unit
	double factor = 1.0;
	if (unit == "s") factor = 1e9;
	else if (unit == "ms") factor = 1e6;
	else if (unit == "us" || unit == QString("\u00B5s"))
	{
		factor = 1e3;
		unit = QString("\u00B5s");
	}
	else if (unit == "ns") factor = 1.0;
	else return result; // Indicate invalid unit

	// Convert to nanoseconds and apply rounding
	long long nsRaw = static_cast<long long>(std::round(originalValue * factor));

	if (nsRaw > maxNs) nsRaw = maxNs; // Exceeds hardware limit
	if (nsRaw < 0) nsRaw = 0; // Negative value

	// Round to nearest multiple of 32 ns
	long long nsRounded = ((nsRaw + 16) / 32) * 32;
	double displayValue = static_cast<double>(nsRounded) / factor;
	result.ns = nsRounded;
	result.formatted = QString::number(displayValue, 'g', 6) + " " + unit;
	result.valid = true;
	return result;
}

/**
* @brief Slot called when the delay line edit editing is finished. Processes the input and updates the delay.
* @return none
*/
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
		ui.lineEditDelay->setStyleSheet("border: 1px solid red; border-radius: 4px; background-color: #FFF0F0;");
	}
	return;
}

/**
* @brief Update the pulse delay in the hardware and save it to QSettings.
* @param delay The new delay value in nanoseconds.
* @return none
*/
void IoctrlWidget::delayChanged(int delay)
{
	mainWindow->lsc.camIOCtrl_setPulseDelay(_drvno, channel, delay);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlDelayIn1nsPath, delay);
	settings.endGroup();
	return;
}

/**
* @brief Slot called when the width line edit editing is finished. Processes the input and updates the width.
* @return none
*/
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
		ui.lineEditWidth->setStyleSheet("border: 1px solid red; border-radius: 4px; background-color: #FFF0F0;");
	}
	return;
}

/**
* @brief Update the pulse width in the hardware and save it to QSettings.
* @param width The new width value in nanoseconds.
* @return none
*/
void IoctrlWidget::widthChanged(int width)
{
	mainWindow->lsc.camIOCtrl_setPulseWidth(_drvno, channel, width);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingIoctrlWidthIn1nsPath, width);
	settings.endGroup();
	return;
}

QString IoctrlWidget::formatNsToHumanReadable(long long ns)
{
	if (ns >= 1000000000LL) return QString::number(ns / 1e9, 'g', 6) + " s";
	else if (ns >= 1000000LL) return QString::number(ns / 1e6, 'g', 6) + " ms";
	else if (ns >= 1000LL) return QString::number(ns / 1e3, 'g', 6) + " \u00B5s";
	else return QString::number(ns) + " ns";
}
