!include(../CommonLib.pri) {
  error(Could not find the CommonLib.pri file!)
}

QT += core gui widgets concurrent

CONFIG -= console


SOURCES += \
    MapGenerator.cpp \
    FormMapPreview.cpp \
    MapParameters.cpp \
    Plate.cpp \
    FormEarth.cpp

HEADERS += \
    MapGenerator.h \
    FormMapPreview.h \
    MapParameters.h \
    Plate.h \
    FormEarth.h \
    EarthLandscape.h

FORMS += \
    FormMapPreview.ui \
    FormEarth.ui


LIBS += \
    -lUi \
    -lCommon


RESOURCES += \
    LibZ.qrc

