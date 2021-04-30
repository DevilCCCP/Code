win32 {
 !exists("$$EXTERN_DIR/ffmpeg-3.2") {
   error(External library Ffmpeg 3.2 not found in Externs dir)
 }

 INCLUDEPATH += $$EXTERN_DIR/ffmpeg-3.2/include/
 msvc {
  !exists("$$EXTERN_DIR/ffmpeg-3.2/include/msvc") {
    error(External library Ffmpeg 3.2 includes for build msvc version not found)
  }
  INCLUDEPATH += $$EXTERN_DIR/ffmpeg-3.2/include/msvc
 }
 LIBS += -L$$EXTERN_DIR/ffmpeg-3.2/lib/
}

contains(QMAKE_HOST.arch, aarch64) {
DEFINES += \
    SWS_SCALE_BUG \
}

LIBS += \
    -lavcodec \
    -lavformat \
    -lavdevice \
    -lavutil \
    -lswscale

