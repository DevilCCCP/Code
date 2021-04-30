!include($$OUT_PWD/../Local.pri) {
  error(Could not find the Local.pri file!)
}

TEMPLATE = subdirs

CONFIG -= ordered

!contains(DEFINES, NOLICENSE) {
 SUBDIRS += License
}
contains(INCLUDE_LIB, Install) {
 SUBDIRS += KeyGenerator
}
contains(INCLUDE_LIB, AllTest) {
 SUBDIRS += DbTest
 SUBDIRS += HttpTest
 SUBDIRS += NetTest
 SUBDIRS += JobTest
}
contains(INCLUDE_LIB, StoreTest) {
 SUBDIRS += StoreTest
}
contains(INCLUDE_LIB, DbBackup) {
 SUBDIRS += DbBackup
}
contains(INCLUDE_LIB, Update) {
 SUBDIRS += Update
}
#SUBDIRS += CryptTest
