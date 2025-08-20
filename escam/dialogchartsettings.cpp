/*****************************************************************//**
 * @file   dialogchartsettings.cpp
 * @copydoc dialogchartsettings.h
 *********************************************************************/

#include "dialogchartsettings.h"
#include "lsc-gui.h"

DialogChartSettings::DialogChartSettings(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	QList<QAbstractAxis*> axes = mainWindow->ui->chartView->chart()->axes();
	if(!axes.isEmpty())
	{
		QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
		QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
		xmax_old = axis0->max();
		xmin_old = axis0->min();
		ymax_old = axis1->max();
		ymin_old = axis1->min();
	}
	ui.spinBoxXmax->setValue(xmax_old);
	ui.spinBoxXmin->setValue(xmin_old);
	ui.spinBoxYmax->setValue(ymax_old);
	ui.spinBoxYmin->setValue(ymin_old);
	ui.checkBoxMirrorX->setChecked(settings.value(settingAxesMirrorXPath, settingAxesMirrorXPathDefault).toBool());
	ui.checkBoxShowCrosshair->setChecked(settings.value(settingShowCrosshairPath, settingShowCrosshairDefault).toBool());
	populateCameras();
}

DialogChartSettings::~DialogChartSettings()
{}

void DialogChartSettings::on_spinBoxXmin_valueChanged(int arg1)
{
	QList<QAbstractAxis*> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	axis0->setMin(arg1);
	mainWindow->ui->chartView->curr_xmin = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogChartSettings::on_spinBoxXmax_valueChanged(int arg1)
{
	QList<QAbstractAxis*> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis0 = static_cast<QValueAxis*>(axes[0]);
	axis0->setMax(arg1);
	mainWindow->ui->chartView->curr_xmax = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogChartSettings::on_spinBoxYmin_valueChanged(int arg1)
{
	QList<QAbstractAxis*> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis1->setMin(arg1);
	mainWindow->ui->chartView->curr_ymin = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogChartSettings::on_spinBoxYmax_valueChanged(int arg1)
{
	QList<QAbstractAxis*> axes = mainWindow->ui->chartView->chart()->axes();
	if (axes.isEmpty()) return;
	QValueAxis* axis1 = static_cast<QValueAxis*>(axes[1]);
	axis1->setMax(arg1);
	mainWindow->ui->chartView->curr_ymax = arg1;
	emit spinBoxAxes_valueChanged();
	return;
}

void DialogChartSettings::on_rubberband_valueChanged()
{
	ui.spinBoxXmin->setValue(mainWindow->ui->chartView->curr_xmin);
	ui.spinBoxXmax->setValue(mainWindow->ui->chartView->curr_xmax);
	ui.spinBoxYmin->setValue(mainWindow->ui->chartView->curr_ymin);
	ui.spinBoxYmax->setValue(mainWindow->ui->chartView->curr_ymax);
	return;
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
void DialogChartSettings::on_checkBoxMirrorX_stateChanged(int state)
#else
void DialogChartSettings::on_checkBoxMirrorX_checkStateChanged(Qt::CheckState state)
#endif
{
	(void)state;
	settings.setValue(settingAxesMirrorXPath, ui.checkBoxMirrorX->isChecked());
	mainWindow->loadCameraData();
	return;
}


#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
void DialogChartSettings::on_checkBoxShowCrosshair_stateChanged(int state)
#else
void DialogChartSettings::on_checkBoxShowCrosshair_checkStateChanged(Qt::CheckState state)
#endif
{
	(void)state;
	settings.setValue(settingShowCrosshairPath, ui.checkBoxShowCrosshair->isChecked());
	return;
}

/**
 * @brief This slot opens the settings dialog for selecting the cameras to be displayed on the chart.
 * @return none
 */
void DialogChartSettings::populateCameras()
{
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toDouble();
	uint32_t camcnt = 0;
	for (uint32_t drvno = 0; drvno < mainWindow->lsc.numberOfBoards; drvno++)
	{
		settings.beginGroup("board" + QString::number(drvno));
		if ((board_sel >> drvno) & 1)
		// Check if the drvno'th bit is set
		{
			camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
			// If camcnt is 0, treat as camcnt 1
			if (camcnt == 0)
				camcnt = 1;
			for (uint16_t cam = 0; cam < camcnt; cam++)
			{
				QCheckBox* checkbox = new QCheckBox(("Board " + QString::number(drvno) + ", Camera " + QString::number(cam)), this);
				checkbox->setChecked(settings.value(settingShowCameraBaseDir + QString::number(cam), settingShowCameraDefault).toBool());
				ui.verticalLayoutCameras->addWidget(checkbox);
				// Lambda syntax is used to pass additional argument i
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0))
				connect(checkbox, &QCheckBox::stateChanged, this, [checkbox, this, cam, drvno] {on_checkBoxShowCamera(checkbox->isChecked(), cam, drvno); mainWindow->loadCameraData(); });
#else
				connect(checkbox, &QCheckBox::checkStateChanged, this, [checkbox, this, cam, drvno] {on_checkBoxShowCamera(checkbox->isChecked(), cam, drvno); mainWindow->loadCameraData(); });
#endif
			}
		}
		settings.endGroup();
	}
	return;
}

void DialogChartSettings::on_checkBoxShowCamera(bool state, int camera, uint32_t drvno)
{
	settings.beginGroup("board" + QString::number(drvno));
	settings.setValue(settingShowCameraBaseDir + QString::number(camera), state);
	settings.endGroup();
	return;
}

void DialogChartSettings::on_pushButtonDefault_pressed()
{
	mainWindow->ui->chartView->setDefaultAxes();
	on_rubberband_valueChanged();
}
