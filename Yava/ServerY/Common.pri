HEAD_DIR = $$PWD/..

!include(../../Misc/AppCommon.pri) {
  error(Could not find the AppCommon.pri file!)
}
!include(../Local.pri) {
  error(Could not find the Local.pri file!)
}
