#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ftd2xx.h"

namespace Ui {
class MainWindow;
}

#define FT_BUFFER_SIZE 4096

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void selectFileToSend();
    void sendDataFromFile();
    void sendLoopback();
    void searchAndOpenDevice();
private:
    Ui::MainWindow *ui;
    FT_HANDLE ftHandle;

    void searchConnectedDevice();
    void setupFifoMode();
};

#endif // MAINWINDOW_H
