!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


SOURCES += \
    CtrlWorker.cpp \
    CtrlManager.cpp \
    WorkerStat.cpp

HEADERS += \
    CtrlWorker.h \
    CtrlManager.h \
    ManagerThread.h \
    WorkerStat.h


LIBS += \
    -lLog \
    -lCommon

