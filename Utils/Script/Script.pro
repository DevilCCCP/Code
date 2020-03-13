system($$QMAKE_COPY_FILE $$PWD/../Local.pri $$OUT_PWD/Local.pri)

TEMPLATE = subdirs

# build must be last:
CONFIG += ordered

SUBDIRS += DbPrepare
