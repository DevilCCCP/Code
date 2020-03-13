TEMPLATE = lib

CONFIG += staticlib

BUILD_POSTFIX = .lib/

!include(Main.pri) {
  error(Could not find the Main.pri file!)
}
