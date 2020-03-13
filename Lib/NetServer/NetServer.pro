!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    NetServer.cpp \
    ServerResponder.cpp \
    Handler.cpp \
    HandlerManager.cpp \
    HttpHandler.cpp

HEADERS += \
    NetServer.h \
    ServerResponder.h \
    Handler.h \
    HandlerManager.h \
    HttpHandler.h


LIBS += \
    -lCtrl \
    -lLog \
    -lCommon

