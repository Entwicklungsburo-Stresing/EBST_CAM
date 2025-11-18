#pragma once

#include <QWidget>
#include "ui_ioctrlwidget.h"
#include <QSettings>

class IoctrlWidget : public QWidget
{
	Q_OBJECT

public:
	IoctrlWidget(QWidget *parent = nullptr);
	~IoctrlWidget();
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
	void on_spinBoxWidth_valueChanged(int val);
	void on_spinBoxDelay_valueChanged(int val);
private:
	QString convertDecimalToBinary(QString decimalString);
	QString convertHexToBinary(QString hexString);
	QString convertBinaryToDecimal(QString binaryString);
	QString convertBinaryToHex(QString binaryString);
	QString addLeadingZerosToBin(QString bin);
	Ui::IoctrlWidgetClass ui;
	QSettings settings;
	uint32_t _drvno = 0;
	void sendSequence();
	void sendAllSettings();
};

