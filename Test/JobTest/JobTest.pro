!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += gui widgets sql concurrent

SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    Calc.cpp \
    MultiThreadCalc.cpp \
    MultiProcessCalc.cpp \
    TestJob.cpp

HEADERS += \
    MainWindow.h \
    Calc.h \
    MultiThreadCalc.h \
    MultiProcessCalc.h \
    TestJob.h

DEPEND_LIBS = \
    Updater \
    Dispatcher \
    DbUi \
    Db \
    Ctrl \
    Settings \
    Log

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}

FORMS += \
    MainWindow.ui

DISTFILES += \
    TestJob.sql
