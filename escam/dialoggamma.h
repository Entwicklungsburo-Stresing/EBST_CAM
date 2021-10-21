#pragma once

#include <QDialog>
namespace Ui { class DialogGamma; };

class DialogGamma : public QDialog
{
	Q_OBJECT

public:
	DialogGamma(QWidget *parent = Q_NULLPTR);
	~DialogGamma();

private:
	Ui::DialogGamma *ui;
};
