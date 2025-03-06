#pragma once

#include <QDialog>
#include "ui_dialogshutter.h"
#include "lsc-gui.h"

class DialogShutter : public QDialog
{
	Q_OBJECT

public:
	DialogShutter(QWidget *parent = nullptr);
	~DialogShutter();
private slots:
	void on_checkBoxMshut_checkStateChanged(Qt::CheckState checkState);
	void on_checkBoxShutterX_checkStateChanged(Qt::CheckState checkState, int shutterNumber);
private:
	Ui::DialogShutterClass ui;

};
