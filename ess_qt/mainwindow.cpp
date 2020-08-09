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

    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: ------------------");
    // TODO set uParams etc
    rCal->setUserParams(priceData, ui->tChargeEdit->text().toInt(), ui->tDischargeEdit->text().toInt(), minPriceDiff, nCycleMax);
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: User params loaded");

    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Calculating rMax...");
    rCal->calculateRInfo();
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: rMax calculated");

    double rMax = rCal->getRevenueInfo().totalRevenue;
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: rMax = " + QString::number(rMax, 'f', 2));

    // debug
   /* rCal->automataReference();
    rMax = rCal->getRevenueInfo().totalRevenue;
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Automata rMax = " + QString::number(rMax, 'f', 2));*/

    rMax = rCal->rMaxReference(ui->nCycleMaxEdit->text().toInt());
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: K Reference rMax = " + QString::number(rMax, 'f', 2));
    if (rCal->cycleVerification()) logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Verification successful");
    else logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Verification failed");
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

void MainWindow::on_pushButton_clicked()
{
    on_openFile_triggered();
}

void MainWindow::on_actionRun_debug_content_triggered()
{
    RevenueCalculator* rCal = new RevenueCalculator();
    //vector<double> prices = {14, 15, 10, 11, 17, 20, 32, 11, 10, 33, 33}; // cents
    vector<double> prices = {24.14,22.12,13.01,4.97,9.81,18.37,23.50,27.17,36.56,
                             40.43,32.22,38.98,38.60,37.90,38.00,39.58,42.28,46.06,
                             47.73,46.00,42.20,39.74,38.88,37.39};

    rCal->setUserParams(&prices,
                        ui->tChargeEdit->text().toInt(),
                        ui->tDischargeEdit->text().toInt(),
                        ui->deltaPMinEdit->text().toDouble(),
                        ui->nCycleMaxEdit->text().toInt());

    double rMax = rCal->rMaxReference(ui->nCycleMaxEdit->text().toDouble());
    logText("Maximum revenue reference: " + QString::number(rMax));
    rCal->calculateRInfo();
    logText("Maximum revenue func: " + QString::number(rCal->getRevenueInfo().totalRevenue));
    if (rCal->cycleVerification()) logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Verification successful");
    else logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Verification failed");

    delete rCal;
    return;
}
