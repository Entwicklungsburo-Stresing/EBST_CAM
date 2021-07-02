#include "dialogtdc.h"
#include "ui_dialogtdc.h"
#include <QMessageBox>

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

void DialogTDC::updateTDC()
{
	QMessageBox* d = new QMessageBox( this );
	d->setWindowTitle( "TestTDC" );
	d->setWindowModality( Qt::ApplicationModal );
	//d->showMessage( tr( (char*)ConvertErrorCodeToMsg( status ) ) );
	//pixel6/7 to tdc view
}
