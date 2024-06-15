#-------------------------------------------------
#
# Project created by QtCreator 2016-09-07T19:59:19
#
#-------------------------------------------------

QT       += core gui serialport widgets

TARGET = SerialClient
TEMPLATE = app


SOURCES += Main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

unix {
  QMAKE_CXXFLAGS += -std=c++0x
} win32 {
  QMAKE_CFLAGS += -Zc:wchar_t-
  QMAKE_CXXFLAGS += -Zm200
}

RESOURCES += \
  SerialClient.qrc
