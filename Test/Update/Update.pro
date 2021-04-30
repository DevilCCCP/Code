!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT +=

DEFINES += NOLICENSE

SOURCES += \
    Main.cpp

HEADERS += \


DEPEND_LIBS = \
    UpdaterCore \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

win32 {
 msvc {
  LIBS += \
    -lrstrtmgr
 } gcc {
  LIBS += \
    -lrstrmgr
 }
}


TARGET = $${APP_PREFIX}_up$$APP_EXTRA_EXTANTION
