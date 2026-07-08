#include "dialogservo.h"
#include "dialogsettings.h"
#include <algorithm>
#include <bitset>

DialogServo::DialogServo(QWidget* parent)
	: QDialog(parent),
	ui(new Ui::dialogservoClass)
{
	ui->setupUi(this);

	ui->spinBoxBoard->setMaximum(MAXPCIECARDS);
	if (mainWindow->lsc.numberOfBoards > 1)
	{
		ui->labelBoard->setVisible(true);
		ui->spinBoxBoard->setVisible(true);
	}
	else
	{
		ui->labelBoard->setVisible(false);
		ui->spinBoxBoard->setVisible(false);
	}

	QRegularExpression decRegex("^[0-9]*$");
	QValidator* decValidator = new QRegularExpressionValidator(decRegex, this);
	ui->lineEditDec->setValidator(decValidator);

	QRegularExpression hexRegex("^[0-9A-Fa-f]*$");
	QValidator* hexValidator = new QRegularExpressionValidator(hexRegex, this);
	ui->lineEditHex->setValidator(hexValidator);

	QRegularExpression binRegex("^[01]*$");
	QValidator* binValidator = new QRegularExpressionValidator(binRegex, this);
	ui->lineEditBin->setValidator(binValidator);

	loadSettings();
}

DialogServo::~DialogServo()
{}

void DialogServo::on_spinBoxBoard_valueChanged()
{
	loadSettings();
	return;
}

void DialogServo::loadSettings()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	ui->spinBoxSeqLength->setValue(settings.value(settingServoSequenceLengthPath, settingServoSequenceLengthDefault).toInt());
	ui->lineEditBin->setText(settings.value(settingServoBinPath, settingServoBinDefault).toString());
	ui->lineEditDec->setText(settings.value(settingServoDecPath, settingServoDecDefault).toString());
	ui->lineEditHex->setText(settings.value(settingServoHexPath, settingServoHexDefault).toString());
	ui->comboBoxTriggerSource->setCurrentIndex(settings.value(settingServoTriggerSourcePath, settingServoTriggerSourceDefault).toInt());
	ui->spinBoxPos1->setValue(settings.value(settingServoPos1Path, settingServoPos1Default).toInt());
	ui->spinBoxPos2->setValue(settings.value(settingServoPos2Path, settingServoPos2Default).toInt());
	ui->spinBoxStepPeriod->setValue(settings.value(settingServoStpPeriodPath, settingServoStpPeriodDefault).toInt());
	settings.endGroup();
	return;
}

void DialogServo::on_pushButtonDefault_clicked()
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	ui->spinBoxSeqLength->setValue(settingServoSequenceLengthDefault);
	ui->lineEditBin->setText(settingServoBinDefault);
	ui->lineEditDec->setText(settingServoDecDefault);
	ui->lineEditHex->setText(settingServoHexDefault);
	ui->comboBoxTriggerSource->setCurrentIndex(settingServoTriggerSourceDefault);
	ui->spinBoxPos1->setValue(settingServoPos1Default);
	ui->spinBoxPos2->setValue(settingServoPos2Default);
	settings.endGroup();
	return;
}

void DialogServo::on_spinBoxPos1_valueChanged(int value)
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoPos1Path, value);
	settings.endGroup();
	mainWindow->lsc.camSetGalvoRestState1(ui->spinBoxBoard->value(), 0, value);
	return;
}

void DialogServo::on_spinBoxPos2_valueChanged(int value)
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoPos2Path, value);
	settings.endGroup();
	mainWindow->lsc.camSetGalvoRestState2(ui->spinBoxBoard->value(), 0, value);
	return;
}

void DialogServo::on_spinBoxSeqLength_valueChanged(int val)
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoSequenceLengthPath, val);
	settings.endGroup();
	if (ui->lineEditBin->text().length() > val)
	{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
		ui->lineEditBin->setText(ui->lineEditBin->text().right(val));
#else
		ui->lineEditBin->setText(ui->lineEditBin->text().last(val));
#endif
	}
	mainWindow->lsc.camSetGalvoBinSeqLen(ui->spinBoxBoard->value(), 0, val);
	return;
}

