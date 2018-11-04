#include "ftdicontrol.h"

#include <QThread>
#include <QMessageBox>
#include <QFile>

FtdiControl::FtdiControl(QObject *parent) : QObject(parent), recPackSize(DEFAULT_REC_PACKAGE_SIZE)
{

}

int FtdiControl::openDevice() {
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE * info;
    DWORD numdevs;
    status = FT_CreateDeviceInfoList(&numdevs);
    if (status != FT_OK || numdevs == 0) {
        // no connected devices
        return status;
    }
    status = FT_SetVIDPID(0x0403, 0x6010); // this operation is not necessary with root access and can be omitted further
    if (status != FT_OK) {
        return status;
    }
    if (numdevs > 0) {
        info = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * numdevs);
        status = FT_GetDeviceInfoList(info, &numdevs);
        if (status == FT_OK) {
            // try to open first device in the list
            status = FT_Open(0, &ftHandle);
            if (status != FT_OK) {
                // cannot open specified device
                return status;
            }
        }
    }
    isOpen = true;
    return 0;
}

int FtdiControl::goToFifoMode() {
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
        return status;
    }

    status = FT_SetLatencyTimer(ftHandle, 2);
    status |= FT_SetUSBParameters(ftHandle, 0x10000, 0x10000);
    status |= FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0x0, 0x0);
    status |= FT_Purge(ftHandle, FT_PURGE_RX);
    status |= FT_Purge(ftHandle, FT_PURGE_TX);

    if (status != FT_OK) {
        // cannot set parameters
        return status;
    }
    return 0;
}

void FtdiControl::sendDataLoopback() {
    FT_STATUS status;
    DWORD rxBytes, txBytes, events, written, read;
    char txBuffer[FT_BUFFER_SIZE]; // actual size of data buffer could be changed according to the buffer from the other communication side
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
            emit errorHappened();
            return;
        }
    }
    QThread::msleep(20); // wait for data processing
    // read data back
    status = FT_GetStatus(ftHandle, &rxBytes, &txBytes, &events);
    if (status == FT_OK && rxBytes > 0) {
        status = FT_Read(ftHandle, rxBuffer, rxBytes, &read);
        if (status == FT_OK && read == rxBytes) {
            if (read != FT_BUFFER_SIZE) {
                return;
            }
            for (int i = 0; i < FT_BUFFER_SIZE; ++i) {
                if (rxBuffer[i] != char(i % 256)) {
                    emit errorHappened();
                    break;
                }
            }
        } else {

        }
    }
    emit loopbackSuccessful();
}

void FtdiControl::sendDataFromFile(const QString & filename) {
    QFile file(filename);
    qint64 read;
    DWORD written;
    FT_STATUS status;
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        char txBuffer[FT_BUFFER_SIZE];
        read = file.read(txBuffer, FT_BUFFER_SIZE);
        while (read > 0) {
            status = FT_Write(ftHandle, txBuffer, read, &written);
            if (status != FT_OK || written != read) {
                break;
            }
            read = file.read(txBuffer, FT_BUFFER_SIZE);
        }
    }
}

void FtdiControl::receiveDataInFile(const QString & filename) {
    QFile file(filename);
    char rxBuffer[FT_MAX_BUFFER_SIZE];
    DWORD read, tx, event;
    DWORD toRead;
    FT_STATUS status;

    file.open(QFile::WriteOnly);
    uint leftToReceive = recPackSize;
    while (leftToReceive != 0) {
        FT_GetStatus(ftHandle, &toRead, &tx, &event);
        status = FT_Read(ftHandle, rxBuffer, toRead, &read);
        if (status == FT_OK) {
            file.write(rxBuffer, read);
            leftToReceive -= read;
        }
    }
    file.flush();
    file.close();
}

void FtdiControl::setRecPackSize(uint newSize) {
    recPackSize = newSize;
}

bool FtdiControl::isOpened() const {
    return isOpen;
}
