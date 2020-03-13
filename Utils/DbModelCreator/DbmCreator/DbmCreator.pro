QT += core
QT -= gui

BUILD_DIR = $$PWD/../bin

TARGET = DbmCreator
DESTDIR = $$BUILD_DIR
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp


LIBS += \
    -L$$BUILD_DIR/lib \
    -lCore