void DialogServo::on_lineEditDec_textChanged()
{
	QString dec = ui->lineEditDec->text();
	if (dec.isEmpty()) {
		return;
	}
	QString maxDec = convertBinaryToDecimal(QString("1").repeated(ui->spinBoxSeqLength->value()));
	if (dec.length() > maxDec.length()) {
		ui->lineEditDec->setText(maxDec);
		dec = maxDec;
	}
	if (dec.length() == maxDec.length()) {

		for (int i = 0; i < maxDec.length(); ++i) {
			if (maxDec[i].digitValue() < dec[i].digitValue()) {
				ui->lineEditDec->setText(maxDec);
				dec = maxDec;
				break;
			}
			else if (maxDec[i].digitValue() > dec[i].digitValue()) {
				break;
			}
		}
	}
	QString bin = convertDecimalToBinary(dec);
	QString hex = convertBinaryToHex(bin);

	ui->lineEditHex->blockSignals(true);
	ui->lineEditBin->blockSignals(true);

	ui->lineEditHex->setText(hex);
	ui->lineEditBin->setText(addLeadingZerosToBin(bin));
	mainWindow->lsc.camSetGalvoBinSeq(ui->spinBoxBoard->value(), 0, ui->lineEditBin->text());

	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoBinPath, bin);
	settings.setValue(settingServoDecPath, dec);
	settings.setValue(settingServoHexPath, hex);
	settings.endGroup();

	ui->lineEditHex->blockSignals(false);
	ui->lineEditBin->blockSignals(false);
	return;
}

void DialogServo::on_lineEditHex_textChanged()
{
	QString hex = ui->lineEditHex->text();
	QString maxHex = convertBinaryToHex(QString("1").repeated(ui->spinBoxSeqLength->value()));
	if (hex.isEmpty()) {
		return;
	}
	if (hex.length() > maxHex.length()) {
		ui->lineEditHex->setText(maxHex);
		hex = maxHex;
	}
	else if (hex.length() == maxHex.length()) {
		if (QString(maxHex[0]).toInt(nullptr, 16) < QString(hex[0]).toInt(nullptr, 16)) {
			ui->lineEditHex->setText(maxHex);
			hex = maxHex;
		}
	}

	QString bin = convertHexToBinary(hex);
	QString dec = convertBinaryToDecimal(bin);

	ui->lineEditDec->blockSignals(true);
	ui->lineEditBin->blockSignals(true);

	ui->lineEditDec->setText(dec);
	ui->lineEditBin->setText(addLeadingZerosToBin(bin));
	mainWindow->lsc.camSetGalvoBinSeq(ui->spinBoxBoard->value(), 0, ui->lineEditBin->text());

	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoBinPath, bin);
	settings.setValue(settingServoDecPath, dec);
	settings.setValue(settingServoHexPath, hex);
	settings.endGroup();

	ui->lineEditDec->blockSignals(false);
	ui->lineEditBin->blockSignals(false);
	return;
}

void DialogServo::on_lineEditBin_textChanged()
{

	if (ui->lineEditBin->text().isEmpty()) {
		return;
	}
	int cursorPosition = ui->lineEditBin->cursorPosition();
	if (ui->lineEditBin->text().length() >= ui->spinBoxSeqLength->value()) {
		ui->lineEditBin->setText(ui->lineEditBin->text().right(ui->spinBoxSeqLength->value()));
	}
	// Saves the cursor position before changing the text to prevent it from jumping to the end. - 1 because it is 0-based index
	ui->lineEditBin->setCursorPosition(cursorPosition - 1);
	QString bin = ui->lineEditBin->text();
	QString dec = convertBinaryToDecimal(bin);
	QString hex = convertBinaryToHex(bin);

	ui->lineEditDec->blockSignals(true);
	ui->lineEditHex->blockSignals(true);

	ui->lineEditDec->setText(dec);
	ui->lineEditHex->setText(hex);

	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoBinPath, bin);
	settings.setValue(settingServoDecPath, dec);
	settings.setValue(settingServoHexPath, hex);
	settings.endGroup();

	ui->lineEditDec->blockSignals(false);
	ui->lineEditHex->blockSignals(false);
	return;
}

void DialogServo::on_lineEditBin_editingFinished() {
	ui->lineEditBin->blockSignals(true);
	QString bin = ui->lineEditBin->text();
	ui->lineEditBin->setText(addLeadingZerosToBin(bin));
	ui->lineEditBin->blockSignals(false);
	mainWindow->lsc.camSetGalvoBinSeq(ui->spinBoxBoard->value(), 0, ui->lineEditBin->text());
}

QString DialogServo::convertDecimalToBinary(QString decimal)
{
	std::string decimalAsStdString = decimal.toStdString();

	if (decimalAsStdString.empty() || decimalAsStdString == "0") {
		return QString("0");
	}

	std::string binary = "";
	std::string temp = decimalAsStdString;

	while (temp != "0") {
		int remainder = 0;
		std::string dividedNumberAsString = "";
		for (const char digit : temp) {
			int current = remainder * 10 + (digit - '0');

			remainder = current % 2;
			dividedNumberAsString += (current / 2) + '0';
		}

		binary += (remainder + '0');
		size_t firstNonZero = dividedNumberAsString.find_first_not_of('0');
		if (firstNonZero != std::string::npos) {
			temp = dividedNumberAsString.substr(firstNonZero);
		}
		else {
			temp = "0";
		}
	}
	std::reverse(binary.begin(), binary.end());
	while (binary.length() > 1 && binary[0] == '0') {
		binary.erase(0, 1);
	}

	return QString::fromStdString(binary);
}

