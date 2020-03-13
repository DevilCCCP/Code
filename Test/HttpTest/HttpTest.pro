!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


QT += network

SOURCES += \
    Main.cpp

HEADERS += 

LIBS += \
  -lLog \
  -lCtrl \
  -lNetServer \
  -lNet

unix {
#LIBS += \
}

