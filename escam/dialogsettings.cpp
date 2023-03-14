#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QMessageBox>
#include <QtGlobal>
#include "lsc-gui.h"

DialogSettings::DialogSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);
	// setup cameraSettingsWidgets
	ui->cameraSettingsWidgetBoard0->drvno = 0;
	ui->cameraSettingsWidgetBoard1->drvno = 1;
	ui->cameraSettingsWidgetBoard2->drvno = 2;
	ui->cameraSettingsWidgetBoard3->drvno = 3;
	ui->cameraSettingsWidgetBoard4->drvno = 4;
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->cameraSettingsWidgetBoard0, &CameraSettingsWidget::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->cameraSettingsWidgetBoard1, &CameraSettingsWidget::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->cameraSettingsWidgetBoard2, &CameraSettingsWidget::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->cameraSettingsWidgetBoard3, &CameraSettingsWidget::on_accepted);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, ui->cameraSettingsWidgetBoard4, &CameraSettingsWidget::on_accepted);
	connect(ui->comboBoxSettingsLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->cameraSettingsWidgetBoard0, &CameraSettingsWidget::changeSettingsLevel);
	connect(ui->comboBoxSettingsLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->cameraSettingsWidgetBoard1, &CameraSettingsWidget::changeSettingsLevel);
	connect(ui->comboBoxSettingsLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->cameraSettingsWidgetBoard2, &CameraSettingsWidget::changeSettingsLevel);
	connect(ui->comboBoxSettingsLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->cameraSettingsWidgetBoard3, &CameraSettingsWidget::changeSettingsLevel);
	connect(ui->comboBoxSettingsLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->cameraSettingsWidgetBoard4, &CameraSettingsWidget::changeSettingsLevel);
	connect(this, &DialogSettings::defaults_loaded, ui->cameraSettingsWidgetBoard0, &CameraSettingsWidget::loadDefaults);
	connect(this, &DialogSettings::defaults_loaded, ui->cameraSettingsWidgetBoard1, &CameraSettingsWidget::loadDefaults);
	connect(this, &DialogSettings::defaults_loaded, ui->cameraSettingsWidgetBoard2, &CameraSettingsWidget::loadDefaults);
	connect(this, &DialogSettings::defaults_loaded, ui->cameraSettingsWidgetBoard3, &CameraSettingsWidget::loadDefaults);
	connect(this, &DialogSettings::defaults_loaded, ui->cameraSettingsWidgetBoard4, &CameraSettingsWidget::loadDefaults);
	connect(this, &DialogSettings::initializingDone, ui->cameraSettingsWidgetBoard0, &CameraSettingsWidget::initializeWidget);
	connect(this, &DialogSettings::initializingDone, ui->cameraSettingsWidgetBoard1, &CameraSettingsWidget::initializeWidget);
	connect(this, &DialogSettings::initializingDone, ui->cameraSettingsWidgetBoard2, &CameraSettingsWidget::initializeWidget);
	connect(this, &DialogSettings::initializingDone, ui->cameraSettingsWidgetBoard3, &CameraSettingsWidget::initializeWidget);
	connect(this, &DialogSettings::initializingDone, ui->cameraSettingsWidgetBoard4, &CameraSettingsWidget::initializeWidget);

	// load settings and apply them to UI
	ui->doubleSpinBoxNos->setValue(settings.value(settingNosPath, settingNosDefault).toDouble());
	ui->doubleSpinBoxNob->setValue(settings.value(settingNobPath, settingNobDefault).toDouble());
	ui->doubleSpinBoxContiniousPause_in_ms->setValue(settings.value(settingContinuousPauseInMicrosecondsPath, settingContinuousPausInMicrosecondsDefault).toDouble() / 1000);
	ui->comboBoxTheme->setCurrentIndex(settings.value(settingThemePath, settingThemeDefault).toInt());
	ui->comboBoxSettingsLevel->setCurrentIndex(settings.value(settingSettingsLevelPath, settingSettingsLevelDefault).toInt());
	ui->comboBoxSettingsLevel->currentIndexChanged(ui->comboBoxSettingsLevel->currentIndex());

	// hide all board select elements
	ui->labelBoardSel->setVisible(false);
	ui->checkBoxBoard0->setVisible(false);
	ui->checkBoxBoard1->setVisible(false);
	ui->checkBoxBoard2->setVisible(false);
	ui->checkBoxBoard3->setVisible(false);
	ui->checkBoxBoard4->setVisible(false);
	ui->checkBoxBoard0->setChecked(false);
	ui->checkBoxBoard1->setChecked(false);
	ui->checkBoxBoard2->setChecked(false);
	ui->checkBoxBoard3->setChecked(false);
	ui->checkBoxBoard4->setChecked(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
	ui->cameraSettingsTabs->setTabVisible(0, false);
	ui->cameraSettingsTabs->setTabVisible(1, false);
	ui->cameraSettingsTabs->setTabVisible(2, false);
	ui->cameraSettingsTabs->setTabVisible(3, false);
	ui->cameraSettingsTabs->setTabVisible(4, false);
#endif
	// show board select elements depending on number_of_boards with intended fall through
	switch (number_of_boards)
	{
	case 5:
		ui->checkBoxBoard4->setVisible(true);
		ui->checkBoxBoard4->setChecked(settings.value(settingBoard4Path, settingBoard4Default).toBool());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->cameraSettingsTabs->setTabVisible(4, true);
#endif
		on_checkBoxBoard4_stateChanged(ui->checkBoxBoard4->isChecked());
	case 4:
		ui->checkBoxBoard3->setVisible(true);
		ui->checkBoxBoard3->setChecked(settings.value(settingBoard3Path, settingBoard3Default).toBool());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->cameraSettingsTabs->setTabVisible(3, true);
#endif
		on_checkBoxBoard3_stateChanged(ui->checkBoxBoard3->isChecked());
	case 3:
		ui->checkBoxBoard2->setVisible(true);
		ui->checkBoxBoard2->setChecked(settings.value(settingBoard2Path, settingBoard2Default).toBool());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->cameraSettingsTabs->setTabVisible(2, true);
#endif
		on_checkBoxBoard2_stateChanged(ui->checkBoxBoard2->isChecked());
	case 2:
		ui->checkBoxBoard1->setVisible(true);
		ui->labelBoardSel->setVisible(true);
		ui->checkBoxBoard1->setChecked(settings.value(settingBoard1Path, settingBoard1Default).toBool());
		ui->checkBoxBoard0->setVisible(true);
		ui->checkBoxBoard0->setChecked(settings.value(settingBoard0Path, settingBoard0Default).toBool());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->cameraSettingsTabs->setTabVisible(1, true);
		ui->cameraSettingsTabs->setTabVisible(0, true);
#endif
		on_checkBoxBoard1_stateChanged(ui->checkBoxBoard1->isChecked());
		on_checkBoxBoard0_stateChanged(ui->checkBoxBoard0->isChecked());
		break;
	default:
	case 1:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
		ui->cameraSettingsTabs->setTabVisible(0, true);
#endif
		on_checkBoxBoard0_stateChanged(ui->checkBoxBoard0->isChecked());
		ui->checkBoxBoard0->setChecked(true);
	}
	setWindowModality(Qt::ApplicationModal);
	emit initializingDone();
}

