!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network concurrent

SOURCES += \
    MainInfo.cpp \
    ProcessManager.cpp \
    Dispatcher.cpp \
    ModuleLoaderA.cpp \
    Overseer.cpp \
    Imp.cpp \
    Conveyor.cpp \
    ModuleLoaderD.cpp \
    ListenSvc.cpp \
    SettingMLoader.cpp \
    ModuleLoaderO.cpp \
    ImpD.cpp \
    JobImp.cpp \
    LogPublisher.cpp \
    LogCleaner.cpp

HEADERS += \
    MainInfo.h \
    ProcessManager.h \
    Dispatcher.h \
    ModuleLoaderA.h \
    Overseer.h \
    Imp.h \
    Conveyor.h \
    ModuleLoaderD.h \
    ModuleInfo.h \
    ListenSvc.h \
    SettingMLoader.h \
    OverseerState.h \
    ModuleLoaderO.h \
    ModuleStatic.h \
    ModuleDb.h \
    OverseerThread.h \
    ImpD.h \
    JobImp.h \
    LogPublisher.h \
    LogCleaner.h \
    Tools.h

win32 {
SOURCES += \
    Win/WinService.cpp \
    Win/WinTools.cpp

HEADERS += \
    Win/WinService.h \
    Win/WinTools.h

LIBS += \
    -ladvapi32
} unix {
SOURCES += \
    Linux/LinuxService.cpp
HEADERS += \
    Linux/LinuxService.h \
    Linux/LinuxTools.h
}


LIBS += \
    -lUpdater \
    -lSettings \
    -lDb \
    -lCtrl \
    -lLog \
    -lCommon

