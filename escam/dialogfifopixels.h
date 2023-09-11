#pragma once

#include <QDialog>
#include "ui_dialogfifopixels.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogFifoPixelsClass; };
QT_END_NAMESPACE

class DialogFifoPixels : public QDialog
{
	Q_OBJECT

public:
	DialogFifoPixels(QWidget *parent = nullptr);
	~DialogFifoPixels();

private:
	Ui::DialogFifoPixelsClass *ui;
	void updateValues();
};
