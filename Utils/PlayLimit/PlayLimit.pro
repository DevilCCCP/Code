QT       += core gui widgets

TARGET = PlayLimit
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    MainWindow.cpp \
    Main.cpp \
    Format.cpp

HEADERS += \
    MainWindow.h \
    Format.h

FORMS += \
    MainWindow.ui

RESOURCES += \
    Main.qrc
