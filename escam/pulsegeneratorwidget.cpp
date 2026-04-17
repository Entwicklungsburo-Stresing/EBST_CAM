#include "pulsegeneratorwidget.h"
#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>
#include "dialogsettings.h"
#include "lsc-gui.h"
#include "numericconverterwidget.h"

PulseGeneratorWidget::PulseGeneratorWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::PulseGeneratorWidgetClass())
{
	ui->setupUi(this);

	connect(ui->widgetNumericConverter, &NumericConverterWidget::sequenceChanged, this, &PulseGeneratorWidget::sendSequence);
	connect(ui->widgetNumericConverter, &NumericConverterWidget::sequenceLengthChanged, this, &PulseGeneratorWidget::sequenceLengthChanged);
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

	ui->widgetNumericConverter->setSequenceLength(settingPulseGeneratorSequenceLengthDefault);
	ui->widgetNumericConverter->setHex(settingPulseGeneratorSequenceDefault);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorDelayIn1nsPath, ui->doubleSpinBoxDelayMicroseconds->value() * 1000);
	settings.setValue(settingPulseGeneratorWidthIn1nsPath, ui->doubleSpinBoxWidthMicroseconds->value() * 1000);
	settings.setValue(settingPulseGeneratorSequencePath, ui->widgetNumericConverter->getHex());
	settings.setValue(settingPulseGeneratorSequenceLengthPath, ui->widgetNumericConverter->getSequenceLength());
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
	ui->widgetNumericConverter->setHex(sequence);
	ui->widgetNumericConverter->setSequenceLength(sequenceLength);
	_drvno = drvno;
	sendAllSettings();
	return;
}

void PulseGeneratorWidget::sequenceLengthChanged()
{
	int val = ui->widgetNumericConverter->getSequenceLength().toInt();
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, val);
	settings.beginGroup(QString("board%1/ch%2").arg(_drvno).arg(channel));
	settings.setValue(settingPulseGeneratorSequenceLengthPath, val);
	settings.endGroup();
	return;
}

void PulseGeneratorWidget::sendSequence()
{
	QString hex = ui->widgetNumericConverter->getHex();

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
	QString hex = ui->widgetNumericConverter->getHex();
	ui->widgetNumericConverter->setHex(hex);
	mainWindow->lsc.camIOCtrl_setSequenceLength(_drvno, channel, ui->widgetNumericConverter->getSequenceLength().toInt());
	return;
}

void PulseGeneratorWidget::on_checkBoxSequenceOn_checkStateChanged(Qt::CheckState checked)
{
	ui->widgetNumericConverter->setEnabled(checked);
	ui->widgetNumericConverter->setVisible(checked);
	return;
}
