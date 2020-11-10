!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

!include($$HEAD_DIR/$$PROJECT_NAME/Local/Icons.pri) {
  error(Could not find the Icons.pri at '$$HEAD_DIR/$$PROJECT_NAME/Local/' file!)
}


QT += sql gui widgets


SOURCES += \
    MonitoringWindow.cpp \
    FormSystem.cpp \
    FormSysLog.cpp \
    FormEvents.cpp \
    Core.cpp

HEADERS += \
    MonitoringWindow.h \
    FormSystem.h \
    FormSysLog.h \
    FormEvents.h \
    Core.h

contains(INCLUDE_LIB, Analytics) {
 DEFINES += USE_EVENTS
}


LIBS = \
    -lDbUi \
    -lUi \
    -lSettings \
    -lUpdater \
    -lDb \
    -lLog

FORMS += \
    MonitoringWindow.ui \
    FormSystem.ui \
    FormSysLog.ui \
    FormEvents.ui

RESOURCES += \
    Monitoring.qrc

