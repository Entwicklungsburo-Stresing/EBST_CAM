#pragma once

#include <QDialog>
#include "ui_dialogpulsegenerator.h"
#include "lsc-gui.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DialogPulseGeneratorClass; };
QT_END_NAMESPACE

class DialogPulseGenerator : public QDialog
{
	Q_OBJECT

public:
	DialogPulseGenerator(QWidget *parent = nullptr);
	~DialogPulseGenerator();

signals:
	void settingsLoaded(int drvno);
	void defaults_loaded();
private:
	Ui::DialogPulseGeneratorClass *ui;
	QSettings settings;
private slots:
	void on_comboBoxTrigSource_currentIndexChanged(int index);
	void loadSettings();
	void on_spinBoxBoard_valueChanged(int value);
	void on_pushButtonDefault_pressed();
	void on_pushButtonTriggerManually_pressed();
};

