!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    ReporterA.cpp

HEADERS += \
    ReporterA.h


LIBS += \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lDb \
    -lLog

