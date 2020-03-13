!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}


HEADERS += \
    Rsa.h \
    Jwt.h \
    Tools.h \
    InnerCrypt.h \
    Xtea.h

SOURCES += \
    Rsa.cpp \
    Jwt.cpp \
    InnerCrypt.cpp \
    Xtea.cpp


LIBS += \
    -lLog \
    -lCommon

!include(../Openssl.pri) {
  error(Could not find the Openssl.pri file!)
}

RESOURCES += \
    InnerData.qrc
