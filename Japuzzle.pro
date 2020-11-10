SRC_LOCAL=$$shell_path($$PWD/Japuzzle/Local.pri)
DST_LOCAL=$$shell_path($$OUT_PWD/Local.pri)
system($$QMAKE_COPY_FILE $$SRC_LOCAL $$DST_LOCAL)

TEMPLATE = subdirs

# build must be last:
CONFIG += ordered

SUBDIRS += Lib/CoreUi/Ui.pro
SUBDIRS += Japuzzle/Japuzzle
