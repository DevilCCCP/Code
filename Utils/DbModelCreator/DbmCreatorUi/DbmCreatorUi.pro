#-------------------------------------------------
#
# Project created by QtCreator 2016-02-12T15:51:24
#
#-------------------------------------------------

QT       += core gui widgets

BUILD_DIR = $$PWD/../bin

TARGET = DbmCreatorUi
DESTDIR = $$BUILD_DIR
TEMPLATE = app


SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    MainWindow2.cpp

HEADERS  += \
    MainWindow.h \
    MainWindow2.h

FORMS    += \
    MainWindow.ui


LIBS += \
    -L$$BUILD_DIR/lib \
    -lCore

linux {
 TARGETDEPS += $$BUILD_DIR/lib/libCore.a
}

RC_FILE = Resource.rc

RESOURCES += \
    Ui.qrc

