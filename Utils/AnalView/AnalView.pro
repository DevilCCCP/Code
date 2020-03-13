#-------------------------------------------------
#
# Project created by QtCreator 2015-07-14T08:04:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AnalView
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    SelectDlg.cpp

HEADERS  += MainWindow.h \
    SelectDlg.h

FORMS    += MainWindow.ui \
    SelectDlg.ui

RC_FILE = Resource.rc

RESOURCES += \
    AnalView.qrc
