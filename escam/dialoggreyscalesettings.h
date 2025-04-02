/*****************************************************************//**
 * @file		dialoggreyscalesettings.h
 * @brief		Dialog to set the gamma values for the greyscale viewer.
 * @author		Florian Hahn
 * @date		09.03.2023
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is release under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "ui_dialoggreyscalesettings.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogGreyscaleSettingsClass; };
QT_END_NAMESPACE

class DialogGreyscaleSettings : public QDialog
{
	Q_OBJECT

public:
	DialogGreyscaleSettings(QWidget *parent = nullptr);
	~DialogGreyscaleSettings();

private:
	Ui::DialogGreyscaleSettingsClass *ui;
	QSettings settings;

private slots:
	void on_spinBoxWhite_valueChanged(int value);
	void on_spinBoxBlack_valueChanged(int value);
	void on_spinBoxBoard_valueChanged(int value);
	void on_spinBoxCamera_valueChanged(int value);
};