DialogSettings::~DialogSettings()
{
	delete ui;
}

void DialogSettings::on_accepted()
{
	//Here the settings on the UI are saved to the system
	//Measurement
	settings.setValue(settingContinuousPauseInMicrosecondsPath, ui->doubleSpinBoxContiniousPause_in_ms->value() * 1000);
	//Camera setup
	settings.setValue(settingBoard0Path, ui->checkBoxBoard0->isChecked());
	settings.setValue(settingBoard1Path, ui->checkBoxBoard1->isChecked());
	settings.setValue(settingBoard2Path, ui->checkBoxBoard2->isChecked());
	settings.setValue(settingBoard3Path, ui->checkBoxBoard3->isChecked());
	settings.setValue(settingBoard4Path, ui->checkBoxBoard4->isChecked());
	uint8_t board_sel = 0;
	board_sel |= ui->checkBoxBoard4->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard3->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard2->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard1->isChecked();
	board_sel <<= 1;
	board_sel |= ui->checkBoxBoard0->isChecked();
	settings.setValue(settingBoardSelPath, board_sel);
	//Appearance
	settings.setValue(settingThemePath, ui->comboBoxTheme->currentIndex());
	settings.setValue(settingSettingsLevelPath, ui->comboBoxSettingsLevel->currentIndex());
	settings.setValue(settingNosPath, ui->doubleSpinBoxNos->value());
	settings.setValue(settingNobPath, ui->doubleSpinBoxNob->value());
	emit settings_saved();
	return;
}

