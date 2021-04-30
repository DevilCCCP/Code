!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network gui widgets


SOURCES += \
    Downloader.cpp \
    FileSaver.cpp \
    CameraPlayer.cpp \
    Ffmpeg/FfmpegOut.cpp \
    PlayerSettings.cpp

HEADERS += \
    FrameReceiver.h \
    Downloader.h \
    FileSaver.h \
    CameraPlayer.h \
    Ffmpeg/FfmpegOut.h \
    CameraInfo.h \
    PlayerSettings.h

contains(DEFINES, VIDEO_ARM) {
 SOURCES +=  \
    Render.cpp \
    Drawer.cpp \
    WndProcA.cpp \
    Sdl/WndProcL.cpp \
    Sdl/SdlDrawer.cpp \
    Sdl/SdlPlayer.cpp

 HEADERS += \
    Render.h \
    Drawer.h \
    Icons.h \
    ArmState.h \
    WndProcA.h \
    Sdl/WndProcL.h \
    Sdl/SdlDrawer.h \
    Sdl/SdlPlayer.h

 win32 {
 QT += winextras

 SOURCES += \
    Win/WndProc.cpp \
    Win/DirectDraw.cpp \
    Win/DdrawSurface.cpp \
    Win/DdrawDrawer.cpp \
    Win/ToolsWnd.cpp

 HEADERS += \
    Win/WndProc.h \
    Win/DirectDraw.h \
    Win/DdrawSurface.h \
    Win/DdrawDrawer.h \
    Win/DdrawErrors.h \
    Win/ToolsWnd.h \
    Win/GdiplusEngine.h \
    Win/CtrlWndDef.h
 }
} contains(DEFINES, USE_SDL2) {
 SOURCES +=  \
    Sdl/SdlPlayer.cpp

 HEADERS += \
    Sdl/SdlPlayer.h
}


LIBS = \
    -lCtrl \
    -lSettings \
    -lDispatcher \
    -lDecoder \
    -lNet \
    -lDb \
    -lLog

# ffmpeg
!include($$HEAD_DIR/LibV/Ffmpeg.pri) {
  error(Could not find the Ffmpeg.pri file!)
}

# ddraw
win32 {
 LIBS += -L$$EXTERN_DIR/ddraw/
 LIBS += -lddraw
}

# SDL
win32 {
 INCLUDEPATH += $$EXTERN_DIR/sdl2/include/
 LIBS += -L$$EXTERN_DIR/sdl2/lib/x86/
 LIBS += \
    -lSDL2
} else {
 INCLUDEPATH += /usr/include/SDL2/
 LIBS += \
    -lSDL2
}

# Win API
win32 {
 LIBS += \
    -luser32 \
    -lGdi32 \
    -lgdiplus \
    -lComctl32
}

RESOURCES += \
    Player.qrc
