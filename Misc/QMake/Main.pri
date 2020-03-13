isEmpty(HEAD_DIR) {
  error(HEAD_DIR must be defined!)
}

unix{
  WARNINGS += -Wall
}

unix {
  APP_EXTRA_EXTANTION = .exe
}
win32 {
  APP_EXTRA_EXTANTION =
}


QT -= gui

CONFIG += warn_on

PRI_DIR = $$PWD
PROJECT_DIR = $$HEAD_DIR/$$PROJECT_NAME
EXTERN_DIR = $$HEAD_DIR/Extern

CONFIG(release, debug|release) {
  BUILD_DIR = $$PROJECT_DIR/bin/release
}
CONFIG(debug, debug|release) {
  BUILD_DIR = $$PROJECT_DIR/bin/debug
}

DESTDIR = $$BUILD_DIR/$$BUILD_POSTFIX

INNER_LIB_DIR = $$BUILD_DIR/.lib


INCLUDEPATH += \
  $$HEAD_DIR \
  $$PROJECT_DIR \
  $$EXTERN_DIR

LIBS += \
  -L$$INNER_LIB_DIR

gcc {
  QMAKE_CXXFLAGS += -std=c++11
} msvc {
  QMAKE_CFLAGS += -Zc:wchar_t-
  QMAKE_CXXFLAGS += -Zm200
  QMAKE_LFLAGS += /NODEFAULTLIB:libcmt
}

contains(INCLUDE_LIB, Video) {
 !contains(EXCLUDE_LIB, Mfx) {
  INCLUDE_LIB += Mfx
 }
 !contains(EXCLUDE_LIB, Live555) {
  INCLUDE_LIB += Live555
 }
}
