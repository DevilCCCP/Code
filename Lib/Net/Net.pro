!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    Listener.cpp \
    Responder.cpp \
    Requester.cpp \
    NetMessage.cpp \
    Chater.cpp \
    Messenger.cpp \
    QTcpServer2.cpp \
    Receiver.cpp \
    ChaterManager.cpp \
    SyncSocket.cpp

HEADERS += \
    Listener.h \
    Responder.h \
    Requester.h \
    NetMessage.h \
    Chater.h \
    Messenger.h \
    QTcpSocket2.h \
    QTcpServer2.h \
    Receiver.h \
    ChaterManager.h \
    SyncSocket.h \
    QSslSocket2.h


LIBS += \
    -lCtrl \
    -lLog \
    -lCommon

