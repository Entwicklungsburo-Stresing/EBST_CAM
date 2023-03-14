#include "dialogdac.h"
#include "ui_dialogdac.h"
#include "dacspinboxes.h"

DialogDac::DialogDac(QWidget* parent)
	: QDialog(parent),
	ui(new Ui::DialogDac)
{
	ui->setupUi(this);

	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesCameraBoard_0, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesCameraBoard_1, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesCameraBoard_2, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesCameraBoard_3, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesCameraBoard_4, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesPcieBoard_0, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesPcieBoard_1, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesPcieBoard_2, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesPcieBoard_3, &DacSpinBoxes::initialize);
	connect(this, &DialogDac::initializingDone, ui->dacSpinBoxesPcieBoard_4, &DacSpinBoxes::initialize);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesCameraBoard_0, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesCameraBoard_1, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesCameraBoard_2, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesCameraBoard_3, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesCameraBoard_4, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesPcieBoard_0, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesPcieBoard_1, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesPcieBoard_2, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesPcieBoard_3, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->dacSpinBoxesPcieBoard_4, &DacSpinBoxes::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesCameraBoard_0, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesCameraBoard_1, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesCameraBoard_2, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesCameraBoard_3, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesCameraBoard_4, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesPcieBoard_0, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesPcieBoard_1, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesPcieBoard_2, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesPcieBoard_3, &DacSpinBoxes::on_rejected);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, ui->dacSpinBoxesPcieBoard_4, &DacSpinBoxes::on_rejected);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesCameraBoard_0, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesCameraBoard_1, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesCameraBoard_2, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesCameraBoard_3, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesCameraBoard_4, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesPcieBoard_0, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesPcieBoard_1, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesPcieBoard_2, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesPcieBoard_3, &DacSpinBoxes::on_default_pressed);
	connect(ui->pushButtonDefault, &QPushButton::pressed, ui->dacSpinBoxesPcieBoard_4, &DacSpinBoxes::on_default_pressed);

	// initialize spin box widgets
	ui->dacSpinBoxesCameraBoard_0->location = DAC8568_camera;
	ui->dacSpinBoxesCameraBoard_1->location = DAC8568_camera;
	ui->dacSpinBoxesCameraBoard_2->location = DAC8568_camera;
	ui->dacSpinBoxesCameraBoard_3->location = DAC8568_camera;
	ui->dacSpinBoxesCameraBoard_4->location = DAC8568_camera;
	ui->dacSpinBoxesPcieBoard_0->location = DAC8568_pcie;
	ui->dacSpinBoxesPcieBoard_1->location = DAC8568_pcie;
	ui->dacSpinBoxesPcieBoard_2->location = DAC8568_pcie;
	ui->dacSpinBoxesPcieBoard_3->location = DAC8568_pcie;
	ui->dacSpinBoxesPcieBoard_4->location = DAC8568_pcie;
	ui->dacSpinBoxesCameraBoard_0->drvno = 0;
	ui->dacSpinBoxesCameraBoard_1->drvno = 1;
	ui->dacSpinBoxesCameraBoard_2->drvno = 2;
	ui->dacSpinBoxesCameraBoard_3->drvno = 3;
	ui->dacSpinBoxesCameraBoard_4->drvno = 4;
	ui->dacSpinBoxesPcieBoard_0->drvno = 0;
	ui->dacSpinBoxesPcieBoard_1->drvno = 1;
	ui->dacSpinBoxesPcieBoard_2->drvno = 2;
	ui->dacSpinBoxesPcieBoard_3->drvno = 3;
	ui->dacSpinBoxesPcieBoard_4->drvno = 4;

	// hide all spin box widgets
	ui->dacSpinBoxesCameraBoard_0->setVisible(false);
	ui->dacSpinBoxesCameraBoard_1->setVisible(false);
	ui->dacSpinBoxesCameraBoard_2->setVisible(false);
	ui->dacSpinBoxesCameraBoard_3->setVisible(false);
	ui->dacSpinBoxesCameraBoard_4->setVisible(false);
	ui->dacSpinBoxesPcieBoard_0->setVisible(false);
	ui->dacSpinBoxesPcieBoard_1->setVisible(false);
	ui->dacSpinBoxesPcieBoard_2->setVisible(false);
	ui->dacSpinBoxesPcieBoard_3->setVisible(false);
	ui->dacSpinBoxesPcieBoard_4->setVisible(false);
	ui->dacSpinBoxesCameraBoard_0->setEnabled(false);
	ui->dacSpinBoxesCameraBoard_1->setEnabled(false);
	ui->dacSpinBoxesCameraBoard_2->setEnabled(false);
	ui->dacSpinBoxesCameraBoard_3->setEnabled(false);
	ui->dacSpinBoxesCameraBoard_4->setEnabled(false);
	ui->dacSpinBoxesPcieBoard_0->setEnabled(false);
	ui->dacSpinBoxesPcieBoard_1->setEnabled(false);
	ui->dacSpinBoxesPcieBoard_2->setEnabled(false);
	ui->dacSpinBoxesPcieBoard_3->setEnabled(false);
	ui->dacSpinBoxesPcieBoard_4->setEnabled(false);
	uint32_t board_sel = settings.value(settingBoardSelPath, settingBoardSelDefault).toUInt();
	// only show needed spin box widgets
	switch (number_of_boards)
	{
	case 5:
		ui->dacSpinBoxesCameraBoard_4->setVisible(true);
		ui->dacSpinBoxesPcieBoard_4->setVisible(true);
		if ((board_sel >> 4) & 1)
		{
			ui->dacSpinBoxesCameraBoard_4->setEnabled(true);
			ui->dacSpinBoxesPcieBoard_4->setEnabled(true);
		}
	case 4:
		ui->dacSpinBoxesCameraBoard_3->setVisible(true);
		ui->dacSpinBoxesPcieBoard_3->setVisible(true);
		if ((board_sel >> 3) & 1)
		{
			ui->dacSpinBoxesCameraBoard_3->setEnabled(true);
			ui->dacSpinBoxesPcieBoard_3->setEnabled(true);
		}
	case 3:
		ui->dacSpinBoxesCameraBoard_2->setVisible(true);
		ui->dacSpinBoxesPcieBoard_2->setVisible(true);
		if ((board_sel >> 2) & 1)
		{
			ui->dacSpinBoxesCameraBoard_2->setEnabled(true);
			ui->dacSpinBoxesPcieBoard_2->setEnabled(true);
		}
	case 2:
		ui->dacSpinBoxesCameraBoard_1->setVisible(true);
		ui->dacSpinBoxesPcieBoard_1->setVisible(true);
		if ((board_sel >> 1) & 1)
		{
			ui->dacSpinBoxesCameraBoard_1->setEnabled(true);
			ui->dacSpinBoxesPcieBoard_1->setEnabled(true);
		}
	case 1:
		ui->dacSpinBoxesCameraBoard_0->setVisible(true);
		ui->dacSpinBoxesPcieBoard_0->setVisible(true);
		if (board_sel & 1)
		{
			ui->dacSpinBoxesCameraBoard_0->setEnabled(true);
			ui->dacSpinBoxesPcieBoard_0->setEnabled(true);
		}
	}
	emit initializingDone();
}

DialogDac::~DialogDac()
{
}
