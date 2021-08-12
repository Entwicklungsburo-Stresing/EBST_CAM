#include "dialogrms.h"
#include "ui_dialogrms.h"
#include "lsc-gui.h"

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



void DialogRMS::updateRMS()
{
	//get values from ui
	uint32_t firstSample = ui->spinBox_firstsample->value();
	uint32_t lastSample = ui->spinBox_lastsample->value();
	uint32_t pixel = ui->spinBox_pixel->value();
	double mwf, trms;
	QString smwf, strms;

	//calculate trms
	mainWindow->lsc.calcTRMS( 1, firstSample, lastSample, pixel, 0, &mwf, &trms );
	//convert the numbers to strings
	smwf = QString::number( mwf );
	strms = QString::number( trms );
	//show values
	ui->label_mwf->setText( smwf );
	ui->label_trms->setText( strms );



	//not activated: for two boards or two cams
	//if (number_of_boards == 2)
		//mainWindow->lsc.calcTRMS( 1, firstSample, lastSample, pixel, 0, &mwf, &trms );
	

}


void DialogRMS::on_pushButton_update_clicked()
{
	updateRMS();
}
