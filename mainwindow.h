#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ftd2xx.h"
#include "ftdicontrol.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void selectFileToSend();
    void selectFileToReceive();
    void sendDataFromFile();
    void sendLoopback();
    void searchAndOpenDevice();

    void reportError();
    void reportSuccess();
private:
    FtdiControl * control;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
