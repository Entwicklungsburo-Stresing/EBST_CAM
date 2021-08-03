#include "dialogtdc.h"
#include "ui_dialogtdc.h"
#include <QMessageBox>

#include "lsc-gui.h"

DialogTDC::DialogTDC(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTDC)
{
    ui->setupUi(this);
}

DialogTDC::~DialogTDC()
{
    delete ui;
}

void DialogTDC::updateTDC(uint32_t tdc1, uint32_t tdc2)
{
	//pixel 6low/7high of tdc1 and 8low/9high of tdc2 to tdc view
	QString stdc1 = QString::number(tdc1);
	QString stdc2 = QString::number(tdc2);
	ui->viewTDC1->setText(stdc1);
	ui->viewTDC2->setText(stdc2);
}
