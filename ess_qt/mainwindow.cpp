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

    fileLoaded = false;
    fileProcessed = false;

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
    delete rCal;
    rCal = new RevenueCalculator;

    // validation
    if (!validateUserParams() || !fileLoaded) return;

    fileProcessed = true;

    double minPriceDiff = ui->deltaPMinEdit->text().isEmpty() ? 0.0 : ui->deltaPMinEdit->text().toDouble();
    int nCycleMax = ui->nCycleMaxEdit->text().isEmpty() ? INT32_MAX : ui->nCycleMaxEdit->text().toInt();

    logText("[" + QDateTime::currentDateTime().toString() + "]: ==============================================");
    logText("[" + QDateTime::currentDateTime().toString() + "]: Current file: " + openedFilename);

    // set uParams etc
    rCal->setUserParams(priceData, ui->tChargeEdit->text().toInt(), ui->tDischargeEdit->text().toInt(), minPriceDiff, nCycleMax);
    //logText("[" + QDateTime::currentDateTime().toString() + "]: User params loaded");

    //logText("[" + QDateTime::currentDateTime().toString() + "]: Calculating rMax...");
    rCal->calculateRInfo();
    //logText("[" + QDateTime::currentDateTime().toString() + "]: rMax calculated");

    double rMax = rCal->getRevenueInfo().totalRevenue;
    int nCycle = rCal->getRevenueInfo().cycleTiming.size();
    bool validationResult = rCal->cycleValidation();

    if (minPriceDiff != 0) {
        double filteredRMax = rCal->getRevenueInfo().filteredSum;
        int filteredNCycle = rCal->getRevenueInfo().filteredCycleTiming.size();
        logText("[" + QDateTime::currentDateTime().toString() + "]: rMax = " + QString::number(filteredRMax, 'f', 2));
        logText("[" + QDateTime::currentDateTime().toString() + "]: nCycle = " + QString::number(filteredNCycle));
    } else {
        logText("[" + QDateTime::currentDateTime().toString() + "]: rMax = " + QString::number(rMax, 'f', 2));
        logText("[" + QDateTime::currentDateTime().toString() + "]: nCycle = " + QString::number(nCycle));
    }


    // debug
   /* rCal->automataReference();
    rMax = rCal->getRevenueInfo().totalRevenue;
    logText("[" + QDateTime::currentDateTime().toString() + "]: Automata rMax = " + QString::number(rMax, 'f', 2));*/

    if (validationResult) logText("[" + QDateTime::currentDateTime().toString() + "]: Result validation successful");
    else logText("[" + QDateTime::currentDateTime().toString() + "]: Validation failed: "  + QString::number(rCal->getRevenueInfo().validationSum));

    /*if (ui->tChargeEdit->text().toInt() == 1 && ui->tDischargeEdit->text().toInt() == 1 && minPriceDiff == 0) {
        double reference = rCal->rMaxReference(nCycleMax);
        if (abs(rMax - reference) < 1.0) logText("[" + QDateTime::currentDateTime().toString() + "]: Enhanced validation successful");
        else logText("[" + QDateTime::currentDateTime().toString() + "]: Enhanced validation failed: " + QString::number(reference, 'f', 2));
    }*/
    logText("[" + QDateTime::currentDateTime().toString() + "]: ==============================================");
}

