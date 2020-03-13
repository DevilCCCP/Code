!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    UniteHandler.cpp \
    Main.cpp \
    UniteInfo.cpp \
    UniteAgent.cpp

HEADERS += \
    UniteHandler.h \
    UniteInfo.h \
    UniteAgent.h


DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    NetServer \
    Net \
    Db \
    Settings \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_unite$$APP_EXTRA_EXTANTION

