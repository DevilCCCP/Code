!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += sql network

SOURCES += \
    Main.cpp \
    UpDLoader.cpp

HEADERS += \
    UpDLoader.h


DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

DAEMON_NAME = up
DAEMON_PATH = $$PWD
!include($$PRI_DIR/Daemon.pri) {
  error(Could not find the Daemon.pri file!)
}

RC_FILE = Resource.rc

