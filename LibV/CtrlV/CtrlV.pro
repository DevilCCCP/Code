!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += sql network gui widgets


SOURCES += \
    CtrlV.cpp

HEADERS += \
    CtrlV.h

win32 {
SOURCES += \
    Win/BackWnd.cpp \
    Win/BackWndProc.cpp \
    Win/ToolWndProc.cpp


HEADERS += \
    Win/BackWnd.h \
    Win/BackWndProc.h \
    Win/ToolWndProc.h
}



DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

# Win API
win32 {
LIBS += \
    -luser32 \
    -lGdi32
}

