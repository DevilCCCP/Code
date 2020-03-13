!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}


SOURCES += \
    CameraForm.cpp \
    ThumbnailReceiver.cpp \
    DecodeReceiver.cpp \
    MonitorForm.cpp \
    LayoutLabel.cpp \
    ImageWithPoints.cpp \
    FrameLabel.cpp \
    StorageForm.cpp

HEADERS += \
    CameraForm.h \
    ThumbnailReceiver.h \
    DecodeReceiver.h \
    MonitorForm.h \
    LayoutLabel.h \
    ImageWithPoints.h \
    FrameLabel.h \
    StorageForm.h


LIBS = \
    -lDecoder \
    -lNet \
    -lDispatcher \
    -lCtrl \
    -lDb \
    -lLog


RESOURCES += \
    VideoUi.qrc

FORMS += \
    CameraForm.ui \
    MonitorForm.ui \
    StorageForm.ui

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

