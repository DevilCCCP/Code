!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += gui

SOURCES += \
    Decoder.cpp \
    Thumbnail.cpp \
    Codec.cpp \
    CodecA.cpp \
    Ffmpeg/FfmpegDec.cpp \
    Ffmpeg/FfmpegCodec.cpp \
    Ffmpeg/CodecF.cpp \
    Ffmpeg/Convert.cpp

HEADERS += \
    Decoder.h \
    Thumbnail.h \
    Codec.h \
    CodecA.h \
    Ffmpeg/FfmpegDec.h \
    Ffmpeg/FfmpegCodec.h \
    Ffmpeg/CodecF.h \
    Ffmpeg/Convert.h


LIBS += \
    -lSettings \
    -lLog \
    -lCommon


!include($$HEAD_DIR/LibV/Ffmpeg.pri) {
  error(Could not find the Ffmpeg.pri file!)
}

win32 {
 # --- MFX ----
 contains(INCLUDE_LIB, Mfx) {
 !isEmpty(INTELMEDIASDKROOT) {

  DEFINES += USE_MFX

  SOURCES +=  \
    Mfx/MfxDec.cpp \
    Mfx/CodecM.cpp \
    Mfx/MfxContainer.cpp \
    Mfx/FrameM.cpp

  HEADERS +=  \
    Mfx/MfxDec.h \
    Mfx/CodecM.h \
    Mfx/FrameM.h \
    Mfx/MfxContainer.h \
    Mfx/MfxDef.h

  !include($$HEAD_DIR/LibV/Mfx.pri) {
   error(Could not find the Mfx.pri file!)
  }
 }
 } # Mfx
} linux {
 contains(INCLUDE_LIB, Vdp) {
 system(dpkg -s libvdpau-dev>/dev/null 2>/dev/null) {
  # --- vdpau ----
  SOURCES +=  \
    Vdp/VdpDec.cpp \
    Vdp/CodecP.cpp \
    Vdp/VdpContext.cpp

  HEADERS +=  \
    Vdp/VdpDec.h \
    Vdp/CodecP.h \
    Vdp/VdpContext.h \
    Vdp/VdpDef.h

  !include($$HEAD_DIR/LibV/Vdp.pri) {
   error(Could not find the Vdp.pri file!)
  }
 }
 } # Vdp
 exists(/opt/vc/lib) {
 # --- OMX ----
  DEFINES += USE_OMX

  SOURCES +=  \
    Omx/OmxDec.cpp \
    Omx/CodecO.cpp \
    Omx/ilclient.c \
    Omx/ilcore.c

  HEADERS +=  \
    Omx/OmxDec.h \
    Omx/CodecO.h \
    Omx/IlDef.h \
    Omx/ilclient.h

  !include($$HEAD_DIR/LibV/Omx.pri) {
   error(Could not find the Omx.pri file!)
  }
 } # Omx
} # linux
