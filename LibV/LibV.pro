!include($$OUT_PWD/../Local.pri) {
  error(Could not find the Local.pri file!)
}

TEMPLATE = subdirs

CONFIG -= ordered

win32 {
 contains(DEFINES, VIDEO_ARM) {
  SUBDIRS += CtrlV
 }
}
SUBDIRS += Decoder
SUBDIRS += MediaServer
SUBDIRS += Player
SUBDIRS += Render
SUBDIRS += Source
SUBDIRS += Storage
SUBDIRS += Va
SUBDIRS += VideoUi
