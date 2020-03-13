!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT +=

SOURCES += \
    Main.cpp

HEADERS += \

DEPEND_LIBS = \
    Crypto \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = CryptTest$$APP_EXTRA_EXTANTION
