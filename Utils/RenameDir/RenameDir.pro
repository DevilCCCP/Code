QT += core gui widgets

TARGET = RenameDir
TEMPLATE = app


SOURCES += \
        MainWindow.cpp \
    Main.cpp \
    RenameWorker.cpp

HEADERS += \
        MainWindow.h \
    RenameWorker.h

FORMS += \
        MainWindow.ui

RESOURCES += \
    RenameDir.qrc
