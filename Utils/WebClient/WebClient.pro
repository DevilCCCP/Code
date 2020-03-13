#-------------------------------------------------
#
# Project created by QtCreator 2016-09-07T10:41:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WebClient
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RC_FILE = Resource.rc

RESOURCES += \
    WebClient.qrc
