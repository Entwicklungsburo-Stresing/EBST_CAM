#pragma once

#include <QDialog>
#include "ui_dialogfancontrol.h"

class DialogFanControl : public QDialog
{
	Q_OBJECT

public:
	DialogFanControl(QWidget *parent = nullptr);
	~DialogFanControl();

private:
	Ui::DialogFanControlClass ui;
};

