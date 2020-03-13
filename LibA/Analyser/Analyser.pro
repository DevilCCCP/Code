!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += gui


SOURCES +=\
    UinPre.cpp \
    Uin.cpp \
    SignalMark.cpp \
    Analyser.cpp \
    SignalMark2.cpp \
    ObjConnect.cpp \
    SignalMark3.cpp \
    UinPlate.cpp \
    UinAreaStat.cpp

HEADERS  +=\
    UinPre.h \
    Uin.h \
    SignalMark.h \
    Analyser.h \
    SignalMark2.h \
    ObjConnect.h \
    SignalMark3.h \
    UinPlate.h \
    UinMetrics.h \
    UinAreaStat.h

LIBS += \

RESOURCES += \
    Analyser.qrc
