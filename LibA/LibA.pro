!include($$OUT_PWD/../../Local.pri) {
  error(Could not find the Local.pri at '$$OUT_PWD/../../' file!)
}

TEMPLATE = subdirs

CONFIG -= ordered

SUBDIRS += Analyser
SUBDIRS += Analytics
