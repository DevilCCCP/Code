!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    LiLoader.cpp \
    Main.cpp

HEADERS += \
    LiLoader.h


DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    Net \
    Db \
    Settings \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

#license
win32 {
LIBS += \
    -lwbemuuid
} unix {
}

TARGET = $${APP_PREFIX}_lilo$$APP_EXTRA_EXTANTION

