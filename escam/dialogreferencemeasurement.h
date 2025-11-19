/*****************************************************************//**
 * @file		dialogreferencemeasurement.h
 * @brief		Dialog to create reference measurements.
 * @author		Dennis Vollenweider
 * @date		17.11.2025
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released under the LPGL-3.0.
 *********************************************************************/

#pragma once

#include <QDialog>
#include "ui_dialogreferencemeasurement.h"
#include "lsc-gui.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogReferenceMeasurementClass; };
QT_END_NAMESPACE

class DialogReferenceMeasurement : public QDialog
{
	Q_OBJECT

public:
	DialogReferenceMeasurement(QWidget* parent = nullptr);
	~DialogReferenceMeasurement();

private:
	Ui::DialogReferenceMeasurementClass* ui;
	QSettings settings;
	QList<QLineSeries*> referenceSeriesList;
	void initDialog();
	void handleReference(QString id);
	void saveReference(QString seriesName);
	void clearReference(QString seriesName);
	void loadReferenceButtonState();

private slots:
	void on_spinBoxBoard_valueChanged();
	void on_spinBoxCamera_valueChanged();
	void on_pushButtonHandleReference1_pressed();
	void on_pushButtonHandleReference2_pressed();
};

