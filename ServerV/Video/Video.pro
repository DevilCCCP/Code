!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += sql network core gui

CONFIG(debug, debug|release) {
 DEFINES += ANAL_DEBUG
 QT += widgets
}

contains(INCLUDE_LIB, Media) {
 DEFINES += MEDIA_INFO
}

contains(INCLUDE_LIB, Analytics) {
 DEFINES += USE_ANALIZER
}

SOURCES += \
    Main.cpp \
    Saver.cpp \
    TrReceiver.cpp \
    Transmit.cpp

HEADERS += \
    Saver.h \
    TrReceiver.h \
    Transmit.h


DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    Decoder \
    Source \
    MediaServer \
    Storage \
    Va \
    Analytics \
    Analyser \
    Db \
    Net \
    Settings \
    Log

CONFIG(debug, debug|release) {
 DEPEND_LIBS += Ui
}

!contains(INCLUDE_LIB, Analytics) {
 DEPEND_LIBS -= Analyser
 DEPEND_LIBS -= Analytics
}

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

include($$PROJECT_DIR/Local/Analytics.pri)

win32 {
  QMAKE_POST_LINK  = VideoDeploy.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)
}

contains(INCLUDE_LIB, Analytics) {
 TARGET = $${APP_PREFIX}_va$$APP_EXTRA_EXTANTION
} else {
 TARGET = $${APP_PREFIX}_video$$APP_EXTRA_EXTANTION
}

