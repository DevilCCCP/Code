#-------------------------------------------------
#
# Project created by QtCreator 2016-09-07T19:59:19
#
#-------------------------------------------------

QT       += core gui serialport


TARGET = SerialClientConsole
TEMPLATE = app

CONFIG += console


SOURCES += main.cpp

HEADERS  +=


unix {
  QMAKE_CXXFLAGS += -std=c++0x
} win32 {
  QMAKE_CFLAGS += -Zc:wchar_t-
  QMAKE_CXXFLAGS += -Zm200
}
