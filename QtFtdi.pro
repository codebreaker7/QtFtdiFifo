#-------------------------------------------------
#
# Project created by QtCreator 2018-10-24T20:58:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtFtdi
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/local/lib/release/ -lftd2xx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/local/lib/debug/ -lftd2xx
else:unix: LIBS += -L$$PWD/../../../../../../usr/local/lib/ -lftd2xx

INCLUDEPATH += $$PWD/../../../../Downloads/release
DEPENDPATH += $$PWD/../../../../Downloads/release
