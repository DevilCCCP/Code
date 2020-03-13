!include(../CommonApp.pri) {
  error(Could not find the CommonApp.pri file!)
}


QT += core gui widgets

CONFIG -= console


SOURCES +=\
    MainWindow.cpp \
    Main.cpp

HEADERS  +=\
    MainWindow.h

LIBS += \
    -lUi

FORMS    +=\
    MainWindow.ui

RESOURCES += \
    FfmpegUi.qrc

RC_FILE = Resource.rc
