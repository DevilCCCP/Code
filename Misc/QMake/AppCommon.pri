TEMPLATE = app

CONFIG += console

BUILD_POSTFIX = 
unix {
  APP_EXTRA_EXTANTION = .exe
}
win32 {
  APP_EXTRA_EXTANTION = 
}

!include(Main.pri) {
  error(Could not find the Main.pri file!)
}

#license
!contains(DEFINES, NOLICENSE) {
 win32 {
  LIBS += \
    -lwbemuuid \
    -lOle32 \
    -lOleAut32
 }
}

