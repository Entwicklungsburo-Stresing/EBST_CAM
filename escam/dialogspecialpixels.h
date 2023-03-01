#pragma once

#include <QDialog>
#include "ui_dialogspecialpixels.h"
#include "dialogsettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogSpecialPixelsClass; };
QT_END_NAMESPACE

class DialogSpecialPixels : public QDialog
{
	Q_OBJECT

public:
	DialogSpecialPixels(QWidget *parent = nullptr);
	~DialogSpecialPixels();
public slots:
	void updateValues();
	void updateSample(int sample);
	void updateBlock(int block);

private:
	Ui::DialogSpecialPixelsClass *ui;
	uint32_t _sample = 0;
	uint32_t _block = 0;
	QSettings settings;
private slots:
	void on_spinBoxBoard_valueChanged(int index);
};
