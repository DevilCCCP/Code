#include <Lib/Log/Log.h>
#include <LibV/Decoder/Omx/IlDef.h>

#include "OmxSound.h"


const int kFrequency = 44100;
const int kChannels = 2;
const int kBitsPerSample = 32;
const int kBufferCount = 4;

//const char* kAudioOutput = "hdmi";
const char* kAudioOutput = "local";

bool OmxSound::InitSound(int bufferSize)
{
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

  mIlSound.reset(new IlComponents());
  if (ilclient_create_component(mIlClient.data(), &mIlSound->Sound(), "audio_render"
                                , (ILCLIENT_CREATE_FLAGS_T)((int)ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_DISABLE_ALL_PORTS)) != 0) {
    Log.Error(QString("Omx sound init fail"));
    return false;
  }

  OMX_PARAM_PORTDEFINITIONTYPE pp;
  memset(&pp, 0, sizeof(pp));
  pp.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  pp.nVersion.nVersion = OMX_VERSION;
  pp.nPortIndex = kSoundPortIn;

  if (OMX_GetParameter(mIlSound->SoundHandle(), OMX_IndexParamPortDefinition, &pp) != OMX_ErrorNone) {
    Log.Error(QString("Omx sound setup read fail"));
    return false;
  }

  pp.nBufferSize = bufferSize;
  pp.nBufferCountActual = kBufferCount;

  if (OMX_SetParameter(mIlSound->SoundHandle(), OMX_IndexParamPortDefinition, &pp) != OMX_ErrorNone) {
    Log.Error(QString("Omx sound setup write fail"));
    return false;
  }

  OMX_AUDIO_PARAM_PCMMODETYPE pcm;
  memset(&pcm, 0, sizeof(pcm));
  pcm.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
  pcm.nVersion.nVersion = OMX_VERSION;
  pcm.nPortIndex = kSoundPortIn;
  pcm.nChannels = kChannels;
  pcm.eNumData = OMX_NumericalDataSigned;
  pcm.eEndian = OMX_EndianLittle;
  pcm.nSamplingRate = kFrequency;
  pcm.bInterleaved = OMX_TRUE;
  pcm.nBitPerSample = kBitsPerSample;
  pcm.ePCMMode = OMX_AUDIO_PCMModeLinear;

  switch(kChannels) {
  case 1:
     pcm.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
     break;
  case 3:
     pcm.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
     pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
     pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
     break;
  case 8:
     pcm.eChannelMapping[7] = OMX_AUDIO_ChannelRS;
  case 7:
     pcm.eChannelMapping[6] = OMX_AUDIO_ChannelLS;
  case 6:
     pcm.eChannelMapping[5] = OMX_AUDIO_ChannelRR;
  case 5:
     pcm.eChannelMapping[4] = OMX_AUDIO_ChannelLR;
  case 4:
     pcm.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
     pcm.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
  case 2:
     pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
     pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
     break;
  }

  if (OMX_SetParameter(mIlSound->SoundHandle(), OMX_IndexParamAudioPcm, &pcm) != OMX_ErrorNone) {
    Log.Error(QString("Omx sound setup write fail"));
    return false;
  }

  if (ilclient_change_component_state(mIlSound->Sound(), OMX_StateIdle) != 0) {
    Log.Error(QString("Omx sound idle fail"));
    return false;
  }

  if (ilclient_enable_port_buffers(mIlSound->Sound(), kSoundPortIn, NULL, NULL, NULL) != 0) {
    Log.Error(QString("Omx sound setup in buffer fail"));
    return false;
  }

  if (ilclient_change_component_state(mIlSound->Sound(), OMX_StateExecuting) != 0) {
    Log.Error(QString("Omx sound execute fail"));
    return false;
  }

  OMX_CONFIG_BRCMAUDIODESTINATIONTYPE ad;
  memset(&ad, 0, sizeof(ad));
  ad.nSize = sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE);
  ad.nVersion.nVersion = OMX_VERSION;
  strcpy((char *)ad.sName, kAudioOutput);

  if (OMX_SetConfig(mIlSound->SoundHandle(), OMX_IndexConfigBrcmAudioDestination, &ad) != OMX_ErrorNone) {
      Log.Error(QString("Omx sound setup write fail"));
      return false;
  }

  return true;
}

void OmxSound::CloseSound()
{
  ilclient_change_component_state(mIlSound->Sound(), OMX_StateIdle);
  OMX_SendCommand(mIlSound->SoundHandle(), OMX_CommandStateSet, OMX_StateLoaded, NULL);
}

bool OmxSound::PlayFrame(const char* audioData, int audioDataSize)
{
  while (audioDataSize > 0) {
    OMX_BUFFERHEADERTYPE* srcBuffer = ilclient_get_input_buffer(mIlSound->Sound(), kSoundPortIn, 1);

    if (!srcBuffer) {
      Log.Error(QString("Omx sound get source buffer fail (too fast feeding)"));
      return false;
    }

    int writeBytes = 0;
    if ((int)srcBuffer->nAllocLen < audioDataSize) {
      writeBytes = (int)srcBuffer->nAllocLen;
    } else {
      writeBytes = audioDataSize;
    }
//    srcBuffer->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
    memcpy(srcBuffer->pBuffer, audioData, writeBytes);
    srcBuffer->pAppPrivate = nullptr;
    srcBuffer->nOffset = 0;
    srcBuffer->nFilledLen = writeBytes;

    if (OMX_EmptyThisBuffer(mIlSound->SoundHandle(), srcBuffer) != OMX_ErrorNone) {
      Log.Error(QString("Omx sound get source buffer fail"));
      return false;
    }
    audioData += writeBytes;
    audioDataSize -= writeBytes;
  }

  return true;
}

void OmxSound::ResetFrames()
{
  mAudioStack.clear();
}


OmxSound::OmxSound()
  : mInit(false)
{
}

