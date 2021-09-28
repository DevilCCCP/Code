!include(../CommonApp.pri) {
  error(Could not find the CommonApp.pri file!)
}


QT += core gui widgets

CONFIG -= console


SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    ImageLabel.cpp \
    GraphLabel.cpp \
    FormImageLineView.cpp \
    FormImageLineRegion.cpp \
    HystLabel.cpp

HEADERS  +=\
    MainWindow.h \
    ImageLabel.h \
    GraphLabel.h \
    FormImageLineView.h \
    FormImageLineRegion.h \
    HystLabel.h

LIBS += \
    -lAnalyser \
    -lUi

linux {
  PRE_TARGETDEPS += $$INNER_LIB_DIR/libAnalyser.a $$INNER_LIB_DIR/libUi.a
}

FORMS    +=\
    MainWindow.ui

RESOURCES += \
    ImageAnalizer.qrc

RC_FILE = Resource.rc
