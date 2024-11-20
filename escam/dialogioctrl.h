#pragma once
#include <QDialog>
#include "lsc-gui.h"

namespace Ui {
	class DialogIoctrl;
}

class DialogIoctrl : public QDialog
{
	Q_OBJECT

public:
	DialogIoctrl(QWidget *parent = Q_NULLPTR);
	~DialogIoctrl();

private slots:
	void setOutput(uint8_t outputNumber, uint16_t width_in_5ns, uint16_t delay_in_5ns);
	void setT0(uint32_t period_in_10ns);
	void on_buttonBox_rejected();
	void on_buttonBox_accepted();
	void on_pushButtonDefault_pressed();
	void on_spinBoxBoard_valueChanged();
private:
	Ui::DialogIoctrl *ui;
	QSettings settings;
	uint16_t outputWidth_old[7];
	uint16_t outputDelay_old[7];
	uint32_t t0_old;
};
