/*****************************************************************//**
 * @file		ioctrlwidget.h
 * @brief		Widget for dialog ioctrl.
 * @author		Florian Hahn
 * @date		13.11.2025
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released under the GPL-3.0.
 *********************************************************************/

#pragma once

#include <QWidget>
#include "ui_ioctrlwidget.h"
#include <QSettings>

class IoctrlWidget : public QWidget
{
	Q_OBJECT

public:
	IoctrlWidget(QWidget *parent = nullptr);
	~IoctrlWidget();
	int channel = 0;
public slots:
	void loadDefaults();
	void loadSettings(int drvno);
private slots:
	void sequenceLengthChanged();
	void on_lineEditDelay_editingFinished();
	void on_lineEditWidth_editingFinished();
private:
	Ui::IoctrlWidgetClass ui;
	QSettings settings;
	uint32_t _drvno = 0;
	void sendSequence();
	void sendAllSettings();
	struct TimeResult {
		long long ns;
		QString formatted;
		bool valid;
	};
	TimeResult processTimeInput(const QString& input, long long maxNs);
	void delayChanged(int delay);
	void widthChanged(int width);
	long long delayLimit = 2097120;
	long long widthLimit = 2097120;
	QString formatNsToHumanReadable(long long ns);
};

