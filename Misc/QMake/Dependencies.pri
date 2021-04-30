msvc: LIB_EXT=lib
gcc:  LIB_EXT=a

msvc: LIB_PREFIX=
gcc:  LIB_PREFIX=lib

contains(DEPEND_LIBS, Db) {
 !contains(DEPEND_LIBS, Crypto) {
  DEPEND_LIBS += Crypto
 }
}
!contains(DEPEND_LIBS, Common) {
 DEPEND_LIBS += Common
}

for(dep, DEPEND_LIBS) {
  #message($$TARGET depends on $$dep ($${INNER_LIB_DIR}/$${LIB_PREFIX}$${dep}.$${LIB_EXT}))
  LIBS += -l$${dep}
  PRE_TARGETDEPS += $${INNER_LIB_DIR}/$${LIB_PREFIX}$${dep}.$${LIB_EXT}
}

contains(DEPEND_LIBS, Crypto) {
 !include($$HEAD_DIR/Lib/Openssl.pri) {
   error(Could not find the Openssl.pri file!)
 }
}

contains(DEPEND_LIBS, Decoder) {
 win32{
  contains(INCLUDE_LIB, Mfx) {
  !isEmpty(INTELMEDIASDKROOT) {
   !include($$HEAD_DIR/LibV/MfxDeploy.pri) {
    error(Could not find the MfxDeploy.pri file!)
   }
  }
  }
 }

 !include($$HEAD_DIR/LibV/FfmpegDeploy.pri) {
   error(Could not find the FfmpegDeploy.pri file!)
 }

 linux {
  system(dpkg -s libvdpau-dev>/dev/null 2>/dev/null) {
   !include($$HEAD_DIR/LibV/Vdp.pri) {
    error(Could not find the Vdp.pri file!)
   }
  }
  exists("/opt/vc/src/hello_pi") {
   !include($$HEAD_DIR/LibV/Omx.pri) {
     error(Could not find the Omx.pri file!)
   }
  }
 }
}

contains(DEPEND_LIBS, Source): contains(INCLUDE_LIB, Video) {
 win32{
  contains(INCLUDE_LIB, Live555) {
   !include($$HEAD_DIR/LibV/Live555.pri) {
    error(Could not find the Live555.pri file!)
   }
  }
 }

 !include($$HEAD_DIR/LibV/Live555.pri) {
   error(Could not find the Live555.pri file!)
 }
 !include($$HEAD_DIR/LibV/FfmpegDeploy.pri) {
   error(Could not find the FfmpegDeploy.pri file!)
 }
}

contains(DEFINES, USE_SDL2) {
  LIBS += \
    -lSDL2
}
