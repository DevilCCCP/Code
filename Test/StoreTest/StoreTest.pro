!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT +=

SOURCES += \
    Main.cpp

HEADERS += \

DEPEND_LIBS = \
    Storage \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = StoreTest$$APP_EXTRA_EXTANTION
