#include "dialoggamma.h"
#include "ui_dialoggamma.h"
#include "shared_src/ESLSCDLL_pro.h"

DialogGamma::DialogGamma(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui::DialogGamma();
	ui->setupUi(this);
	white_old = DLLGetGammaWhite();
	black_old = DLLGetGammaBlack();
	ui->spinBoxWhite->setValue( white_old );
	ui->spinBoxBlack->setValue( black_old );
}

DialogGamma::~DialogGamma()
{
	delete ui;
}

/**
 * \brief This slots changes the white gamma value in greyscale viewer.
 * 
 */
void DialogGamma::on_spinBoxWhite_valueChanged(int value)
{
	DLLSetGammaValue( value, DLLGetGammaBlack() );
	return;
}

/**
 * \brief This slots changes the black gamma value in greyscale viewer.
 *
 */
void DialogGamma::on_spinBoxBlack_valueChanged(int value)
{
	DLLSetGammaValue( DLLGetGammaWhite(), value );
	return;
}

void DialogGamma::on_buttonBox_rejected()
{
	DLLSetGammaValue( white_old, black_old );
	return;
}