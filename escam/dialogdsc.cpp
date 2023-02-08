#include "dialogdsc.h"
#include "ui_dialogdsc.h"
#include "lsc-gui.h"

DialogDSC::DialogDSC(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogDSC)
{
	ui->setupUi(this);
	connect(ui->spinBoxBoard, qOverload<int>(&QSpinBox::valueChanged), this, &DialogDSC::updateDSC);
}

DialogDSC::~DialogDSC()
{
	delete ui;
}

void DialogDSC::on_pushButton_RS_DSC_1_clicked()
{
	uint8_t DSCNumber = 1;
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.resetDSC(drvno, DSCNumber);
	return;
}

void DialogDSC::on_pushButton_RS_DSC_2_clicked()
{
	uint8_t DSCNumber = 2;
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.resetDSC(drvno, DSCNumber);
	return;
}

void DialogDSC::on_comboBox_DIR_DSC_1_currentIndexChanged(int index)
{
	uint8_t DSCNumber = 1;
	bool dir = index;
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.setDIRDSC(drvno, DSCNumber, dir);
	return;
}

void DialogDSC::on_comboBox_DIR_DSC_2_currentIndexChanged(int index)
{
	uint8_t DSCNumber = 2;
	bool dir = index;
	uint32_t drvno = ui->spinBoxBoard->value();
	mainWindow->lsc.setDIRDSC(drvno, DSCNumber, dir);
	return;
}

void DialogDSC::updateDSC()
{
	uint32_t ADSC, LDSC;
	QString sADSC, sLDSC;
	uint32_t drvno = ui->spinBoxBoard->value();
	for (uint8_t DSCNumber = 1; DSCNumber <= 2; DSCNumber++)
	{
		//get the DSC values from the registers
		mainWindow->lsc.getDSC(drvno, DSCNumber, &ADSC, &LDSC);
		//convert the numbers to strings
		sADSC = QString::number(ADSC);
		sLDSC = QString::number(LDSC);
		//print the strings to the gui view(labels)
		switch (DSCNumber)
		{
		case 1:
			ui->label_act_DSC_1->setText(sADSC);
			ui->label_last_DSC_1->setText(sLDSC);
			break;
		case 2:
			ui->label_act_DSC_2->setText(sADSC);
			ui->label_last_DSC_2->setText(sLDSC);
			break;
		}
	}
	return;
}


void DialogDSC::on_pushButton_update_cur_clicked()
{
	updateDSC();
	return;
}
