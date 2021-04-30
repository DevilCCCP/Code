!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    Package.cpp \
    Installer.cpp \
    UpChecker.cpp \
    Updater.cpp \
    UpSync.cpp \
    UpInfo.cpp

HEADERS += \
    Package.h \
    Installer.h \
    UpChecker.h \
    Updater.h \
    UpSync.h \
    UpInfo.h


LIBS += \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lDb \
    -lLog

