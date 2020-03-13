!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    Main.cpp \
    Key.cpp \
    LicenseHandler.cpp

HEADERS += \
    LicenseHandler.h


DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    NetServer \
    Net \
    Db \
    Settings \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = License$$APP_EXTRA_EXTANTION

#license
win32 {
LIBS += \
    -lwbemuuid
} unix {
}
