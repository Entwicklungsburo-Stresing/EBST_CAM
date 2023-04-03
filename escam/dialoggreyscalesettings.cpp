#include "dialoggreyscalesettings.h"
#include "shared_src/ESLSCDLL_pro.h"
#include "lsc-gui.h"

DialogGreyscaleSettings::DialogGreyscaleSettings(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogGreyscaleSettingsClass())
{
	ui->setupUi(this);
	// load current values to UI
	ui->spinBoxWhite->setValue(DLLGetGammaWhite());
	ui->spinBoxBlack->setValue(DLLGetGammaBlack());
	ui->spinBoxBoard->setValue(mainWindow->greyscale_viewer_board);
	ui->spinBoxCamera->setValue(mainWindow->greyscale_viewer_camera);
	// set limits to UI
	ui->spinBoxBoard->setMaximum(number_of_boards - 1);
	if (number_of_boards == 1)
		ui->spinBoxBoard->setEnabled(false);
	on_spinBoxBoard_valueChanged(mainWindow->greyscale_viewer_board);
}

DialogGreyscaleSettings::~DialogGreyscaleSettings()
{
	delete ui;
}

/**
 * \brief This slots changes the white gamma value in greyscale viewer.
 *
 */
void DialogGreyscaleSettings::on_spinBoxWhite_valueChanged(int value)
{
	DLLSetGammaValue(value, DLLGetGammaBlack());
	return;
}

/**
 * \brief This slots changes the black gamma value in greyscale viewer.
 *
 */
void DialogGreyscaleSettings::on_spinBoxBlack_valueChanged(int value)
{
	DLLSetGammaValue(DLLGetGammaWhite(), value);
	return;
}

void DialogGreyscaleSettings::on_spinBoxBoard_valueChanged(int value)
{
	mainWindow->greyscale_viewer_board = value;
	settings.beginGroup("board" + QString::number(value));
	uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toUInt();
	uint camcnt = settings.value(settingCamcntPath, settingCamcntDefault).toUInt();
	settings.endGroup();
	uint32_t nos = settings.value(settingNosPath, settingNosDefault).toUInt();
	uint32_t block = mainWindow->ui->horizontalSliderBlock->value() - 1;
	DLLShowNewBitmap(value, block, mainWindow->greyscale_viewer_camera, pixelcount, nos);
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
	uint16_t pixelcount = settings.value(settingPixelPath, settingPixelDefault).toUInt();
	settings.endGroup();
	uint32_t nos = settings.value(settingNosPath, settingNosDefault).toUInt();
	uint32_t block = mainWindow->ui->horizontalSliderBlock->value() - 1;
	DLLShowNewBitmap(mainWindow->greyscale_viewer_board, block, value, pixelcount, nos);
	return;
}