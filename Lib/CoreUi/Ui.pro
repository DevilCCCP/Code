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
    FormImage.cpp \
    QWidgetB.cpp \
    Version.cpp \
    FormImageView.cpp \
    FormImageRegion.cpp \
    Icon.cpp \
    QScrollAreaZoom.cpp

HEADERS += \
    DockWidget2.h \
    MainWindow2.h \
    FormSection.h \
    FormImage.h \
    QWidgetB.h \
    Version.h \
    FormImageView.h \
    FormImageRegion.h \
    Icon.h \
    QScrollAreaZoom.h \
    ImagePainter.h

FORMS += \
    FormSection.ui \
    FormImage.ui \
    FormImageView.ui


LIBS += \

RESOURCES += \
    Ui.qrc

