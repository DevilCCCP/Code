
!isEmpty(SRC_DIR) {
  for(SRC_DIR_ONE, SRC_DIR) {
    DIR_LOCAL=$$shell_path($$PROJECT_DIR/Local/$$SRC_DIR_ONE)
    DIR_RELEASE=$$shell_path($$BUILD_DIR/)
    system($$QMAKE_COPY_DIR $$DIR_LOCAL $$DIR_RELEASE)
  }
}
