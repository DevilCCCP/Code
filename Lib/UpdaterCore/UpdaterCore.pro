!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += sql network

SOURCES += \
    PackLoaderA.cpp \
    PackLoaderFile.cpp \
    PackLoaderHttp.cpp \
    HttpUploader.cpp \
    InstallerSimple.cpp \
    UpdateSettings.cpp \
    BackgroundLoader.cpp \
    HttpUploaderSimple.cpp \
    PackLoaderHttpSimple.cpp \
    PackageCore.cpp \
    InstallerCore.cpp

HEADERS += \
    PackLoaderA.h \
    PackLoaderFile.h \
    PackLoaderHttp.h \
    HttpUploader.h \
    Tools.h \
    UpdateSettings.h \
    InstallerSimple.h \
    BackgroundLoader.h \
    HttpUploaderSimple.h \
    PackLoaderHttpSimple.h \
    PackageCore.h \
    InstallerCore.h


LIBS += \
    -lLog

win32 {
SOURCES += \
    Win/WinUtils.cpp

HEADERS += \
    Win/WinUtils.h
} unix {
SOURCES += \
    Linux/LinuxUtils.cpp

HEADERS += \
    Linux/LinuxUtils.h
}

RESOURCES += \
    Updater.qrc

