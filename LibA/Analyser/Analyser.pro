!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += gui


SOURCES +=\
    ByteRegion.cpp \
    CellStatFtr.cpp \
    DigitFtr.cpp \
    Hyst.cpp \
    HystFast.cpp \
    ImageFilter.cpp \
    ImageStatFtr.cpp \
    PlateCalcFtr.cpp \
    PlateFindFtr.cpp \
    SignalMarkFtr.cpp \
    SymbolMarkFtr.cpp \
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
    ByteRegion.h \
    Cell.h \
    CellStatFtr.h \
    DigitFtr.h \
    FilterInfo.h \
    Hyst.h \
    HystFast.h \
    ImageFilter.h \
    ImageStatFtr.h \
    PlateCalcFtr.h \
    PlateFindFtr.h \
    Region.h \
    Signal.h \
    SignalMarkFtr.h \
    Symbol.h \
    SymbolMarkFtr.h \
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
