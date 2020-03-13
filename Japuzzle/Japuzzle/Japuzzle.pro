!include(../Common.pri) {
  error(Could not find the Common.pri file!)
}
!include($$PRI_DIR/Version.pri) {
  error(Could not find the Version.pri file at '$$PRI_DIR'!)
}

QT += core gui widgets concurrent

CONFIG -= console


SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    Core.cpp \
    Puzzle.cpp \
    DigitsWidget.cpp \
    TableWidget.cpp \
    Account.cpp \
    Style.cpp \
    DialogAccount.cpp \
    AccountModel.cpp \
    PreviewWidget.cpp \
    UiInformer.cpp \
    Decoration.cpp \
    Editing.cpp \
    DialogSettings.cpp \
    Ai.cpp \
    GameState.cpp \
    DialogGameState.cpp \
    FormCalcPopup.cpp \
    DialogAbout.cpp \
    DialogSolve.cpp \
    DialogList.cpp \
    CreatorMainWindow.cpp \
    EditWidget.cpp

HEADERS += \
    MainWindow.h \
    Core.h \
    Cell.h \
    Puzzle.h \
    DigitsWidget.h \
    TableWidget.h \
    Account.h \
    Style.h \
    StyleInfo.h \
    DialogAccount.h \
    AccountInfo.h \
    AccountModel.h \
    PreviewWidget.h \
    CoreInfo.h \
    UiInformer.h \
    Decoration.h \
    Editing.h \
    UndoInfo.h \
    Propotion.h \
    DialogSettings.h \
    Ai.h \
    GameState.h \
    DialogGameState.h \
    FormCalcPopup.h \
    DialogAbout.h \
    DialogSolve.h \
    DialogList.h \
    CreatorMainWindow.h \
    EditWidget.h

FORMS += \
    MainWindow.ui \
    DialogAccount.ui \
    DialogSettings.ui \
    DialogGameState.ui \
    FormCalcPopup.ui \
    DialogAbout.ui \
    DialogSolve.ui \
    DialogList.ui \
    CreatorMainWindow.ui


LIBS += \
    -lUi


RC_FILE = Resource.rc

RESOURCES += \
    Japuzzle.qrc


APP_EXTRA_EXTANTION =