void MainWindow::on_openFile_triggered()
{
    // flush history
    if (priceData != nullptr) delete priceData;
    if (priceDataHeader != nullptr) delete priceDataHeader;
    if (priceDataDates != nullptr) delete priceDataDates;
    if (hourColumnIndices != nullptr) delete hourColumnIndices;

    priceData = new vector<double>();
    priceDataHeader = new QStringList();
    priceDataDates = new QStringList();
    hourColumnIndices = new vector<int>();

    QString fileName = QFileDialog::getOpenFileName(this, "Open the csv file");
    openedFilename = fileName;
    if (!fileName.endsWith(".csv")) {
        QMessageBox::warning(this, "Warning", "Cannot open file: Only CSV files are supported!");
        return;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
        return;
    }

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
        vector<double> curPrice;
        for (int i = 1; i < splittedLine.size(); i++) {

            // push price values into vector
            if (std::find(hourColumnIndices->begin(), hourColumnIndices->end(), i) != hourColumnIndices->end()) {
                //priceData->push_back(splittedLine.at(i).toDouble());
                curPrice.push_back(splittedLine.at(i).toDouble());
            }
        }

        for (int i = curPrice.size()-1; i >= 0; i--) {
            priceData->insert(priceData->begin(), curPrice[i]);
        }

        lineInd++;
    }

    fileLoaded = true;
    fileProcessed = false;
    logText("[" + QDateTime::currentDateTime().toString() + "]: Price data csv successfully loaded: " + openedFilename);
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
    logText("debug content empty");
    /*RevenueCalculator* rCal = new RevenueCalculator();
    vector<double> prices = {14, 15, 10, 11, 17, 20, 32, 11, 10, 33, 33}; // cents
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
    if (rCal->cycleValidation()) logText("[" + QDateTime::currentDateTime().toString() + "]: Validation successful");
    else logText("[" + QDateTime::currentDateTime().toString() + "]: Validation failed: " + QString::number(rCal->getRevenueInfo().validationSum));

    delete rCal;*/
    return;
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->logBrowser->clear();
}

void MainWindow::on_actionSave_as_triggered()
{
    on_saveResultAsButton_clicked();
}

void MainWindow::on_saveResultAsButton_clicked()
{
    if (!fileProcessed) {
        QMessageBox::warning(this, "Warning", "Cannot write file: CSV not loaded or processed!");
        return;
    }
    QString filename = QFileDialog::getSaveFileName(this, "Save to...", openedFilename + "_result.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0);
    QFile data(filename);
    if(data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream output(&data);
        RevenueInfo rInfo = rCal->getRevenueInfo();
        UserParams uParams = rCal->getUParams();

        output << "# Filename = " + filename << endl;
        output << "# User parameters: nCycleMax = " << QString::number(uParams.maxCycles)
               << "; deltaPMin = " << QString::number(uParams.minPriceDiff)
               << "; tCharge = " << QString::number(uParams.tCharge)
               << "; tDischarge = " << QString::number(uParams.tDischarge)
               << endl;

        if (uParams.minPriceDiff != 0) {
            output << "# Maximum Revenue = " + QString::number(rInfo.filteredSum) << endl;
            output << "# nCycle = " + QString::number(rInfo.filteredCycleTiming.size()) << endl;
            output << "charge,discharge,revenue" << endl;
            for (int i = rInfo.filteredCycleTiming.size()-1; i >= 0; i--) {
                output << QString::number(rInfo.filteredCycleTiming[i].first) << ","
                       << QString::number(rInfo.filteredCycleTiming[i].second) << ","
                       << QString::number(rInfo.filteredRevenues[i]) << endl;
            }
        }
        else {
            output << "# Maximum Revenue = " + QString::number(rInfo.totalRevenue) << endl;
            output << "# nCycle = " + QString::number(rInfo.cycleTiming.size()) << endl;
            output << "charge,discharge,revenue" << endl;
            for (int i = rInfo.cycleTiming.size()-1; i >= 0; i--) {
                output << QString::number(rInfo.cycleTiming[i].first) << ","
                       << QString::number(rInfo.cycleTiming[i].second) << ","
                       << QString::number(rInfo.revenues[i]) << endl;
            }
        }

        data.close();
    } else {
        logText("[" + QDateTime::currentDateTime().toString() + "]: Result saving failed: " + data.errorString());
        return;
    }

    logText("[" + QDateTime::currentDateTime().toString() + "]: Result saved under " + filename);
}
