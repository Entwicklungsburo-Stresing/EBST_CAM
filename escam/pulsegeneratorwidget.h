#pragma once

#include <QWidget>
#include "ui_pulsegeneratorwidget.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class PulseGeneratorWidgetClass; };
QT_END_NAMESPACE

class PulseGeneratorWidget : public QWidget
{
	Q_OBJECT

public:
	PulseGeneratorWidget(QWidget *parent = nullptr);
	~PulseGeneratorWidget();
	int channel = 0;
public slots:
	void loadDefaults();
	void loadSettings(int drvno);
private slots:
	void on_spinBoxSeqLength_valueChanged(int val);
	void on_lineEditDec_textChanged();
	void on_lineEditHex_textChanged();
	void on_lineEditBin_textChanged();
	void on_lineEditBin_editingFinished();
	void on_checkBoxSequenceOn_checkStateChanged(Qt::CheckState checked);
	void on_doubleSpinBoxDelaySeconds_valueChanged(double val);
	void on_doubleSpinBoxDelayMilliseconds_valueChanged(double val);
	void on_doubleSpinBoxDelayMicroseconds_valueChanged(double val);
	void on_doubleSpinBoxWidthSeconds_valueChanged(double val);
	void on_doubleSpinBoxWidthMilliseconds_valueChanged(double val);
	void on_doubleSpinBoxWidthMicroseconds_valueChanged(double val);
private:
	Ui::PulseGeneratorWidgetClass *ui;
	QString convertDecimalToBinary(QString decimalString);
	QString convertHexToBinary(QString hexString);
	QString convertBinaryToDecimal(QString binaryString);
	QString convertBinaryToHex(QString binaryString);
	QString addLeadingZerosToBin(QString bin);
	QSettings settings;
	uint32_t _drvno = 0;
	void sendSequence();
	void sendAllSettings();
	void widthChanged(int val);
	void delayChanged(int val);
};

