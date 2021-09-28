HEAD_DIR = $$PWD/..

!include($$OUT_PWD/../../Local.pri) {
  error(Could not find the Local.pri file at '$$OUT_PWD/../../'!)
}
!include($$HEAD_DIR/Misc/QMake/AppCommon.pri) {
  error(Could not find the AppCommon.pri file at Misc!)
}
