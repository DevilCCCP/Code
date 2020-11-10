!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += sql network core gui

win32 {
 QT += widgets
}

contains(INCLUDE_LIB, Media) {
 DEFINES += MEDIA_INFO
}

contains(INCLUDE_LIB, Analytics) {
 DEFINES += USE_ANALIZER
} else {
 SOURCES += \
    NoAnalytics.cpp
}

SOURCES += \
    AnalyticsA.cpp \
    Analizer.cpp

HEADERS += \
    AnalyticsA.h \
    Va.h \
    Analizer.h \
    SceneAnalizer.h

win32 {
 SOURCES += \
    DebugWnd.cpp \
    Win/FrameWndWin.cpp

 HEADERS += \
    DebugWnd.h \
    Win/FrameWndWin.h
}


LIBS = \
    -lDecoder \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lCommon \
    -lDb \
    -lLog

win32 {
LIBS += \
    -luser32 \
    -lGdi32
} unix {
}

RESOURCES += \
    $$PROJECT_DIR/Local/Video.qrc

