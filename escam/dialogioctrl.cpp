/*****************************************************************//**
 * @file   dialogioctrl.cpp
 * @copydoc dialogioctrl.h
 *********************************************************************/

#include "dialogioctrl.h"
#include "ui_dialogioctrl.h"
#include "lsc-gui.h"

DialogIoctrl::DialogIoctrl(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::DialogIoctrl)
{
	ui->setupUi(this);
	// Connect spin boxes to slots
	// Lambda syntax is used to pass additional arguments
	
	if (mainWindow->lsc.numberOfBoards > 1)
		ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	// apply saved values to UI

}

DialogIoctrl::~DialogIoctrl()
{
}

