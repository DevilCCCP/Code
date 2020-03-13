# live555
win32 {
 !exists("$$EXTERN_DIR/live") {
   error(External library live555 not found in Externs dir)
 }

INCLUDEPATH += $$EXTERN_DIR/live/include/
LIBS += -L$$EXTERN_DIR/live/lib/
} unix {
INCLUDEPATH += /usr/include/groupsock \
               /usr/include/liveMedia \
               /usr/include/UsageEnvironment \
               /usr/include/BasicUsageEnvironment
}

win32 {
LIVE_PREFIX = lib
} unix {
LIVE_PREFIX =
}

CONFIG(debug, debug|release) {
 win32 {
  LIBS += \
    -l$${LIVE_PREFIX}liveMedia-d \
    -l$${LIVE_PREFIX}BasicUsageEnvironment-d \
    -l$${LIVE_PREFIX}groupsock-d \
    -l$${LIVE_PREFIX}UsageEnvironment-d
 } else {
  LIBS += \
    -l$${LIVE_PREFIX}liveMedia \
    -l$${LIVE_PREFIX}BasicUsageEnvironment \
    -l$${LIVE_PREFIX}groupsock \
    -l$${LIVE_PREFIX}UsageEnvironment
 }
} else {
LIBS += \
    -l$${LIVE_PREFIX}liveMedia \
    -l$${LIVE_PREFIX}BasicUsageEnvironment \
    -l$${LIVE_PREFIX}groupsock \
    -l$${LIVE_PREFIX}UsageEnvironment
}
win32 {
LIBS += \
    -lWs2_32
}
