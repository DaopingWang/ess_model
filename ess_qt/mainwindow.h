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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    RevenueCalculator rCal;

private slots:

    void on_rMaxButton_clicked();

    void on_saveResultAsButton_clicked();

    void on_openFile_triggered();

private:
    void logText(const QString& t);
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
