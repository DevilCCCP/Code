!include($$OUT_PWD/../Local.pri) {
  error(Could not find the Local.pri file!)
}

TEMPLATE = subdirs

# build must be last:
CONFIG -= ordered

SUBDIRS += Installer
SUBDIRS += UpD
SUBDIRS += UpLoader
!contains(DEFINES, NOLICENSE) {
 SUBDIRS += LicenseLoader
}
contains(DEFINES, ROUTER) {
 SUBDIRS += TcpRouter
}
