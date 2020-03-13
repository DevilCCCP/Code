HEAD_DIR = $$PWD/..

!include($$HEAD_DIR/Local.pri) {
  error(Could not find the Local.pri file!)
}
!include(../../Misc/AppCommon.pri) {
  error(Could not find the AppCommon.pri file!)
}
