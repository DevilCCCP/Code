!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

!include($$HEAD_DIR/Local/Icons.pri) {
  error(Could not find the Icons.pri file!)
}


QT += sql network core gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}

CONFIG(release, debug|release) {
  CONFIG -= console
}


SOURCES += \
    Main.cpp

HEADERS += \


contains(INCLUDE_LIB, Analytics) {
 DEFINES += USE_EVENTS
}

DEPEND_LIBS = \
    Monitoring \
    VideoUi \
    DbUi \
    Ui \
    Dispatcher \
    Settings \
    Updater \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_mon$$APP_EXTRA_EXTANTION

RC_FILE = Resource.rc
