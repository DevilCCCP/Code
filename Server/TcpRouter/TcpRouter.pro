!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    Main.cpp

HEADERS += \


DEPEND_LIBS = \
    Router \
    Dispatcher \
    Ctrl \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}


TARGET = $${APP_PREFIX}_tcp$$APP_EXTRA_EXTANTION
