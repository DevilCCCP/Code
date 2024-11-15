!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

!include($$PROJECT_DIR/Local/Icons.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network core gui widgets

CONFIG(release, debug|release) {
  CONFIG -= console
}


SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    FormSourceStore.cpp \
    StorageScaner.cpp \
    FormDestStore.cpp \
    StorageTransfer.cpp \
    DbSaver.cpp

HEADERS += \
    MainWindow.h \
    FormSourceStore.h \
    StorageScaner.h \
    FormDestStore.h \
    Info.h \
    StorageTransfer.h \
    DbSaver.h

contains(INCLUDE_LIB, Analytics) {
 DEFINES += USE_ANALIZER
}

DEPEND_LIBS = \
    Storage \
    Ui \
    DbUi \
    Dispatcher \
    Settings \
    Updater \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_rep_ui$$APP_EXTRA_EXTANTION

FORMS += \
    MainWindow.ui \
    FormSourceStore.ui \
    FormDestStore.ui

RC_FILE = Resource.rc

RESOURCES += \
    StorageUi.qrc

