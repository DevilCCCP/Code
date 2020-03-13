# Intel Media SDK MFX
win32 {
 INTEL_MEDIA_SDK_PATH = $$(INTELMEDIASDKROOT)
 isEmpty(INTEL_MEDIA_SDK_PATH) {
  error(Intel media SDK not installed)
 }
 INCLUDEPATH += $$INTEL_MEDIA_SDK_PATH/include/
 LIBS += -L$$INTEL_MEDIA_SDK_PATH/lib/win32
} unix {
}
