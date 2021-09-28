!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    Main.cpp

HEADERS += \

DEPEND_LIBS = \
    GoogleApi \
    Crypto \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

