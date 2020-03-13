!include($$OUT_PWD/../Local.pri) {
  error(Could not find the Local.pri file!)
}

TEMPLATE = subdirs

# build must be last:
CONFIG -= ordered

SUBDIRS += StoreCreator
SUBDIRS += Video

