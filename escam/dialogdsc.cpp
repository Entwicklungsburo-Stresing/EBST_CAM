#include "dialogdsc.h"
#include "ui_dialogdsc.h"
#include "lsc-gui.h"

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



void DialogDSC::on_pushButton_RS_DSC_1_clicked()
{
	uint8_t DSCNumber = 1;
	mainWindow->lsc.resetDSC( 1, DSCNumber );
	//for two boards
	if(number_of_boards == 2)
		mainWindow->lsc.resetDSC( 2, DSCNumber );
}

void DialogDSC::on_pushButton_RS_DSC_2_clicked()
{
	uint8_t DSCNumber = 2;
	mainWindow->lsc.resetDSC( 1, DSCNumber );
	//for two boards
	if (number_of_boards == 2)
		mainWindow->lsc.resetDSC( 2, DSCNumber );
}

void DialogDSC::on_pushButton_RS_DSC_3_clicked()
{
	uint8_t DSCNumber = 3;
	mainWindow->lsc.resetDSC( 1, DSCNumber );
	//for two boards
	if (number_of_boards == 2)
		mainWindow->lsc.resetDSC( 2, DSCNumber );
}

void DialogDSC::on_comboBox_DIR_DSC_1_currentIndexChanged(int index)
{
	uint8_t DSCNumber = 1;
    bool dir = index;

	mainWindow->lsc.setDIRDSC( 1, DSCNumber, dir);
	//for two boards
	if (number_of_boards == 2)
		mainWindow->lsc.setDIRDSC(2, DSCNumber, dir );
}

void DialogDSC::on_comboBox_DIR_DSC_2_currentIndexChanged(int index)
{
	uint8_t DSCNumber = 2;
    bool dir = index;

	mainWindow->lsc.setDIRDSC( 1, DSCNumber, dir );
	//for two boards
	if (number_of_boards == 2)
		mainWindow->lsc.setDIRDSC( 2, DSCNumber, dir );
}

void DialogDSC::on_comboBox_DIR_DSC_3_currentIndexChanged(int index)
{
	uint8_t DSCNumber = 3;
    bool dir = index;

	mainWindow->lsc.setDIRDSC( 1, DSCNumber, dir );
	//for two boards
	if (number_of_boards == 2)
		mainWindow->lsc.setDIRDSC( 2, DSCNumber, dir );
}


void DialogDSC::updateDSC()
{
	uint32_t ADSC, LDSC;
	QString sADSC, sLDSC;
	for (uint8_t DSCNumber = 1; DSCNumber <= 3; DSCNumber++)
	{
		//get the DSC values from the registers
		mainWindow->lsc.getDSC( 1, DSCNumber, &ADSC, &LDSC );
		//convert the numbers to strings
		sADSC = QString::number( ADSC );
		sLDSC = QString::number( LDSC );
		//print the strings to the gui view(labels)
		switch (DSCNumber)
		{
		case 1:
			ui->label_act_DSC_1->setText( sADSC );
			ui->label_last_DSC_1->setText( sLDSC );
			break;
		case 2:
			ui->label_act_DSC_2->setText( sADSC );
			ui->label_last_DSC_2->setText( sLDSC );
			break;
		case 3:
			ui->label_act_DSC_3->setText( sADSC );
			ui->label_last_DSC_3->setText( sLDSC );
			break;

		}

		//not activated: for two boards
		//if (number_of_boards == 2)
			//mainWindow->lsc.getDSC( 2, DSCNumber, &ADSC, &LDSC );
	}

}


void DialogDSC::on_pushButton_update_act_clicked()
{
	updateDSC();
}
