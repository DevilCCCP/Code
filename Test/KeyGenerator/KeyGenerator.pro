!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


SOURCES += \
    Main.cpp

HEADERS +=


DEPEND_LIBS = \
    Crypto \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}
