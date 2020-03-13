# Open Max Raspberry Pi version

DEFINES += \
    STANDALONE \
    __STDC_CONSTANT_MACROS \
    __STDC_LIMIT_MACROS \
    TARGET_POSIX \
    _LINUX \
    PIC \
    _REENTRANT \
    _LARGEFILE64_SOURCE \
    _FILE_OFFSET_BITS=64 \
    HAVE_LIBOPENMAX=2 \
    OMX \
    OMX_SKIP64BIT \
    USE_EXTERNAL_OMX \
    HAVE_LIBBCM_HOST \
    USE_EXTERNAL_LIBBCM_HOST \
    USE_VCHIQ_ARM

INCLUDEPATH += \
    /opt/vc/include \
    /opt/vc/include \
    /opt/vc/include/interface/vcos/pthreads \
    /opt/vc/include/interface/vmcs_host/linux


LIBS += \
    -L/opt/vc/lib


LIBS += \
    -lGLESv2 \
    -lEGL \
    -lopenmaxil \
    -lbcm_host \
    -lvcos \
    -lvchiq_arm \
    -lpthread \
    -lrt \
    -lm

QMAKE_CXXFLAGS += \
    -Wwrite-strings \
    -Wsign-compare

QMAKE_CFLAGS += \
    -Wwrite-strings \
    -Wsign-compare
