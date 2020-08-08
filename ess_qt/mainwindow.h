#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDateTime>
#include <QValidator>

#include "RevenueCalculator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

static const QStringList HOUR_COLUMN_HEADERS = {"Hour 1","Hour 2","Hour 3A","Hour 4","Hour 5","Hour 6","Hour 7",
                                                     "Hour 8","Hour 9","Hour 10","Hour 11","Hour 12","Hour 13","Hour 14",
                                                     "Hour 15","Hour 16","Hour 17","Hour 18","Hour 19","Hour 20","Hour 21","Hour 22","Hour 23","Hour 24"};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    RevenueCalculator* rCal;

private slots:

    void on_rMaxButton_clicked();

    void on_saveResultAsButton_clicked();

    void on_openFile_triggered();

    void on_pushButton_clicked();

    void on_actionRun_debug_content_triggered();

private:
    void logText(const QString& t);
    bool validateUserParams();

    // price data
    QStringList *priceDataHeader;
    vector<int> *hourColumnIndices;
    QStringList *priceDataDates;
    vector<double> *priceData;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
