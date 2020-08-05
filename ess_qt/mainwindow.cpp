#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    rCal = new RevenueCalculator();
    priceData = nullptr;
    priceDataHeader = nullptr;
    priceDataDates = nullptr;
    hourColumnIndices = nullptr;

    // lineedit validation
    ui->nCycleMaxEdit->setValidator(new QIntValidator(1, INT32_MAX, this));         // still accepting 0??
    ui->deltaPMinEdit->setValidator(new QDoubleValidator(0, INT32_MAX, 2, this));
    ui->tChargeEdit->setValidator(new QIntValidator(1, INT32_MAX, this));
    ui->tDischargeEdit->setValidator(new QIntValidator(1,INT32_MAX, this));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete rCal;
    delete priceData;
    delete priceDataHeader;
    delete priceDataDates;
    delete hourColumnIndices;
}


void MainWindow::on_rMaxButton_clicked()
{
    // validation
    if (!validateUserParams()) return;

    double minPriceDiff = ui->deltaPMinEdit->text().isEmpty() ? 0.0 : ui->deltaPMinEdit->text().toDouble();
    int nCycleMax = ui->nCycleMaxEdit->text().isEmpty() ? INT32_MAX : ui->nCycleMaxEdit->text().toInt();

    // TODO set uParams etc
    rCal->setUserParams(priceData, ui->tChargeEdit->text().toInt(), ui->tDischargeEdit->text().toInt(), minPriceDiff, nCycleMax);
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: User params loaded");

    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Calculating rMax...");
    rCal->calculateRInfo();
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: rMax calculated");

    int rMax = rCal->getRevenueInfo().totalRevenue;
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: rMax = " + QString::number(rMax));

    // debug
    rCal->calculateRevenueInfo();
    rMax = rCal->getRevenueInfo().totalRevenue;
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Automata rMax = " + QString::number(rMax));

}

void MainWindow::on_saveResultAsButton_clicked()
{
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Result saved");
}

void MainWindow::on_openFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open the csv file");
    if (!fileName.endsWith(".csv")) {
        QMessageBox::warning(this, "Warning", "Cannot open file: Only CSV files are supported!");
        return;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        return;
    }

    // TODO csv file contains unused columns, remove them based on headers

    // reload metadata
    if (priceData != nullptr) delete priceData;
    if (priceDataHeader != nullptr) delete priceDataHeader;
    if (priceDataDates != nullptr) delete priceDataDates;
    if (hourColumnIndices != nullptr) delete hourColumnIndices;

    priceData = new vector<double>();
    priceDataHeader = new QStringList();
    priceDataDates = new QStringList();
    hourColumnIndices = new vector<int>();

    int lineInd = 0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (line.at(0) == '#') continue;
        QList<QByteArray> splittedLine = line.split(',');

        // first line of csv contains metadata
        if (priceDataHeader->isEmpty()) {
            int i = 0;
            for (QByteArray w : splittedLine) {
                priceDataHeader->append(w);

                // find relevant column indices
                if (HOUR_COLUMN_HEADERS.contains(w)) hourColumnIndices->push_back(i);
                i++;
            }
            lineInd++;
            continue;
        }

        // first element of each line is the price timestamp
        priceDataDates->append(splittedLine.at(0));
        for (int i = 1; i < splittedLine.size(); i++) {

            // push price values into vector
            if (std::find(hourColumnIndices->begin(), hourColumnIndices->end(), i) != hourColumnIndices->end()) {
                priceData->push_back(splittedLine.at(i).toDouble());
            }
        }
        lineInd++;
    }
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Price data csv successfully loaded!");
}

void MainWindow::logText(const QString &t)
{
    ui->logBrowser->append(t);
}

bool MainWindow::validateUserParams()
{
    if (ui->tChargeEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "tCharge must not be empty!");
        return false;
    }
    if (ui->tDischargeEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "tDischarge must not be empty!");
        return false;
    }
    if (priceData == nullptr || priceData->size() == 0) {
        QMessageBox::warning(this, "Warning", "Price data not yet initialized!");
        return false;
    }
    return true;
}
