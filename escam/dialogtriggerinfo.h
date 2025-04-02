/*****************************************************************//**
 * @file		dialogtriggerinfo.h
 * @brief		Dialog for displaying the trigger information.
 * @author		Florian Hahn
 * @date		06.06.2024
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>

namespace Ui {
class DialogTriggerInfo;
}

class DialogTriggerInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTriggerInfo(QWidget *parent = nullptr);
	~DialogTriggerInfo();
public slots:
	void on_measureDone();

private:
	Ui::DialogTriggerInfo *ui;
};
