!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    MediaServer.cpp \
    Media.cpp \
    Channel.cpp \
    Rtsp/RtspServer.cpp \
    Rtsp/RtspMedia.cpp \
    Rtsp/Sdp.cpp \
    Rtsp/RtspHandler.cpp \
    Rtsp/RtspChannel.cpp \
    Rtsp/RtspInit.cpp \
    H264/H264Sprop.cpp \
    H264/H264NalUnit.cpp \
    MediaPlayerV.cpp \
    Rtsp/RtspPackager.cpp \
    VideoSysInfo.cpp \
    MpController.cpp

HEADERS += \
    MediaServer.h \
    Media.h \
    Channel.h \
    TrFrame.h \
    Rtsp/RtspServer.h \
    Rtsp/RtspMedia.h \
    Rtsp/Sdp.h \
    Rtsp/RtspHandler.h \
    MediaPlayer.h \
    Rtsp/SdpExtantion.h \
    Rtsp/RtspChannel.h \
    Rtsp/RtspInit.h \
    Rtsp/RtspCamReceiver.h \
    H264/H264Sprop.h \
    H264/H264NalUnit.h \
    H264/H264Sps.h \
    H264/H264Pps.h \
    MediaPlayerV.h \
    MediaPackager.h \
    Rtsp/RtspPackager.h \
    VideoSysInfo.h \
    MpController.h


LIBS += \
    -lNetServer \
    -lNet \
    -lSettings \
    -lCtrl \
    -lLog

