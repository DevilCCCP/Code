!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += gui network

SOURCES += \
    Source.cpp \
    SourceFfmpeg/FfmpegIn.cpp \
    SourceFfmpeg/SourceFfmpeg.cpp \
    SourceScript/SourceScript.cpp \
    SourceScript/ScriptIn.cpp \
    SourceState.cpp \
    DvrA.cpp \
    FrameChannel.cpp \
    SourceChild.cpp \
    Ptz.cpp \
    Ptz/PtzOnvif.cpp \
    SourceProc/SourceProc.cpp

HEADERS += \
    Source.h \
    SourceFfmpeg/FfmpegIn.h \
    SourceFfmpeg/SourceFfmpeg.h \
    SourceScript/SourceScript.h \
    SourceScript/ScriptIn.h \
    SourceState.h \
    SourceTypes.h \
    DvrA.h \
    FrameChannel.h \
    SourceChild.h \
    Ptz.h \
    Ptz/PtzOnvif.h \
    SourceProc/SourceProc.h

contains(INCLUDE_LIB, Live555) {
 DEFINES += USE_LIVE555

 SOURCES += \
    SourceLive555/MediaSinkImpl.cpp \
    SourceLive555/SourceLive.cpp

 HEADERS += \
    SourceLive555/Def.h \
    SourceLive555/MediaSinkImpl.h \
    SourceLive555/SourceLive.h
}

linux {
 DEFINES += USE_V4L
 SOURCES += \
    Linux/LinuxUsbDevice.cpp \
    Linux/MmapBuffer.cpp \
    SourceV4l/SourceV4l.cpp \
    SourceV4l/V4lIn.cpp

 HEADERS += \
    Linux/LinuxUsbDevice.h \
    Linux/FileDescriptor.h \
    Linux/MmapBuffer.h \
    SourceV4l/SourceV4l.h \
    SourceV4l/V4lIn.h
}


LIBS += \
    -lMediaServer \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lLog

contains(INCLUDE_LIB, Live555) {
 !include($$HEAD_DIR/LibV/Live555.pri) {
   error(Could not find the Live555.pri file!)
 }
}

!include($$HEAD_DIR/LibV/Ffmpeg.pri) {
  error(Could not find the Ffmpeg.pri file!)
}

RESOURCES += \
    Ptz.qrc
