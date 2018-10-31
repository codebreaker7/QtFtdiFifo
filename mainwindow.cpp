#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    control = new FtdiControl(this);
    connect(control, &FtdiControl::errorHappened, this, &MainWindow::reportError);
    connect(control, &FtdiControl::loopbackSuccessful, this, &MainWindow::reportSuccess);
}

void MainWindow::searchAndOpenDevice() {
      if (control->openDevice() != 0) {
          QMessageBox::information(this, "FTDI", "Cannot open device", QMessageBox::Ok);
          return;
      }
      if (control->goToFifoMode() != 0) {
          QMessageBox::information(this, "FTDI", "Cannot set FIFO mode", QMessageBox::Ok);
      }
}

void MainWindow::reportError() {
    QMessageBox::information(this, "Error from FTDI", "Error happened", QMessageBox::Ok);
}

void MainWindow::reportSuccess() {
    QMessageBox::information(this, "Loopback", "Loopback transmission is successful", QMessageBox::Ok);
}

void MainWindow::sendLoopback() {
    if (control->isOpened()) {
        control->sendDataLoopback();
    }
}

void MainWindow::selectFileToSend() {
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setNameFilter(tr("Text files (*.txt)"));
    QStringList fileNames;
    if (fileDialog.exec()) {
        fileNames = fileDialog.selectedFiles();
        QString selectedFileName = fileNames.first();
        ui->sendFileNameEdit->setText(selectedFileName);
    }
}

void MainWindow::sendDataFromFile() {
    if (control->isOpened() && ui->sendFileButton->text().length() != 0) {
        control->sendDataFromFile(ui->sendFileNameEdit->text());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
