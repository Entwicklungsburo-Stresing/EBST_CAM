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
signals:
	void initializingDone();
private:
	Ui::DialogDac *ui;
	QSettings settings;
};

#endif // DIALOGDAC_H
