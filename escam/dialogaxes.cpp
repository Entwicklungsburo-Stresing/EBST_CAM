#include "dialogaxes.h"
#include "ui_dialogaxes.h"
#include "lsc-gui.h"

DialogAxes::DialogAxes(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogAxes)
{
	ui->setupUi(this);
	QList<QAbstractAxis *> axes = mainWindow->ui->chartView->chart()->axes();
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	xmax_old = axis0->max();
	xmin_old = axis0->min();
	ymax_old = axis1->max();
	ymin_old = axis1->min();
	ui->spinBoxXmax->setValue(xmax_old);
	ui->spinBoxXmin->setValue(xmin_old);
	ui->spinBoxYmax->setValue(ymax_old);
	ui->spinBoxYmin->setValue(ymin_old);
	connect(ui->checkBoxMirrorX, &QCheckBox::stateChanged, mainWindow, &MainWindow::loadCameraData);
	ui->checkBoxMirrorX->setChecked(settings.value(settingAxesMirrorXPath, settingAxesMirrorXPathDefault).toBool());
}

DialogAxes::~DialogAxes()
{
    delete ui;
}

void DialogAxes::on_buttonBox_rejected()
{
	QList<QAbstractAxis *> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis0->setMax(xmax_old);
	axis0->setMin(xmin_old);
	axis1->setMax(ymax_old);
	axis1->setMin(ymin_old);
	mainWindow->ui->chartView->curr_xmax = xmax_old;
	mainWindow->ui->chartView->curr_xmin = xmin_old;
	mainWindow->ui->chartView->curr_ymax = ymax_old;
	mainWindow->ui->chartView->curr_ymin = ymin_old;
	return;
}

void DialogAxes::on_spinBoxXmin_valueChanged(int arg1)
{
	QList<QAbstractAxis *> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	axis0->setMin(arg1);
	mainWindow->ui->chartView->curr_xmin = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogAxes::on_spinBoxXmax_valueChanged(int arg1)
{
	QList<QAbstractAxis *> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	axis0->setMax(arg1);
	mainWindow->ui->chartView->curr_xmax = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogAxes::on_spinBoxYmin_valueChanged(int arg1)
{
	QList<QAbstractAxis *> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis1->setMin(arg1);
	mainWindow->ui->chartView->curr_ymin = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogAxes::on_spinBoxYmax_valueChanged(int arg1)
{
	QList<QAbstractAxis *> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis1->setMax(arg1);
	mainWindow->ui->chartView->curr_ymax = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogAxes::on_rubberband_valueChanged()
{
	ui->spinBoxXmin->setValue(mainWindow->ui->chartView->curr_xmin);
	ui->spinBoxXmax->setValue(mainWindow->ui->chartView->curr_xmax);
	ui->spinBoxYmin->setValue(mainWindow->ui->chartView->curr_ymin);
	ui->spinBoxYmax->setValue(mainWindow->ui->chartView->curr_ymax);
	return;
}

void DialogAxes::on_checkBoxMirrorX_stateChanged(int state)
{
	settings.setValue(settingAxesMirrorXPath, ui->checkBoxMirrorX->isChecked());
	return;
}