#include <Lib/Log/Log.h>
#include <LibV/Decoder/Omx/IlDef.h>

#ifdef USE_OMX
#include "OmxRender.h"
#endif


#define LogLive(X)
//#define LogLive Log.Debug

bool OmxRender::InitDecoder()
{
  mFirstFrame = true;

  bcm_host_init();
  mIlClient = _ILCLIENT_TS(ilclient_init(), ilclient_destroy);
  if (!mIlClient) {
    Log.Error(QString("IL client init fail"));
    return false;
  }
  if (OMX_Init() != OMX_ErrorNone) {
    Log.Error(QString("Omx init fail"));
    return false;
  }
  mInit = true;
  ilclient_set_error_callback(mIlClient.data(), IlComponents::IlErrorCallback, nullptr);

  mIlDecoder.reset(new IlComponents());
  mIlTunnels.reset(new IlTunnels());
  if (ilclient_create_component(mIlClient.data(), &mIlDecoder->Decoder(), "video_decode"
                                , (ILCLIENT_CREATE_FLAGS_T)(
                                  ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0) {
    Log.Error(QString("Omx decoder init fail"));
    return false;
  }
  if (ilclient_create_component(mIlClient.data(), &mIlDecoder->Render(), "video_render"
                                , ILCLIENT_DISABLE_ALL_PORTS) != 0) {
    Log.Error(QString("Omx render init fail"));
    return false;
  }

  set_tunnel(mIlTunnels->DecoderToRender(), mIlDecoder->Decoder(), kDecodePortOut, mIlDecoder->Render(), kRenderPortIn);

  if (ilclient_change_component_state(mIlDecoder->Decoder(), OMX_StateIdle) != 0) {
    Log.Error(QString("Omx decoder idle fail"));
    return false;
  }

  OMX_VIDEO_PARAM_PORTFORMATTYPE srcFormat;
  memset(&srcFormat, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
  srcFormat.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
  srcFormat.nVersion.nVersion = OMX_VERSION;
  srcFormat.nPortIndex = kDecodePortIn;
  srcFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;
  srcFormat.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;

  if (OMX_SetParameter(mIlDecoder->DecodeHandle(), OMX_IndexParamVideoPortFormat, &srcFormat) != OMX_ErrorNone) {
    Log.Error(QString("Omx decoder setup source fail"));
    return false;
  }

  if (ilclient_enable_port_buffers(mIlDecoder->Decoder(), kDecodePortIn, 0, 0, 0) != 0) {
    Log.Error(QString("Omx decoder setup in buffer fail"));
    return false;
  }

  if (ilclient_change_component_state(mIlDecoder->Decoder(), OMX_StateExecuting) != 0) {
    Log.Error(QString("Omx decoder execute fail"));
    return false;
  }
  mPause = false;

  mAfterReadSetupDone = false;
  Log.Info("Init omx done");
  return true;
}

bool OmxRender::SetRenderRegion(const QRect& region, const QSize& imgSize)
{
//  if (ilclient_change_component_state(mIlDecoder->Render(), OMX_StateIdle) != 0) {
//    Log.Error(QString("Omx render idle fail"));
//    return false;
//  }

  OMX_CONFIG_DISPLAYREGIONTYPE configDisplay;
  memset(&configDisplay, 0, sizeof(OMX_CONFIG_DISPLAYREGIONTYPE));
  configDisplay.nSize = sizeof(OMX_CONFIG_DISPLAYREGIONTYPE);
  configDisplay.nVersion.nVersion = OMX_VERSION;
  configDisplay.nPortIndex = kRenderPortIn;
  configDisplay.fullscreen = OMX_FALSE;
  configDisplay.noaspect   = OMX_TRUE;
  configDisplay.set = (OMX_DISPLAYSETTYPE)(OMX_DISPLAY_SET_DEST_RECT|OMX_DISPLAY_SET_SRC_RECT|OMX_DISPLAY_SET_FULLSCREEN|OMX_DISPLAY_SET_NOASPECT);
  configDisplay.dest_rect.x_offset  = region.left();
  configDisplay.dest_rect.y_offset  = region.top();
  configDisplay.dest_rect.width     = region.width();
  configDisplay.dest_rect.height    = region.height();
  configDisplay.src_rect.x_offset   = 0;
  configDisplay.src_rect.y_offset   = 0;
  configDisplay.src_rect.width      = imgSize.width();
  configDisplay.src_rect.height     = imgSize.height();

  if (OMX_SetParameter(mIlDecoder->RenderHandle(), OMX_IndexConfigDisplayRegion, &configDisplay) != OMX_ErrorNone) {
    Log.Error(QString("Omx render setup fail"));
    return false;
  }

  if (mAfterReadSetupDone) {
//    if (ilclient_change_component_state(mIlDecoder->Render(), OMX_StateExecuting) != 0) {
//      Log.Error(QString("Omx render execute fail"));
//      return false;
//    }
  }

  return true;
}

bool OmxRender::SetRenderPause(bool paused)
{
  if (mPause == paused) {
    return true;
  }
  if (paused) {
    if (ilclient_change_component_state(mIlDecoder->Render(), OMX_StateIdle) != 0) {
      Log.Error(QString("Omx render idle fail"));
      return false;
    }
  } else {
    if (ilclient_change_component_state(mIlDecoder->Render(), OMX_StateExecuting) != 0) {
      Log.Error(QString("Omx render execute fail"));
      return false;
    }
  }
  mPause = paused;
  return true;
}

bool OmxRender::CloseDecoder()
{
  OMX_BUFFERHEADERTYPE* srcBuffer = ilclient_get_input_buffer(mIlDecoder->Decoder(), kDecodePortIn, 1);

  if (!srcBuffer) {
    Log.Error(QString("Omx decoder get source buffer fail"));
    return false;
  }

  srcBuffer->nFilledLen = 0;
  srcBuffer->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

  if (OMX_EmptyThisBuffer(mIlDecoder->DecodeHandle(), srcBuffer) != OMX_ErrorNone) {
    Log.Warning(QString("Omx decoder eos buffer fail"));
  }
  return true;
}

void OmxRender::DeinitDecoder()
{
  if (mInit) {
    ilclient_disable_tunnel(mIlTunnels->DecoderToRender());
    ilclient_disable_port_buffers(mIlDecoder->Decoder(), kDecodePortIn, 0, 0, 0);
    mIlTunnels.clear();
    mIlDecoder.clear();
    mIlClient.clear();
    OMX_Deinit();
    Log.Info("Deinit omx done");
    mInit = false;
  }
}

bool OmxRender::DecodeIn(char* frameData, int frameSize)
{
  for (int writeBytes = 0; frameSize > 0; frameData += writeBytes, frameSize -= writeBytes) {
    OMX_BUFFERHEADERTYPE* srcBuffer = ilclient_get_input_buffer(mIlDecoder->Decoder(), kDecodePortIn, 1);

    if (!srcBuffer) {
      Log.Error(QString("Omx decoder get source buffer fail"));
      return false;
    }

    if (!mAfterReadSetupDone) {
      if (ilclient_remove_event(mIlDecoder->Decoder(), OMX_EventPortSettingsChanged, kDecodePortOut, 0, 0, 1) == 0) {
        if (ilclient_setup_tunnel(mIlTunnels->DecoderToRender(), 0, 0) != 0) {
          Log.Error(QString("Omx decode -> render tunnel setup fail"));
          return false;
        }

        if (ilclient_change_component_state(mIlDecoder->Render(), OMX_StateExecuting) != 0) {
          Log.Error(QString("Omx render execute fail"));
          return false;
        }
        mAfterReadSetupDone = true;
      }
    }

    if ((int)srcBuffer->nAllocLen < frameSize) {
      writeBytes = srcBuffer->nAllocLen;
    } else {
      writeBytes = frameSize;
      //srcBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
    }
    if (mFirstFrame) {
      srcBuffer->nFlags = OMX_BUFFERFLAG_STARTTIME;
      mFirstFrame = false;
    } else {
      srcBuffer->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
    }
    memcpy(srcBuffer->pBuffer, frameData, writeBytes);
    srcBuffer->nFilledLen = writeBytes;

    LogLive(QString("decode in: %1").arg(writeBytes));
    if (OMX_EmptyThisBuffer(mIlDecoder->DecodeHandle(), srcBuffer) != OMX_ErrorNone) {
      Log.Error(QString("Omx decoder get source buffer fail"));
      return false;
    }
  }

  return true;
}


OmxRender::OmxRender()
  : mInit(false), mFirstFrame(true), mPause(true)
{
}

OmxRender::~OmxRender()
{
  DeinitDecoder();
}

