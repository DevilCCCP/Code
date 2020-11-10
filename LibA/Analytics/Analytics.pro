!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += gui

win32 {
 QT += widgets
}

SOURCES += \
    AnalyticsB.cpp \
    MacroMotion.cpp \
    DiffLayers.cpp \
    BlockObj.cpp \
    DlWorker.cpp \
    CarMotion.cpp \
    CarDetection.cpp \
    CarTracker.cpp

HEADERS += \
    AnalyticsB.h \
    MacroMotion.h \
    DiffLayers.h \
    BlockScene.h \
    BlockSceneAnalizer.h \
    BlockObj.h \
    PointHst.h \
    DlWorker.h \
    Analytics.h \
    CarMotion.h \
    CarDetection.h \
    CarTracker.h \
    CarDef.h

LIBS += \
    -lAnalyser \
    -lVa \
    -lSettings \
    -lCommon \
    -lDb \
    -lLog

RESOURCES += \
    $$HEAD_DIR/Local/Video.qrc
