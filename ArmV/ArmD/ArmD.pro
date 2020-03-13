!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network core
CONFIG(release, debug|release) {
  CONFIG -= console
}

SOURCES += \
    Main.cpp \
    ArmLoader.cpp

HEADERS += \
    ArmLoader.h

DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_armd$$APP_EXTRA_EXTANTION

RC_FILE = Resource.rc
