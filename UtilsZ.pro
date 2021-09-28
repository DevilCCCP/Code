system($$QMAKE_COPY_FILE $$PWD/Utils/Local.pri $$OUT_PWD/Local.pri)

TEMPLATE = subdirs

# build must be last:
CONFIG += ordered

SUBDIRS += Lib/CoreUi/Ui.pro
SUBDIRS += LibA/Analyser

SUBDIRS += Utils/CalcTraining
SUBDIRS += Utils/PlayLimit
SUBDIRS += Utils/ImageAnalizer
SUBDIRS += Utils/RenameDir