void DialogSettings::on_pushButtonDefault_clicked()
{
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning", "All settings are going to be replaced by its default values. Are you sure?", QMessageBox::Yes|QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		loadDefaults();
		emit defaults_loaded();
	}
	return;
}

void DialogSettings::on_checkBoxBoard0_stateChanged(int state)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
	{
		//basic
	case 0:
		enabled = state;
		visible = state;
		break;
		//advanced
	case 1:
		enabled = state;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->cameraSettingsTabs->setTabEnabled(0, enabled);
	return;
}

void DialogSettings::on_checkBoxBoard1_stateChanged(int state)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
	{
		//basic
	case 0:
		enabled = state;
		visible = state;
		break;
		//advanced
	case 1:
		enabled = state;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->cameraSettingsTabs->setTabEnabled(1, enabled);
	return;
}

void DialogSettings::on_checkBoxBoard2_stateChanged(int state)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
	{
		//basic
	case 0:
		enabled = state;
		visible = state;
		break;
		//advanced
	case 1:
		enabled = state;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->cameraSettingsTabs->setTabEnabled(2, enabled);
	return;
}

void DialogSettings::on_checkBoxBoard3_stateChanged(int state)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
	{
		//basic
	case 0:
		enabled = state;
		visible = state;
		break;
		//advanced
	case 1:
		enabled = state;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->cameraSettingsTabs->setTabEnabled(3, enabled);
	return;
}

void DialogSettings::on_checkBoxBoard4_stateChanged(int state)
{
	bool enabled = true,
		visible = true;
	switch (ui->comboBoxSettingsLevel->currentIndex())
	{
		//basic
	case 0:
		enabled = state;
		visible = state;
		break;
		//advanced
	case 1:
		enabled = state;
		visible = true;
		break;
		//expert
	case 2:
		enabled = true;
		visible = true;
		break;
	}
	ui->cameraSettingsTabs->setTabEnabled(4, enabled);
	return;
}

void DialogSettings::loadDefaults()
{
	ui->doubleSpinBoxContiniousPause_in_ms->setValue(settingContinuousPausInMicrosecondsDefault / 1000);
	ui->doubleSpinBoxNos->setValue(settingNosDefault);
	ui->doubleSpinBoxNob->setValue(settingNobDefault);
	//camera setup
	ui->checkBoxBoard0->setChecked(settingBoard0Default);
	ui->checkBoxBoard1->setChecked(settingBoard1Default);
	ui->checkBoxBoard2->setChecked(settingBoard2Default);
	ui->checkBoxBoard3->setChecked(settingBoard3Default);
	ui->checkBoxBoard4->setChecked(settingBoard4Default);
	//appearance
	ui->comboBoxTheme->setCurrentIndex(settingThemeDefault);
	ui->comboBoxSettingsLevel->setCurrentIndex(settingSettingsLevelDefault);
	return;
}

void DialogSettings::on_comboBoxSettingsLevel_currentIndexChanged(int index)
{
	on_checkBoxBoard0_stateChanged(ui->checkBoxBoard0->isChecked());
	on_checkBoxBoard1_stateChanged(ui->checkBoxBoard1->isChecked());
	on_checkBoxBoard2_stateChanged(ui->checkBoxBoard2->isChecked());
	on_checkBoxBoard3_stateChanged(ui->checkBoxBoard3->isChecked());
	on_checkBoxBoard4_stateChanged(ui->checkBoxBoard4->isChecked());
	return;
}