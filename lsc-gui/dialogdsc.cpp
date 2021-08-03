#include "dialogdsc.h"
#include "ui_dialogdsc.h"

DialogDSC::DialogDSC(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDSC)
{
    ui->setupUi(this);
}

DialogDSC::~DialogDSC()
{
    delete ui;
}
