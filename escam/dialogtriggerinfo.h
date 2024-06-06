#ifndef DIALOGTRIGGERINFO_H
#define DIALOGTRIGGERINFO_H

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

#endif // DIALOGTRIGGERINFO_H
