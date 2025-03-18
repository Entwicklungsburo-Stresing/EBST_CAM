/*****************************************************************//**
 * @file   dialoggreyscalesettings.cpp
 * @copydoc dialoggreyscalesettings.h
 *********************************************************************/

#include "dialoggreyscalesettings.h"
#include "lsc-gui.h"

DialogGreyscaleSettings::DialogGreyscaleSettings(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogGreyscaleSettingsClass())
{
	ui->setupUi(this);
	// load current values to UI
	ui->spinBoxWhite->setValue(mainWindow->lsc.getGammaWhite());
	ui->spinBoxBlack->setValue(mainWindow->lsc.getGammaBlack());
	ui->spinBoxBoard->setValue(mainWindow->greyscale_viewer_board);
	ui->spinBoxCamera->setValue(mainWindow->greyscale_viewer_camera);
	// set limits to UI
	ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	if (mainWindow->lsc.numberOfBoards == 1)
		ui->spinBoxBoard->setEnabled(false);
	on_spinBoxBoard_valueChanged(mainWindow->greyscale_viewer_board);
}

DialogGreyscaleSettings::~DialogGreyscaleSettings()
{
	delete ui;
}

/**
 * @brief This slots changes the white gamma value in greyscale viewer.
 *
 */
void DialogGreyscaleSettings::on_spinBoxWhite_valueChanged(int value)
{
	mainWindow->lsc.setGammaValue(value, mainWindow->lsc.getGammaBlack());
	return;
}

/**
 * @brief This slots changes the black gamma value in greyscale viewer.
 *
 */
void DialogGreyscaleSettings::on_spinBoxBlack_valueChanged(int value)
{
	mainWindow->lsc.setGammaValue(mainWindow->lsc.getGammaWhite(), value);
	return;
}

void DialogGreyscaleSettings::on_spinBoxBoard_valueChanged(int value)
{
	mainWindow->greyscale_viewer_board = value;
	settings.beginGroup("board" + QString::number(value));
	uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toDouble();
	uint camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toDouble();
	settings.endGroup();
	uint32_t nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	uint32_t block = mainWindow->ui->horizontalSliderBlock->value() - 1;
	mainWindow->lsc.showNewBitmap(value, block, mainWindow->greyscale_viewer_camera, pixelcount, nos);
	// set camcnt limit to UI
	if (camcnt > 0)
		ui->spinBoxCamera->setMaximum(camcnt - 1);
	else
		ui->spinBoxCamera->setMaximum(0);
	if (camcnt <= 1)
		ui->spinBoxCamera->setDisabled(true);
	else
		ui->spinBoxCamera->setDisabled(false);
	return;
}

void DialogGreyscaleSettings::on_spinBoxCamera_valueChanged(int value)
{
	mainWindow->greyscale_viewer_camera = value;
	settings.beginGroup("board" + QString::number(mainWindow->greyscale_viewer_board));
	uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toDouble();
	settings.endGroup();
	uint32_t nos = settings.value(settingNosPath, settingNosDefault).toDouble();
	uint32_t block = mainWindow->ui->horizontalSliderBlock->value() - 1;
	mainWindow->lsc.showNewBitmap(mainWindow->greyscale_viewer_board, block, value, pixelcount, nos);
	return;
}