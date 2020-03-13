!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network core gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG(release, debug|release) {
  CONFIG -= console
}

SOURCES += \
    Main.cpp \
    stdafx.cpp \
    MainWindowY.cpp

HEADERS += \
    stdafx.h \
    MainWindowY.h

PRECOMPILED_HEADER = stdafx.h
PRECOMPILED_SOURCE = stdafx.cpp

DEPEND_LIBS = \
    Ui \
    DbUi \
    Player \
    Decoder \
    Net \
    Dispatcher \
    Ctrl \
    Settings \
    Db \
    Ctrl \
    Log

!include($$HEAD_DIR/LibV/Ffmpeg.pri) {
  error(Could not find the Ffmpeg.pri file!)
}

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_admin$$APP_EXTRA_EXTANTION

RC_FILE = Resource.rc

RESOURCES += \
    Resource.qrc

