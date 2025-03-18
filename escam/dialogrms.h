/*****************************************************************//**
 * @file   dialogrms.h
 * @brief  Dialog calculating the RMS value of the camera data.
 * 
 * @author Florian Hahn
 * @date   03.08.2021
 *********************************************************************/

#pragma once

#include <QDialog>
#include "dialogsettings.h"

namespace Ui {
class DialogRMS;
}

class DialogRMS : public QDialog
{
	Q_OBJECT

public:
	explicit DialogRMS(QWidget *parent = nullptr);
	~DialogRMS();
	void initDialogRMS();

public slots:
	void updateRMS();

private:
	Ui::DialogRMS *ui;
	QSettings settings;

private slots:
	void on_spinBox_firstsample_valueChanged(int value);
	void on_spinBox_lastsample_valueChanged(int value);
	void on_spinBoxBoard_valueChanged(int index);
};
