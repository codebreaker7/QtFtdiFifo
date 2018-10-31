#ifndef FTDICONTROL_H
#define FTDICONTROL_H

#include <QObject>
#include "ftd2xx.h"

#define FT_BUFFER_SIZE 4096

class FtdiControl : public QObject
{
    Q_OBJECT
public:
    explicit FtdiControl(QObject *parent = 0);
signals:
    void errorHappened();
    void loopbackSuccessful();
public slots:

private:
    FT_HANDLE ftHandle;
    bool isOpen = false; // to control handle state

public:
    int openDevice();
    int goToFifoMode();
    void sendDataLoopback();
    bool isOpened() const;
    void sendDataFromFile(const QString&);
};

#endif // FTDICONTROL_H
