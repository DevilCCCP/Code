SRC_LOCAL=$$PWD/Va/Local.pri
DST_LOCAL=$$OUT_PWD/Local.pri
win32:SRC_LOCAL=$$replace(SRC_LOCAL, /, \\)
win32:DST_LOCAL=$$replace(DST_LOCAL, /, \\)
system($$QMAKE_COPY_FILE $$SRC_LOCAL $$DST_LOCAL)

TEMPLATE = subdirs

# build must be last:
CONFIG += ordered

SUBDIRS += Lib
SUBDIRS += LibV
SUBDIRS += Server
SUBDIRS += ServerV
SUBDIRS += Test
SUBDIRS += Va/ArmZ
