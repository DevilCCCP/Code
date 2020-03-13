!include($$PWD/Ffmpeg.pri) {
  error(Could not find the Ffmpeg.pri file!)
}

win32 {
 TARGETDEPS += $$PWD/Ffmpeg.bat
 QMAKE_POST_LINK  += $$PWD/Ffmpeg.bat $$shell_quote($$HEAD_DIR) $$shell_quote($$DESTDIR)

 OTHER_FILES += \
    $$PWD/Ffmpeg.bat
}
