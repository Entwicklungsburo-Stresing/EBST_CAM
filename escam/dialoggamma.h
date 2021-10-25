#pragma once

#include <QDialog>
namespace Ui { class DialogGamma; };

class DialogGamma : public QDialog
{
	Q_OBJECT

public:
	DialogGamma(QWidget *parent = Q_NULLPTR);
	~DialogGamma();

private:
	Ui::DialogGamma *ui;
	uint16_t white_old;
	uint16_t black_old;
private slots:
	void on_spinBoxWhite_valueChanged(int value);
	void on_spinBoxBlack_valueChanged(int value);
	void on_buttonBox_rejected();
};
