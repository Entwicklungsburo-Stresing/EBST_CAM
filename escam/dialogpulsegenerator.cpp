#include "dialogpulsegenerator.h"

DialogPulseGenerator::DialogPulseGenerator(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DialogPulseGeneratorClass())
{
	ui->setupUi(this);
	if (mainWindow->lsc.numberOfBoards > 1)
		ui->spinBoxBoard->setMaximum(mainWindow->lsc.numberOfBoards - 1);
	else
	{
		ui->spinBoxBoard->setVisible(false);
		ui->labelBoard->setVisible(false);
	}
	// setup ioctrlWidgets
	ui->ch_1->channel = 1;
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_1, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_2, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_3, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_4, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_5, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_6, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_7, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::defaults_loaded, ui->ch_8, &PulseGeneratorWidget::loadDefaults);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_1, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_2, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_3, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_4, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_5, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_6, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_7, &PulseGeneratorWidget::loadSettings);
	connect(this, &DialogPulseGenerator::settingsLoaded, ui->ch_8, &PulseGeneratorWidget::loadSettings);

	loadSettings();
}

DialogPulseGenerator::~DialogPulseGenerator()
{
	delete ui;
}

void DialogPulseGenerator::on_comboBoxTrigSource_currentIndexChanged(int index)
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingTriggerSourcePath, index);
	settings.endGroup();
	mainWindow->lsc.setStateControlRegister(ui->spinBoxBoard->value(), index);
	if (index == statectrl_trigger_select_manual)
		ui->pushButtonTriggerManually->setEnabled(true);
	else
		ui->pushButtonTriggerManually->setEnabled(false);
	return;
}

void DialogPulseGenerator::loadSettings()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	int index = settings.value(settingTriggerSourcePath, settingTriggerSourceDefault).toInt();
	settings.endGroup();
	ui->comboBoxTrigSource->setCurrentIndex(index);
	emit settingsLoaded((ui->spinBoxBoard->value()));
	return;
}

void DialogPulseGenerator::on_spinBoxBoard_valueChanged(int value)
{
	(void)value;
	loadSettings();
	return;
}

void DialogPulseGenerator::on_pushButtonDefault_pressed()
{
	ui->comboBoxTrigSource->setCurrentIndex(settingTriggerSourceDefault);
	emit defaults_loaded();
	return;
}

void DialogPulseGenerator::on_pushButtonTriggerManually_pressed()
{
	mainWindow->lsc.triggerStateControlManually(ui->spinBoxBoard->value());
	return;
}

