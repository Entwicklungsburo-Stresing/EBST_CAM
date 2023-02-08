#pragma once

#include <QWidget>
#include "lsc-gui.h"
#include "ui_dacspinboxes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DacSpinBoxesClass; };
QT_END_NAMESPACE

class DacSpinBoxes : public QWidget
{
	Q_OBJECT

public:
	DacSpinBoxes(QWidget *parent = nullptr);
	~DacSpinBoxes();
	uint32_t location = 0;
	uint32_t drvno = 0;

public slots:
	void on_accepted();
	void on_rejected();
	void on_default_pressed();
	void initialize();

private slots:
	void spinBox_valueChanged();

private:
	Ui::DacSpinBoxesClass *ui;
	QSettings settings;
	uint32_t output_old[8];
};
