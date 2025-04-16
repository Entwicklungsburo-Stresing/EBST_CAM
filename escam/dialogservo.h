#pragma once

#include <QDialog>
#include "ui_dialogservo.h"

class DialogServo : public QDialog
{
	Q_OBJECT

public:
	DialogServo(QWidget *parent = nullptr);
	~DialogServo();

private:
	Ui::dialogservoClass ui;
};
