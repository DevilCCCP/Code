!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}
!include($$PRI_DIR/Version.pri) {
  error(Could not find the Version.pri file at '$$PRI_DIR'!)
}

QT += core gui widgets concurrent

CONFIG -= console


SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    Core.cpp

HEADERS += \
    MainWindow.h \
    Core.h

FORMS += \
    MainWindow.ui


DEPEND_LIBS = \
    LibZ \
    Ui

!include($$PRI_DIR/DependenciesCore.pri) {
  error(Could not find the Dependencies.pri file!)
}


RC_FILE = Resource.rc

RESOURCES += \
    World.qrc

APP_EXTRA_EXTANTION =
