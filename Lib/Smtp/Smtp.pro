!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    Smtp.cpp

HEADERS += \
    Smtp.h \
    SmtpMail.h


LIBS += \
    -lNet \
    -lLog \
    -lCtrl

