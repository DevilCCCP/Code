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


TARGET = $${APP_PREFIX}_upl$$APP_EXTRA_EXTANTION

win32 {
 msvc {
  QMAKE_POST_LINK  = $$PWD/common.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)
 }

 msvc {
  LIBS += \
    -lrstrtmgr
 } gcc {
  LIBS += \
    -lrstrmgr
 }
}

