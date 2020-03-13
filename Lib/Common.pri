HEAD_DIR = $$PWD/..

!include($$OUT_PWD/../../Local.pri) {
  error(Could not find the Local.pri at '$$OUT_PWD/../../' file!)
}
!include($$HEAD_DIR/Misc/QMake/LibCommon.pri) {
  error(Could not find the LibCommon.pri file at Misc!)
}
