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
    MainWindowZ.cpp

HEADERS += \
    MainWindowZ.h


DEPEND_LIBS = \
    VideoUi \
    DbUi \
    Updater \
    Ui \
    Net \
    Decoder \
    Settings \
    Db \
    Dispatcher \
    Ctrl \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_admin$$APP_EXTRA_EXTANTION

RC_FILE = Resource.rc

RESOURCES += \
    Admin.qrc

