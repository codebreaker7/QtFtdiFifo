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
}

void MainWindow::searchConnectedDevice() {
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE * info;
    DWORD numdevs;
    status = FT_CreateDeviceInfoList(&numdevs);
    if (status == FT_OK && numdevs > 0) {
        // some devices have been found
    } else {
        // no connected devices
        return;
    }
    if (numdevs > 0) {
        info = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * numdevs);
        status = FT_GetDeviceInfoList(info, &numdevs);
        if (status == FT_OK) {
            // try to open first device in the list
            //status = FT_OpenEx((*info).SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle); -- no description available
            status = FT_Open(0, &ftHandle);
            if (status != FT_OK) {
                // cannot open specified device
                QMessageBox::information(this, "Cannot open device", "Cannot open device", QMessageBox::Ok);
                return;
            }
        }
    }
}

void MainWindow::setupFifoMode() {
    FT_STATUS status;
    UCHAR mask = 0xff;
    UCHAR mode;

    mode = 0x00;
    status = FT_SetBitMode(ftHandle, mask, mode);
    QThread::msleep(10);
    mode = 0x40;
    status |= FT_SetBitMode(ftHandle, mask, mode);
    if (status != FT_OK) {
        // cannot set bit mode
    }

    status = FT_SetLatencyTimer(ftHandle, 2);
    status |= FT_SetUSBParameters(ftHandle, 0x10000, 0x10000);
    status |= FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0x0, 0x0);
    status |= FT_Purge(ftHandle, FT_PURGE_RX);
    status |= FT_Purge(ftHandle, FT_PURGE_TX);

    if (status != FT_OK) {
        // cannot set parameters
    }

}

void MainWindow::searchAndOpenDevice() {
    searchConnectedDevice();
    setupFifoMode();
}

void MainWindow::sendDataFromFile() {

}

void MainWindow::sendLoopback() {
    FT_STATUS status;
    DWORD rxBytes, txBytes, events, written, read;
    char txBuffer[FT_BUFFER_SIZE];
    char rxBuffer[FT_BUFFER_SIZE];
    // init array with test data
    for (int i = 0; i < FT_BUFFER_SIZE; ++i) {
        txBuffer[i] = i % 256;
    }
    // first try to send
    status = FT_GetStatus(ftHandle, &rxBytes, &txBytes, &events);
    if (status == FT_OK && txBytes == 0) {
        status = FT_Write(ftHandle, txBuffer, FT_BUFFER_SIZE, &written);
        if (status == FT_OK && written == FT_BUFFER_SIZE) {

        } else {
            return;
        }
    }
    // read data back
    status = FT_GetStatus(ftHandle, &rxBytes, &txBytes, &events);
    if (status == FT_OK && rxBytes > 0) {
        status = FT_Read(ftHandle, rxBuffer, rxBytes, &read);
        if (status == FT_OK && read == rxBytes) {

        }
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

MainWindow::~MainWindow()
{
    delete ui;
}
