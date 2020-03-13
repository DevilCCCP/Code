#pragma once
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <QString>

#include <bcm_host.h>
extern "C" {
#include "ilclient.h"
}

const int kDecodePortIn = 130;
const int kDecodePortOut = 131;
const int kRenderPortIn = 90;
const int kSoundPortIn = 100;

inline QString OmxErrorToString(int code)
{
  switch (code) {
  case OMX_ErrorInsufficientResources: return "OMX_ErrorInsufficientResources";
  case OMX_ErrorUndefined: return "OMX_ErrorUndefined";
  case OMX_ErrorInvalidComponentName: return "OMX_ErrorInvalidComponentName";
  case OMX_ErrorComponentNotFound: return "OMX_ErrorComponentNotFound";
  case OMX_ErrorInvalidComponent: return "OMX_ErrorInvalidComponent";
  case OMX_ErrorBadParameter: return "OMX_ErrorBadParameter";
  case OMX_ErrorNotImplemented: return "OMX_ErrorNotImplemented";
  case OMX_ErrorUnderflow: return "OMX_ErrorUnderflow";
  case OMX_ErrorOverflow: return "OMX_ErrorOverflow";
  case OMX_ErrorHardware: return "OMX_ErrorHardware";
  case OMX_ErrorInvalidState: return "OMX_ErrorInvalidState";
  case OMX_ErrorStreamCorrupt: return "OMX_ErrorStreamCorrupt";
  case OMX_ErrorPortsNotCompatible: return "OMX_ErrorPortsNotCompatible";
  case OMX_ErrorResourcesLost: return "OMX_ErrorResourcesLost";
  case OMX_ErrorNoMore: return "OMX_ErrorNoMore";
  case OMX_ErrorVersionMismatch: return "OMX_ErrorVersionMismatch";
  case OMX_ErrorNotReady: return "OMX_ErrorNotReady";
  case OMX_ErrorTimeout: return "OMX_ErrorTimeout";
  case OMX_ErrorSameState: return "OMX_ErrorSameState";
  case OMX_ErrorResourcesPreempted: return "OMX_ErrorResourcesPreempted";
  case OMX_ErrorPortUnresponsiveDuringAllocation: return "OMX_ErrorPortUnresponsiveDuringAllocation";
  case OMX_ErrorPortUnresponsiveDuringDeallocation: return "OMX_ErrorPortUnresponsiveDuringDeallocation";
  case OMX_ErrorPortUnresponsiveDuringStop: return "OMX_ErrorPortUnresponsiveDuringStop";
  case OMX_ErrorIncorrectStateTransition: return "OMX_ErrorIncorrectStateTransition";
  case OMX_ErrorIncorrectStateOperation: return "OMX_ErrorIncorrectStateOperation";
  case OMX_ErrorUnsupportedSetting: return "OMX_ErrorUnsupportedSetting";
  case OMX_ErrorUnsupportedIndex: return "OMX_ErrorUnsupportedIndex";
  case OMX_ErrorBadPortIndex: return "OMX_ErrorBadPortIndex";
  case OMX_ErrorPortUnpopulated: return "OMX_ErrorPortUnpopulated";
  case OMX_ErrorComponentSuspended: return "OMX_ErrorComponentSuspended";
  case OMX_ErrorDynamicResourcesUnavailable: return "OMX_ErrorDynamicResourcesUnavailable";
  case OMX_ErrorMbErrorsInFrame: return "OMX_ErrorMbErrorsInFrame";
  case OMX_ErrorFormatNotDetected: return "OMX_ErrorFormatNotDetected";
  case OMX_ErrorContentPipeOpenFailed: return "OMX_ErrorContentPipeOpenFailed";
  case OMX_ErrorContentPipeCreationFailed: return "OMX_ErrorContentPipeCreationFailed";
  case OMX_ErrorSeperateTablesUsed: return "OMX_ErrorSeperateTablesUsed";
  case OMX_ErrorTunnelingUnsupported: return "OMX_ErrorTunnelingUnsupported";
  default: return QString("code %1").arg(code);
  }
}

class IlComponents
{
  _COMPONENT_T* mList[3];

public:
  _COMPONENT_T* Decoder() const { return mList[0]; }
  _COMPONENT_T*& Decoder() { return mList[0]; }
  _COMPONENT_T* Render() const { return mList[1]; }
  _COMPONENT_T*& Render() { return mList[1]; }
  _COMPONENT_T* Sound() const { return mList[0]; }
  _COMPONENT_T*& Sound() { return mList[0]; }
  OMX_HANDLETYPE DecodeHandle() const { return ilclient_get_handle(Decoder()); }
  OMX_HANDLETYPE RenderHandle() const { return ilclient_get_handle(Render()); }
  OMX_HANDLETYPE SoundHandle() const { return ilclient_get_handle(Sound()); }

public:
  static void IlErrorCallback(void*, COMPONENT_T*, OMX_U32 data)
  {
    static OMX_U32 gLastErrorCode = 0;
    static int     gLastErrorBase = 1;
    static int     gLastErrorCount = 0;

    if (gLastErrorCode == data) {
      gLastErrorCount++;
      if (gLastErrorCount < gLastErrorBase) {
        return;
      } else {
        gLastErrorBase <<= 1;
      }
    } else {
      gLastErrorCode = data;
      gLastErrorBase = 1;
      gLastErrorCount = 0;
    }

    Log.Warning(QString("Omx error (text: %1, cnt: %2").arg(OmxErrorToString(data)).arg(gLastErrorCount));
  }


public:
  IlComponents()
  {
    memset(mList, 0, sizeof(mList));
  }

  ~IlComponents()
  {
    ilclient_cleanup_components(mList);
  }
};

class IlTunnels
{
  TUNNEL_T mTunnels[2];

public:
  const TUNNEL_T* DecoderToRender() const { return mTunnels; }
  TUNNEL_T* DecoderToRender() { return mTunnels; }

public:
  IlTunnels()
  {
    memset(mTunnels, 0, sizeof(mTunnels));
  }

  ~IlTunnels()
  {
    ilclient_teardown_tunnels(mTunnels);
  }
};
