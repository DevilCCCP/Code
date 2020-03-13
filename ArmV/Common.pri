HEAD_DIR = $$PWD/..

!include($$HEAD_DIR/Local.pri) {
  error(Could not find the Local.pri file!)
}
!include($$HEAD_DIR/Misc/QMake/AppCommon.pri) {
  error(Could not find the AppCommon.pri file!)
}
