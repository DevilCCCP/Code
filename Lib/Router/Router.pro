!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    RouterServer.cpp \
    RouterClient.cpp \
    Packet.cpp

HEADERS += \
    RouterServer.h \
    RouterClient.h \
    Packet.h

DEPEND_LIBS += \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

