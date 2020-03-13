!include($$PWD/Mfx.pri) {
  error(Could not find the Mfx.pri file!)
}

win32 {
 CONFIG(debug, debug|release) {
  LIBS += \
    -llibmfx_d
 } else {
  LIBS += \
    -llibmfx
 }
}
