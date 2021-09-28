# Open SSL

win32 {
 OPENSSL_DIR=$$(OPENSSL_DIR)
 isEmpty(OPENSSL_DIR) {
   error(set OPENSSL_DIR to OpenSSL dir)
 }
 !exists("$$OPENSSL_DIR") {
   error(External library OpenSSL not found in $$OPENSSL_DIR dir)
 }
 exists($$OPENSSL_DIR/lib/MinGW) {
   OPENSSL_LIB_DIR=$$OPENSSL_DIR/lib/MinGW
 } else {
   OPENSSL_LIB_DIR=$$OPENSSL_DIR/lib
 }
 exists($$OPENSSL_LIB_DIR/libcrypto.a) {
  !exists($$OPENSSL_LIB_DIR/libcrypto2.a) {
   warning(Open SSL lib link need copy of libcrypto.a)
   OPENSSL_DIR_WIN=$$replace(OPENSSL_LIB_DIR, /, \\)
   system($$QMAKE_COPY_FILE $$OPENSSL_DIR_WIN\libcrypto.a $$OPENSSL_DIR_WIN\libcrypto2.a)
  }
 }
 exists($$OPENSSL_LIB_DIR/libcrypto.lib) {
  !exists($$OPENSSL_LIB_DIR/libcrypto2.lib) {
   warning(Open SSL lib link need copy of libcrypto.lib)
   OPENSSL_DIR_WIN=$$replace(OPENSSL_LIB_DIR, /, \\)
   system($$QMAKE_COPY_FILE $$OPENSSL_DIR_WIN\libcrypto.lib $$OPENSSL_DIR_WIN\libcrypto2.lib)
  }
 }

 INCLUDEPATH += $$OPENSSL_DIR/include/
 LIBS += -L$$OPENSSL_LIB_DIR

 mscv {
  LIBS += \
    -llibeay32MD \
    -lssleay32MD
 } gcc {
  LIBS += \
    -llibcrypto2 \
    -llibssl
 }
} else {
  LIBS += \
    -lcrypto \
    -lssl
}
