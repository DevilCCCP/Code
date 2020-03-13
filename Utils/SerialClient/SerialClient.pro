#-------------------------------------------------
#
# Project created by QtCreator 2016-09-07T19:59:19
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialClient
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

unix {
  QMAKE_CXXFLAGS += -std=c++0x
} win32 {
  QMAKE_CFLAGS += -Zc:wchar_t-
  QMAKE_CXXFLAGS += -Zm200
}
