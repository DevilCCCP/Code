!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
} else {
  QT += gui
}


SOURCES += \
    Main.cpp

HEADERS += \


PRECOMPILED_HEADER = stdafx.h
PRECOMPILED_SOURCE = stdafx.cpp

DEPEND_LIBS = \
    CtrlV \
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

TARGET = $${APP_PREFIX}_ctrl$$APP_EXTRA_EXTANTION

win32 {
  QMAKE_POST_LINK  = icons.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)
}
