!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += network gui


HEADERS += \
    FpsCalc.h \
    TrafficCalc.h \
    Format.h \
    Icon.h \
    Profiler.h \
    Tlv.h \
    TlvParser.h \
    Uri.h \
    CsvWriter.h \
    CsvReader.h \
    Version.h \
    StringUtils.h \
    FpsDown.h \
    Var.h \
    Translit.h \
    AutoCodec.h \
    HystText.h \
    DirCopy.h \
    AutoLang.h \
    FilesPackage.h \
    HwId.h \
    FormatTr.h

SOURCES += \
    FpsCalc.cpp \
    TrafficCalc.cpp \
    Format.cpp \
    Icon.cpp \
    Profiler.cpp \
    Tlv.cpp \
    TlvParser.cpp \
    Uri.cpp \
    CsvWriter.cpp \
    CsvReader.cpp \
    Version.cpp \
    StringUtils.cpp \
    FpsDown.cpp \
    Var.cpp \
    Translit.cpp \
    AutoCodec.cpp \
    HystText.cpp \
    DirCopy.cpp \
    AutoLang.cpp \
    FilesPackage.cpp \
    HwId.cpp \
    FormatTr.cpp

TRANSLATIONS += Common_ru.ts
#"/usr/lib/qt5/bin/lrelease" '/home/devil/!Code/!Code/Lib/Common/Common.pro'

RESOURCES += \
    Common.qrc
