!include(../../CommonApp.pri) {
  error(Could not find the CommonApp.pri file!)
}


QT += core gui

CONFIG += console


SOURCES +=\
    Main.cpp \
    DbPrepare.cpp \
    Log.cpp

HEADERS  +=\
    DbPrepare.h \
    Log.h

LIBS += \

