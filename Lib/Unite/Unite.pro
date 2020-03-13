!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    UniteHandler.cpp \
    UniteInfo.cpp \
    UniteAgentA.cpp \
    UniteObject.cpp

HEADERS += \
    UniteHandler.h \
    UniteInfo.h \
    UniteAgentA.h \
    UniteObject.h


DEPEND_LIBS += \
    Dispatcher \
    Ctrl \
    NetServer \
    Net \
    Db \
    Settings \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