QString DialogServo::convertHexToBinary(QString hex)
{
	std::string hexAsStdString = hex.toUpper().toStdString();
	std::string result;

	if (hexAsStdString.empty()) {
		return QString("0");
	}

	std::string binaryAsStdString = "";
	for (const char hexChar : hexAsStdString) {
		int decimalValue = 0;
		if (hexChar >= '0' && hexChar <= '9') {
			decimalValue = hexChar - '0';
		}
		else if (hexChar >= 'A' && hexChar <= 'F') {
			decimalValue = hexChar - 'A' + 10;
		}

		std::bitset<4> binarySet(decimalValue);
		binaryAsStdString += binarySet.to_string();
	}

	while (binaryAsStdString.length() > 1 && binaryAsStdString[0] == '0') {
		binaryAsStdString.erase(0, 1);
	}

	return QString::fromStdString(binaryAsStdString);
}

QString DialogServo::convertBinaryToDecimal(QString binary)
{
	std::string binaryAsStdString = binary.toStdString();
	constexpr unsigned int numberBase{ 10 };
	std::string result;
	do {
		unsigned int remainder = 0;
		std::string dividedNumberAsString = "";
		for (const char bit : binaryAsStdString) {
			remainder = remainder * 2 + (bit - '0');

			if (remainder >= numberBase) {
				remainder -= numberBase;
				dividedNumberAsString += '1';
			}
			else {
				dividedNumberAsString += '0';
			}
		}
		binaryAsStdString = dividedNumberAsString;
		result.insert(0, 1, '0' + remainder);
	} while (std::count(binaryAsStdString.begin(), binaryAsStdString.end(), '1'));

	return QString::fromStdString(result);
}

QString DialogServo::convertBinaryToHex(QString binary)
{
	if (binary.isEmpty()) {
		return QString("0");
	}

	int remainder = binary.length() % 4;
	if (remainder != 0) {
		QString padding = QString(4 - remainder, '0');
		binary.prepend(padding);
	}

	QString result = "";
	for (int i = 0; i < binary.length(); i += 4) {
		QString chunk = binary.mid(i, 4);
		bool ok;
		int decimalValue = chunk.toInt(&ok, 2);

		if (decimalValue < 10) {
			result.append(QString::number(decimalValue));
		}
		else {
			result.append(QChar('A' + decimalValue - 10));
		}
	}
	return result;
}

QString DialogServo::addLeadingZerosToBin(QString bin)
{
	int length = ui->spinBoxSeqLength->value();
	if (bin.length() < length) {
		bin.prepend(QString(length - bin.length(), '0'));
	}
	return bin;
}

void DialogServo::on_comboBoxTriggerSource_currentIndexChanged()
{
	uint32_t drvno = ui->spinBoxBoard->value();
	int index = ui->comboBoxTriggerSource->currentIndex();
	mainWindow->lsc.setStateControlRegister(drvno, index);

	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoTriggerSourcePath, index);
	settings.endGroup();

	if (index == statectrl_trigger_select_manual)
	{
		ui->labelManualTrig->setVisible(true);
		ui->pushButtonManualTrig->setVisible(true);
	}
	else
	{
		ui->labelManualTrig->setVisible(false);
		ui->pushButtonManualTrig->setVisible(false);
	}

	return;
}

void DialogServo::on_pushButtonManualTrig_clicked()
{
	mainWindow->lsc.triggerStateControlManually(ui->spinBoxBoard->value());
	return;
}

void DialogServo::on_radioButtonCalibratePos1_toggled()
{
	return;
}

void DialogServo::on_radioButtonCalibratePos2_toggled()
{
	return;
}

void DialogServo::on_radioButtonTrigSeq_toggled()
{
	return;
}

void DialogServo::on_spinBoxStepPeriod_valueChanged(int value)
{
	settings.beginGroup("board" + QString::number(ui->spinBoxBoard->value()));
	settings.setValue(settingServoStpPeriodPath, value);
	settings.endGroup();
	mainWindow->lsc.camSetGalvoStpPeriod(ui->spinBoxBoard->value(), 0, value);
	return;
}

void DialogServo::on_pushButtonSendAll_clicked()
{
	on_spinBoxSeqLength_valueChanged(ui->spinBoxSeqLength->value());
	on_lineEditBin_textChanged();
	on_lineEditBin_editingFinished();
	on_spinBoxPos1_valueChanged(ui->spinBoxPos1->value());
	on_spinBoxPos2_valueChanged(ui->spinBoxPos2->value());
	on_comboBoxTriggerSource_currentIndexChanged();
	on_spinBoxStepPeriod_valueChanged(ui->spinBoxStepPeriod->value());
	return;
}
