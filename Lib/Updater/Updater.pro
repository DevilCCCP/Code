!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    Package.cpp \
    PackLoaderA.cpp \
    PackLoaderFile.cpp \
    Installer.cpp \
    UpChecker.cpp \
    Updater.cpp \
    UpSync.cpp \
    UpInfo.cpp \
    PackLoaderHttp.cpp \
    HttpUploader.cpp

HEADERS += \
    Package.h \
    PackLoaderA.h \
    PackLoaderFile.h \
    Installer.h \
    UpChecker.h \
    Updater.h \
    UpSync.h \
    UpInfo.h \
    PackLoaderHttp.h \
    HttpUploader.h

win32 {
SOURCES += \
    Win/WinUtils.cpp

HEADERS += \
    Win/WinUtils.h

LIBS += \
    -lRstrtmgr
} unix {
SOURCES += \
    Linux/LinuxUtils.cpp

HEADERS += \
    Linux/LinuxUtils.h
}


LIBS += \
    -lDispatcher \
    -lCtrl \
    -lSettings \
    -lDb \
    -lLog

RESOURCES += \
    Updater.qrc

