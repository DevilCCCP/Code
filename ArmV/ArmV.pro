!include($$OUT_PWD/../Local.pri) {
  error(Could not find the Local.pri file!)
}

TEMPLATE = subdirs

# build must be last:
CONFIG -= ordered


SUBDIRS += \
    Monitoring \
    StorageUi

win32 {
 contains(DEFINES, VIDEO_ARM) {
  SUBDIRS += \
    ArmD \
    Player \
 }
}
