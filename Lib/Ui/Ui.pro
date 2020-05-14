!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += gui

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
}


SOURCES += \
    DockWidget2.cpp \
    MainWindow2.cpp \
    FormSection.cpp \
    FormUpdateSync.cpp \
    UpWaiter.cpp \
    FormImage.cpp \
    QWidgetB.cpp \
    CheckableTreeView.cpp \
    CheckItemDelegate.cpp \
    DialogName.cpp \
    QSpinBoxZ.cpp \
    ScheduleWidget.cpp \
    FormSchedule.cpp \
    GraphWidget.cpp \
    FormWorkSpace.cpp \
    UserIdleEventFilter.cpp

HEADERS += \
    DockWidget2.h \
    MainWindow2.h \
    FormSection.h \
    FormUpdateSync.h \
    UpWaiter.h \
    FormImage.h \
    QWidgetB.h \
    CheckableTreeView.h \
    CheckItemDelegate.h \
    DialogName.h \
    QSpinBoxZ.h \
    ScheduleWidget.h \
    FormSchedule.h \
    GraphWidget.h \
    FormWorkSpace.h \
    UserIdleEventFilter.h


LIBS += \
    -lUpdater \
    -lCommon \
    -lLog

RESOURCES += \
    Ui.qrc

FORMS += \
    FormSection.ui \
    FormUpdateSync.ui \
    FormImage.ui \
    DialogName.ui \
    FormSchedule.ui \
    FormWorkSpace.ui

