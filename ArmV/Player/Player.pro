!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}


CONFIG(release, debug|release) {
  CONFIG -= console
}

SOURCES += \
    Main.cpp \
    DownloadDialog.cpp \
    DownloadPlayer.cpp

HEADERS += \
    DownloadDialog.h

DEPEND_LIBS = \
    Player \
    Decoder \
    Dispatcher \
    Net \
    Settings \
    Db \
    Ctrl \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

# ddraw
win32 {
LIBS += -L$$EXTERN_DIR/ddraw/
LIBS += -lddraw
}

# SDL
INCLUDEPATH += $$EXTERN_DIR/sdl2/include/
LIBS += -L$$EXTERN_DIR/sdl2/lib/x86/
LIBS += \
    -lSDL2

# Win API
win32 {
LIBS += \
    -lComctl32 \
    -luser32 \
    -lGdi32 \
    -lgdiplus
}

TARGET = $${APP_PREFIX}_plr$$APP_EXTRA_EXTANTION

win32 {
  QMAKE_POST_LINK  = $$PWD/PlayerDeploy.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)
}

FORMS += \
    DownloadDialog.ui

RESOURCES += \
    Resource.qrc
