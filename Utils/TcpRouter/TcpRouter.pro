QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle


SOURCES += \
    Main.cpp \
    RouterServer.cpp \
    RouterClient.cpp

HEADERS += \
    RouterServer.h \
    Def.h \
    RouterClient.h
