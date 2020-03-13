!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}


SOURCES += \
    QtRender.cpp \
    SourceReceiver.cpp

HEADERS += \
    Render.h \
    QtRender.h \
    SourceReceiver.h


LIBS = \
    -lCtrl \
    -lDispatcher \
    -lDecoder \
    -lLog

# ffmpeg
!include($$HEAD_DIR/LibV/Ffmpeg.pri) {
  error(Could not find the Ffmpeg.pri file!)
}

# OMX
linux:exists("/opt/vc/lib") {
DEFINES += USE_OMX

SOURCES +=  \
    Omx/OmxRender.cpp

HEADERS +=  \
    Omx/OmxRender.h

!include($$HEAD_DIR/LibV/Omx.pri) {
  error(Could not find the Omx.pri file!)
}
}

