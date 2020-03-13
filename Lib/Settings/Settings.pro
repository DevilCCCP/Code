!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql

SOURCES += \
    FileSettings.cpp \
    DbSettings.cpp \
    MemSettings.cpp \
    Schedule.cpp \
    ObjectSchedule.cpp

HEADERS += \
    SettingsA.h \
    FileSettings.h \
    DbSettings.h \
    MemSettings.h \
    Schedule.h \
    ObjectSchedule.h


LIBS += \
    -lDb \
    -lCommon \
    -lLog

