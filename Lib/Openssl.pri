# Open SSL

win32 {
 OPENSSL_DIR=$$(OPENSSL_DIR)
 isEmpty(OPENSSL_DIR) {
   error(set OPENSSL_DIR to OpenSSL dir)
 }
 !exists("$$OPENSSL_DIR") {
   error(External library OpenSSL not found in $$OPENSSL_DIR dir)
 }
 !exists($$OPENSSL_DIR/lib/MinGW/libcrypto2.a) {
  OPENSSL_DIR_WIN=$$replace(OPENSSL_DIR, /, \\)
  system($$QMAKE_COPY_FILE $$OPENSSL_DIR_WIN/lib/MinGW/libcrypto.a $$OPENSSL_DIR_WIN/lib/MinGW/libcrypto2.a)
 }

 INCLUDEPATH += $$OPENSSL_DIR/include/
 LIBS += -L$$OPENSSL_DIR/lib/MinGW/

 mscv {
  LIBS += \
    -llibeay32MD \
    -lssleay32MD
 } gcc {
  LIBS += \
    -lcrypto2 \
    -lssl
 }
} else {
 gcc {
  LIBS += \
    -lcrypto \
    -lssl
 }
}
