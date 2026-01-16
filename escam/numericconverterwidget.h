#pragma once

#include <QWidget>
#include "ui_numericconverterwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NumericConverterWidgetClass; };
QT_END_NAMESPACE

class NumericConverterWidget : public QWidget
{
	Q_OBJECT

public:
	NumericConverterWidget(QWidget *parent = nullptr);
	~NumericConverterWidget();
	void setDecimal(QString dec);
	QString getDecimal();
	void setHex(QString hex);
	QString getHex();
	void setBinary(QString bin);
	QString getBinary();
	void setSequenceLength(int len);
	QString getSequenceLength();
private slots:
	void on_spinBoxSeqLength_valueChanged(int val);
	void on_lineEditDec_textChanged();
	void on_lineEditHex_textChanged();
	void on_lineEditBin_textChanged();
	void on_lineEditBin_editingFinished();
private:
	Ui::NumericConverterWidgetClass *ui;
	QString convertDecimalToBinary(QString decimalString);
	QString convertHexToBinary(QString hexString);
	QString convertBinaryToDecimal(QString binaryString);
	QString convertBinaryToHex(QString binaryString);
	QString addLeadingZerosToBin(QString bin);
};

