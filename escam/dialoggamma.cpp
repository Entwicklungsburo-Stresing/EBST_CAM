#include "dialoggamma.h"
#include "ui_dialoggamma.h"

DialogGamma::DialogGamma(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::DialogGamma();
	ui->setupUi(this);
}

DialogGamma::~DialogGamma()
{
	delete ui;
}
