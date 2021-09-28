!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}

message(DbTest is absolutly out of time)

QT += sql

SOURCES += \
    Main.cpp \
    Tester.cpp \
    TestPlane.cpp \
    TestQSqlQueryModel.cpp \
    TestBig.cpp \
    TestBigModel.cpp

HEADERS += \
    Tester.h \
    Test.h \
    TestPlane.h \
    TestQSqlQueryModel.h \
    TestBig.h \
    TestBigModel.h

DEPEND_LIBS = \
    Log \
    Db \
    Ctrl

!include($$PRI_DIR/Dependencies.pri) {
  error(Could not find the Dependencies.pri file!)
}
