#include <QImage>
#include <QImageWriter>
#include <QBuffer>

#include <Lib/Log/Log.h>

#include "OmxDec.h"
#include "IlDef.h"
#include "../Thumbnail.h"


#define LogLive(X)
//#define LogLive Log.Info

bool OmxDec::InitDecoder()
{
  mFirstFrame = true;
  mOutReady = false;

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
  if (ilclient_create_component(mIlClient.data(), &mIlDecoder->Decoder(), "video_decode"
                                , (ILCLIENT_CREATE_FLAGS_T)(
                                  ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS
                                  | ILCLIENT_ENABLE_OUTPUT_BUFFERS)) != 0) {
    Log.Error(QString("Omx decoder init fail"));
    return false;
  }

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

  Log.Info("Init omx done");
  return true;
}

bool OmxDec::CloseDecoder()
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

void OmxDec::DeinitDecoder()
{
  if (mInit) {
    ilclient_disable_port_buffers(mIlDecoder->Decoder(), kDecodePortIn, 0, 0, 0);
    mIlDecoder.clear();
    mIlClient.clear();
    OMX_Deinit();
    Log.Info("Deinit omx done");
    mInit = false;
  }
}

bool OmxDec::DecodeIn(char* frameData, int frameSize)
{
  for (int writeBytes = 0; frameSize > 0; frameData += writeBytes, frameSize -= writeBytes) {
    OMX_BUFFERHEADERTYPE* srcBuffer = ilclient_get_input_buffer(mIlDecoder->Decoder(), kDecodePortIn, 1);

    if (!srcBuffer) {
      Log.Error(QString("Omx decoder get source buffer fail"));
      return false;
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

bool OmxDec::DecodeOut(bool canSkip, FrameS& decodedFrame)
{
  if (!mOutReady) {
    if (ilclient_wait_for_event(mIlDecoder->Decoder(), OMX_EventPortSettingsChanged, kDecodePortOut, 0, 0, 1
                                      , ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10) != 0) {
      return false;
    }
    if (!GetOutputFormat()) {
      return false;
    }
    if (ilclient_enable_port_buffers(mIlDecoder->Decoder(), kDecodePortOut, NULL, NULL, NULL) != 0) {
      Log.Error(QString("Omx decoder setup out buffer fail"));
      return false;
    }
    mOutReady = true;
  }

  int fullSize = 0;
  decodedFrame.clear();
  while (OMX_BUFFERHEADERTYPE* dstBuffer = ilclient_get_output_buffer(mIlDecoder->Decoder(), kDecodePortOut, 0)) {
    LogLive(QString("decode out: %1").arg(dstBuffer->nFilledLen));
    if (dstBuffer->nFilledLen) {
      if (!decodedFrame) {
        decodedFrame.reset(new Frame());
        decodedFrame->ReserveData((!canSkip)? mFrameSize: 0);
        Frame::Header* header = decodedFrame->InitHeader();
        header->Compression = mOutCompression;
        header->Width = mWidth;
        header->Height = mHeight;
        header->VideoDataSize = (!canSkip)? mFrameSize: 0;
      }
      int buffSize = dstBuffer->nFilledLen;
      int fullFrameSize = fullSize + buffSize;
      if (fullFrameSize > mFrameSize) {
        buffSize = mFrameSize - fullSize;
      }
      if (!canSkip) {
        memcpy(decodedFrame->VideoData() + fullSize, (char*)dstBuffer->pBuffer, buffSize);
      }
      fullSize += buffSize;
//        char* srcMem = (char*)dstBuffer->pBuffer;
//        for (int j = fullSize / mWidth; buffSize > 0; j++) {
//          int i = fullSize % mWidth;
//          int usedSize = mWidth - i;
//          memcpy(decodedFrame->VideoData() + fullSize, srcMem, usedSize);
//          fullSize += usedSize;
//        }
    }
    if (OMX_FillThisBuffer(mIlDecoder->DecodeHandle(), dstBuffer) != OMX_ErrorNone) {
      Log.Error(QString("Omx decoder get dest buffer fail"));
      return false;
    }
    if (fullSize >= mFrameSize) {
      if (!canSkip) {
        if (mThumbnail && mThumbnail->IsTimeToCreate()) {
          mThumbnail->Create(decodedFrame);
        }
      }
      break;
    }
  }

  return decodedFrame;
}

bool OmxDec::GetOutputFormat()
{
  OMX_PARAM_PORTDEFINITIONTYPE outFormat;

  outFormat.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  outFormat.nVersion.nVersion = OMX_VERSION;
  outFormat.nPortIndex = kDecodePortOut;
  if (OMX_GetParameter(mIlDecoder->DecodeHandle(), OMX_IndexParamPortDefinition, &outFormat) != OMX_ErrorNone) {
    Log.Error(QString("Omx: get output format fail"));
    return false;
  }

  mWidth       = (int)outFormat.format.video.nFrameWidth;
  mHeight      = (int)outFormat.format.video.nFrameHeight;
  mStride      = (int)outFormat.format.video.nStride;
  mSliceHeight = (int)outFormat.format.video.nSliceHeight;
  int colorFormat = (int)outFormat.format.video.eColorFormat;

  Log.Info(QString("Got output parameters: %1x%2(%3x%4), fmt: %5")
           .arg(mWidth).arg(mHeight).arg(mStride).arg(mSliceHeight).arg(colorFormat));

  if (mWidth <= 0 || mHeight <= 0 || mStride < mWidth || mSliceHeight < mHeight) {
    Log.Error(QString("Omx: output parameters not valid"));
    return false;
  }

  switch (colorFormat) {
  case OMX_COLOR_FormatYUV420PackedPlanar: mOutCompression = eRawYuv; mFrameSize = mWidth * mHeight * 3 / 2; break;
  default:
    Log.Error(QString("Omx: output format not supported"));
    return false;
  }

  return true;
}


OmxDec::OmxDec(const ThumbnailS& _Thumbnail)
  : mInit(false), mFirstFrame(true), mOutReady(false)
  , mThumbnail(_Thumbnail)
{
}

OmxDec::~OmxDec()
{
  DeinitDecoder();
}

