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
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("Text files (*.txt)"));
    QStringList fileNames;
    if (fileDialog.exec()) {
        fileNames = fileDialog.selectedFiles();
        QString selectedFileName = fileNames.first();
        ui->sendFileNameEdit->setText(selectedFileName);
    }
}

void MainWindow::selectFileToReceive() {
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setNameFilter(tr("Text files (*.txt)"));
    if (fileDialog.exec()) {
        QStringList fileNames = fileDialog.selectedFiles();
        ui->receiveFileNameEdit->setText(fileNames.first());
    }
}

void MainWindow::sendDataFromFile() {
    if (control->isOpened() && ui->sendFileButton->text().length() != 0) {
        control->sendDataFromFile(ui->sendFileNameEdit->text());
    }
}

void MainWindow::receiveInFile() {
    if (control->isOpened()) {
        if (ui->receiveFileNameEdit->text().isEmpty()) {
            QMessageBox::information(this, "No file selected", "Select file to receive first!", QMessageBox::Ok);
            return;
        }
        if (ui->useDefSizeCheckBox->isChecked()) {
            control->setRecPackSize(FtdiControl::DEFAULT_REC_PACKAGE_SIZE);
        } else {
            uint newSize = ui->receiveDataSizeEdit->text().toUInt();
            if (newSize < 0) {
                QMessageBox::information(this, "Size error", "Cannot be negative", QMessageBox::Ok);
                return;
            }
            control->setRecPackSize(newSize);
        }
        control->receiveDataInFile(ui->receiveFileNameEdit->text());
    }
}

void MainWindow::processSizeCheckBox(int val) {
    if (val == Qt::Unchecked) {
        ui->receiveDataSizeEdit->setEnabled(true);
    } else if (val == Qt::Checked) {
        ui->receiveDataSizeEdit->setEnabled(false);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
