system($$QMAKE_COPY_FILE $$PWD/Japuzzle/Local.pri $$OUT_PWD/Local.pri)

TEMPLATE = subdirs

# build must be last:
CONFIG += ordered

SUBDIRS += Lib/CoreUi/Ui.pro
SUBDIRS += Japuzzle/Japuzzle
