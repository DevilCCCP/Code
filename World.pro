SRC_LOCAL=$$shell_path($$PWD/World/Local.pri)
DST_LOCAL=$$shell_path($$OUT_PWD/Local.pri)
system($$QMAKE_COPY_FILE $$SRC_LOCAL $$DST_LOCAL)

TEMPLATE = subdirs

# build must be last:
CONFIG += ordered

SUBDIRS += Lib/CoreUi/Ui.pro
SUBDIRS += Lib/Common/Common.pro
SUBDIRS += World/LibZ
SUBDIRS += World/World
SUBDIRS += World/Server

RESOURCES += \
    World/Server/Server.qrc
