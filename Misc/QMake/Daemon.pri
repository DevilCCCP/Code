!include($$PRI_DIR/Version.pri) {
  error(Could not find the Version.pri file at '$$PRI_DIR'!)
}


win32 {
  LIBS += \
    -ladvapi32
}

TARGET = $${APP_PREFIX}_$${DAEMON_NAME}d$$APP_EXTRA_EXTANTION

#license
!contains(DEFINES, NOLICENSE) {
 win32 {
  LIBS += \
    -lwbemuuid \
    -lOle32 \
    -lOleAut32
 }
}

