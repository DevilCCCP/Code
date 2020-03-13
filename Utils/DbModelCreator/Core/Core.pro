#-------------------------------------------------
#
# Project created by QtCreator 2016-02-12T15:49:58
#
#-------------------------------------------------

QT       -= gui

BUILD_DIR = $$PWD/../bin

TARGET = Core
DESTDIR = $$BUILD_DIR/lib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += Core.cpp

HEADERS += Core.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    Core.qrc
