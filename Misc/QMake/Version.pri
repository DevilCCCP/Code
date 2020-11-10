SRC_LOCAL=$$shell_path($$PROJECT_DIR/Local/Version.ini)
DST_LOCAL=$$shell_path($$BUILD_DIR/Version.ini)
system($$QMAKE_COPY_FILE $$SRC_LOCAL $$DST_LOCAL)
