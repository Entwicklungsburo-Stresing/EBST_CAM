#ifndef DIALOGDAC_H
#define DIALOGDAC_H

#include <QDialog>
#include "lsc-gui.h"

namespace Ui {
	class DialogDac;
}

class DialogDac : public QDialog
{
	Q_OBJECT

public:
	DialogDac(QWidget *parent = Q_NULLPTR);
	~DialogDac();

private slots:
	void spinBoxChannelX_valueChanged();
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_pushButtonDefault_pressed();
private:
	Ui::DialogDac *ui;
	QSettings settings;
	uint32_t output_old[2][8];
};

#endif // DIALOGDAC_H
