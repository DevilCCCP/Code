!include($$OUT_PWD/../Local.pri) {
  error(Could not find the Local.pri file!)
}

TEMPLATE = subdirs

Ctrl.depends = Log
Db.depends = Log
DbUi.depends = Log Db
Settings.depends = Log Db
Dispatcher.depends = Log Ctrl Settings
Net.depends = Log Ctrl
NetServer.depends = Log Settings Ctrl Net
Updater.depends = Log Settings Ctrl Net Db UpdaterCore
UpdaterCore.depends = Log Ctrl Net

# build must be last:
CONFIG -= ordered

SUBDIRS += Common
SUBDIRS += Crypto
SUBDIRS += Ctrl
SUBDIRS += Db
SUBDIRS += Dispatcher
SUBDIRS += Log
SUBDIRS += Net
SUBDIRS += NetServer
SUBDIRS += Router
SUBDIRS += Settings
SUBDIRS += Updater
SUBDIRS += UpdaterCore

!contains(EXCLUDE_LIB, Ui) {
 SUBDIRS += DbUi
 SUBDIRS += Monitoring
 SUBDIRS += Ui
}
contains(INCLUDE_LIB, Backup) {
 SUBDIRS += Backup
}
contains(INCLUDE_LIB, Reporter) {
 SUBDIRS += Reporter
 SUBDIRS += Smtp
}
contains(INCLUDE_LIB, Smtp) {
 SUBDIRS += Smtp
}
contains(INCLUDE_LIB, Unite) {
 SUBDIRS += Unite
 Unite.depends = Settings Dispatcher NetServer
}
contains(INCLUDE_LIB, GoogleApi) {
 SUBDIRS += GoogleApi
 GoogleApi.depends = Settings
}
