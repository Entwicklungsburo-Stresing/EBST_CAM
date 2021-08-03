#include "dialogrms.h"
#include "ui_dialogrms.h"

DialogRMS::DialogRMS(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRMS)
{
    ui->setupUi(this);
}

DialogRMS::~DialogRMS()
{
    delete ui;
}
