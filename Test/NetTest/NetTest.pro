!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    Main.cpp \
    Pinger.cpp

HEADERS += \
    Pinger.h

DEPEND_LIBS = \
    Log \
    Ctrl \
    Net

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

