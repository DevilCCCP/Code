!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += sql

SOURCES += \
    Storage.cpp \
    DbIndex.cpp \
    Container.cpp

HEADERS += \
    Storage.h \
    DbIndex.h \
    Container.h \
    Cell.h


LIBS += \
    -lLog \
    -lDb 

