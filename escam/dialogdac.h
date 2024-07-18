#ifndef DIALOGDAC_H
#define DIALOGDAC_H

#include <QDialog>
#include "lsc-gui.h"

namespace Ui {
	class DialogDac;
}

class DialogDac : public QDialog
{
	Q_OBJECT

public:
	DialogDac(QWidget *parent = Q_NULLPTR);
	~DialogDac();

private slots:
	void spinBoxChannel_valueChanged();
	void loadSettings();
	void on_pushButtonDefault_pressed();
	void on_pushButtonAutotune_pressed();
	void on_autotuneStateChanged();
	void reject();
	void checkTargetReached();

private:
	uint32_t output_old[8];
	Ui::DialogDac *ui;
	QSettings settings;
	bool autotuneRunning = false;
	bool allTargetsReached = false, ch1TargetReached = false, ch2TargetReached = false, ch3TargetReached = false, ch4TargetReached = false, ch5TargetReached = false, ch6TargetReached = false, ch7TargetReached = false, ch8TargetReached = false;
	void autotunePressed();
	double calculateMean(uint16_t* data, int start, int end, bool isHsIr);
	bool autotuneAdjust(uint16_t* data, int start, int end, QSpinBox* spinBox, bool isHsir);
};

#endif // DIALOGDAC_H
