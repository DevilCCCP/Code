!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    BackupA.cpp

HEADERS += \
    BackupA.h


LIBS += \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lDb \
    -lLog

