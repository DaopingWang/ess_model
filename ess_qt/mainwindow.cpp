#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // lineedit validation
    ui->nCycleMaxEdit->setValidator(new QIntValidator(1, INT32_MAX, this));         // still accepting 0??
    ui->deltaPMinEdit->setValidator(new QDoubleValidator(0, INT32_MAX, 2, this));
    ui->tChargeEdit->setValidator(new QIntValidator(1, INT32_MAX, this));
    ui->tDischargeEdit->setValidator(new QIntValidator(1,INT32_MAX, this));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_rMaxButton_clicked()
{
    // TODO set uParams etc
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Calculating rMax...");
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

    // TODO load csv into vector, while also keeping track of dates and times
    QStringList wordList;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        for (QByteArray w : line.split(',')) wordList.append(w);
    }
    logText("[" + QString::number(QDateTime::currentSecsSinceEpoch()) + "]: Price data csv successfully loaded!");
}

void MainWindow::logText(const QString &t)
{
    ui->logBrowser->append(t);
}
