#include "mainwindow.h"

/**
 * @brief Constructor of Class MainWindow.
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QChart *chart = ui->chartView->chart();
    chart->legend()->hide();
    chart->setTitle("Camera 1");
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    connect(ui->horizontalSliderSample, SIGNAL(valueChanged(int)), this, SLOT(sampleChanged(int)));
    connect(ui->horizontalSliderBlock, SIGNAL(valueChanged(int)), this, SLOT(blockChanged(int)));
    connect(ui->pushButtonStart, SIGNAL(pressed()), this, SLOT(startPressed()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    if(lsc.initDriver() < 1) showNoDriverFoundDialog();
    else if(lsc.initPcieBoard() < 0) showPcieBoardError();
    else
        loadSettings();
}

/**
 * @brief Destructor of class MainWindow.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Sets the data of chartView.
 * @param series Data as QLineSeries.
 */
void MainWindow::setChartData(QLineSeries* series)
{
    QChart *chart = ui->chartView->chart();
    chart->removeAllSeries();
    chart->addSeries(series);
    chart->createDefaultAxes();
    return;
}

/**
 * @brief This is an overloaded function.
 * @param data Pointer to data.
 * @param length Length of data.
 */
void MainWindow::setChartData(uint16_t* data, uint16_t length)
{
    QLineSeries* series = new QLineSeries();
    for(uint16_t i=0; i<length; i++)
    {
        series->append(i, *(data+i));
    }
    setChartData(series);
    return;
}

/**
 * @brief Slot for the signal valueChanged of horizontalSliderSample.
 * @param sample Current value of slider.
 */
void MainWindow::sampleChanged(int sample)
{
    uint16_t data[576];
    sample--;
    int block = ui->horizontalSliderBlock->value() - 1;
    lsc.returnFrame(0,sample,block,0,data,576);
    setChartData(data,576);
    return;
}


/**
 * @brief Slot for the signal valueChanged of horizontalSliderBlock.
 * @param block Current value of slider.
 */
void MainWindow::blockChanged(int block)
{
    uint16_t data[576];
    block--;
    int sample = ui->horizontalSliderSample->value() - 1 ;
    lsc.returnFrame(0,sample,block,0,data,576);
    setChartData(data,576);
    return;
}

/**
 * @brief Slot for the signal pressed of pushButtonStart.
 * @return none
 */
void MainWindow::startPressed()
{
    lsc.initMeasurement();
    lsc.startMeasurement();
    uint16_t data[576];
    int block = ui->horizontalSliderBlock->value() - 1;
    int sample = ui->horizontalSliderSample->value() - 1 ;
    lsc.returnFrame(0,sample,block,0,data,576);
    setChartData(data,576);
    return;
}

/**
 * @brief This slot opens the settings dialog.
 * @return none
 */
void MainWindow::on_actionEdit_triggered()
{
    DialogSettings* ds = new DialogSettings(&settings);
    ds->show();
    connect(ds, SIGNAL(settings_saved()), this, SLOT(loadSettings()));
    return;
}

/**
 * @brief Load settings and apply to UI.
 */
void MainWindow::loadSettings()
{
    int nos = settings.value(SETTING_NOS,NOS_DEFAULT).toInt();
    ui->horizontalSliderSample->setMaximum(nos);
    ui->spinBoxSample->setMaximum(nos);
    int nob = settings.value(SETTING_NOB,NOB_DEFAULT).toInt();
    ui->horizontalSliderBlock->setMaximum(nob);
    ui->spinBoxBlock->setMaximum(nob);
    int tor = settings.value(SETTING_TOR,TOR_DEFAULT).toInt();
    lsc.setTorOut(tor);
    int theme = settings.value(SETTING_THEME,THEME_DEFAULT).toInt();
    switch(theme)
    {
    default:
    case 0:
        qApp->setStyleSheet("");
        break;
    case 1:
        QFile f(":qdarkstyle/style.qss");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
        break;
    }
    return;
}

void MainWindow::showNoDriverFoundDialog()
{
    QMessageBox* d = new QMessageBox(this);
    d->setWindowTitle("Fatal error");
    d->setWindowModality(Qt::ApplicationModal);
    d->setText("Driver or PCIe board not found.");
    d->setIcon(QMessageBox::Critical);
    d->setDetailedText(QString::fromStdString(lsc.driverInstructions));
    d->open(this,SLOT(close()));
    return;
}

void MainWindow::showPcieBoardError()
{
    QMessageBox* d = new QMessageBox(this);
    d->setWindowTitle("Fatal error");
    d->setWindowModality(Qt::ApplicationModal);
    d->setText("Error while opening PCIe board.");
    d->setIcon(QMessageBox::Critical);
    d->open(this,SLOT(close()));
    return;
}

void MainWindow::on_actionDump_board_registers_triggered()
{
    QDialog* messageBox = new QDialog;
    QVBoxLayout* layout = new QVBoxLayout();
    messageBox->setLayout(layout);
    QTabWidget* tabWidget = new QTabWidget(messageBox);
    tabWidget->setDocumentMode(true);
    QLabel* labelS0 = new QLabel(tabWidget);
    labelS0->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelS0->setText(QString::fromStdString(lsc.dumpS0Registers()));
    labelS0->setAlignment(Qt::AlignTop);
    tabWidget->addTab(labelS0, "S0 registers");
    QLabel* labelDma = new QLabel;
    labelDma->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDma->setText(QString::fromStdString(lsc.dumpDmaRegisters()));
    labelDma->setAlignment(Qt::AlignTop);
    tabWidget->addTab(labelDma, "DMA registers");
    QLabel* labelTlp = new QLabel;
    labelTlp->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelTlp->setText(QString::fromStdString(lsc.dumpTlp()));
    labelTlp->setAlignment(Qt::AlignTop);
    tabWidget->addTab(labelTlp, "TLP size");
    layout->addWidget(tabWidget);
    QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(dialogButtonBox, SIGNAL(accepted()), messageBox, SLOT(accept()));
    layout->addWidget(dialogButtonBox);
    messageBox->setWindowTitle("Register dump");
    messageBox->show();
    return;
}
