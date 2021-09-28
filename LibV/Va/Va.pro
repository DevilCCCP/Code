!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += sql network core gui

CONFIG(debug, debug|release) {
 QT += widgets
 DEFINES += ANAL_DEBUG
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

CONFIG(debug, debug|release) {
 SOURCES += \
    Wnd/DebugWnd.cpp \
    Wnd/FrameWnd.cpp \
    Wnd/FrameWindow.cpp \
    Wnd/DialogFrame.cpp


 HEADERS += \
    Wnd/DebugWnd.h \
    Wnd/FrameWnd.h \
    Wnd/FrameWindow.h \
    Wnd/DialogFrame.h

FORMS += \
    Wnd/DialogFrame.ui \
    Wnd/FrameWindow.ui
}


LIBS = \
    -lDecoder \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lCommon \
    -lDb \
    -lLog

RESOURCES += \
    $$PROJECT_DIR/Local/Video.qrc

