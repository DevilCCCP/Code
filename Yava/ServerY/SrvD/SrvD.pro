!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

!win32 {
!include($$HEAD_DIR/Lib/Dispatcher/qtservice/common.pri) {
  error(Could not find the 'qtservice.pri' file!)
}
}

QT += sql network

SOURCES += \
    stdafx.cpp \
    ServerLoader.cpp

win32 {
 SOURCES += WinMain.cpp
} else {
 SOURCES += LinuxMain.cpp
}

HEADERS += \
    stdafx.h \
    ServerLoader.h

PRECOMPILED_HEADER = stdafx.h
PRECOMPILED_SOURCE = stdafx.cpp

DEPEND_LIBS = \
    Ctrl \
    Dispatcher \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

DAEMON_NAME = srv
DAEMON_PATH = $$PWD
!include($$PRI_DIR/Daemon.pri) {
  error(Could not find the Daemon.pri file!)
}

RC_FILE = Resource.rc

RESOURCES += \
    ver.qrc
