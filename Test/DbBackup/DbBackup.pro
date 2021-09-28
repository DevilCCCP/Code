!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += gui widgets sql

SOURCES += \
    Main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h

DEPEND_LIBS = \
    Dispatcher \
    Updater \
    DbUi \
    Settings \
    Db \
    Ctrl \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

FORMS += \
    MainWindow.ui

RC_FILE = Resource.rc
