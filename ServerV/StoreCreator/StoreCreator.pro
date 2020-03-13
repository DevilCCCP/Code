!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql

SOURCES += \
    Main.cpp \
    Creator.cpp

HEADERS += \
    Creator.h

DEPEND_LIBS = \
    Dispatcher \
    Ctrl \
    Storage \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

TARGET = $${APP_PREFIX}_repc$$APP_EXTRA_EXTANTION

win32 {
  QMAKE_POST_LINK  = $$PWD/repc.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)

  OTHER_FILES += \
    repc.bat
} linux {
  QMAKE_POST_LINK  = /bin/bash $$PWD/repc.sh $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)

  OTHER_FILES += \
    repc.sh
}
