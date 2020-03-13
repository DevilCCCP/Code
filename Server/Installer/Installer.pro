!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    Main.cpp

HEADERS += \


DEPEND_LIBS = \
    Updater \
    Dispatcher \
    Ctrl \
    Settings \
    Db \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

linux {
 QMAKE_POST_LINK = /bin/bash $$PWD/inst.sh $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)

 OTHER_FILES += \
    inst.sh
} win32 {
 QMAKE_POST_LINK  = inst.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)

 OTHER_FILES += \
    inst.bat

 msvc {
  LIBS += \
    -lrstrtmgr
 } gcc {
  LIBS += \
    -lrstrmgr
 }
}


TARGET = $${APP_PREFIX}_inst$$APP_EXTRA_EXTANTION
