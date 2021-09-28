!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

QT += network


HEADERS += \
    GoogleApi.h \
    ServiceAccounts.h \
    RestSender.h \
    GDrive.h \
    Scope.h \
    GSheet.h

SOURCES += \
    GoogleApi.cpp \
    ServiceAccounts.cpp \
    RestSender.cpp \
    GDrive.cpp \
    Scope.cpp \
    GSheet.cpp

DEPEND_LIBS = \
    GoogleApi \
    Crypto \
    Log
