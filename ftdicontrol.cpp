#include "ftdicontrol.h"

#include <QThread>
#include <QMessageBox>

FtdiControl::FtdiControl(QObject *parent) : QObject(parent)
{

}

int FtdiControl::openDevice() {
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE * info;
    DWORD numdevs;
    status = FT_CreateDeviceInfoList(&numdevs);
    if (status != FT_OK || numdevs == 0) {
        // no connected devices
        //QMessageBox::information(this, "FTDI", "No connected devices", QMessageBox::Ok);
        return 1;
    }
    status = FT_SetVIDPID(0x0403, 0x6010); // this operation is not necessary with root access and can be omitted further
    if (status != FT_OK) {
        //QMessageBox::warning(this, "Cannot set VID, PID", "Cannot set VID, PID", QMessageBox::Ok);
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
                //QMessageBox::information(this, "Cannot open device", "Cannot open device", QMessageBox::Ok);
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
    char txBuffer[FT_BUFFER_SIZE]; // actual sizef of data buffer could be changed according to the buffer from the other communication side
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
                //QMessageBox::information(this, "Not enough data", "Not enought data", QMessageBox::Ok);
                return;
            }
            for (int i = 0; i < FT_BUFFER_SIZE; ++i) {
                if (rxBuffer[i] != char(i % 256)) {
                    emit errorHappened();
//                    QMessageBox::information(this,
//                                             QString::asprintf("Unexpected data in pos %d\n", i),
//                                             QString::asprintf("Unexpected data in pos %d\n", i),
//                                             QMessageBox::Ok);
                    break;
                }
            }
        } else {

        }
    }
    emit loopbackSuccessful();
}

bool FtdiControl::isOpened() const {
    return isOpen;
}
