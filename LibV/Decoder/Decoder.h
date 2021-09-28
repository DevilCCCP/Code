#pragma once

#include <Lib/Dispatcher/Conveyor.h>
#include <LibV/Include/ConveyorV.h>
#include <LibV/Include/Frame.h>


DefineClassS(Decoder);
DefineClassS(CodecA);
DefineClassS(Thumbnail);

class Decoder: public ConveyorV
{
  PROPERTY_GET_SET(ECompression, DestCompression)
  PROPERTY_GET_SET(bool        , UseAudio)
  PROPERTY_GET_SET(int         , Fps)
  PROPERTY_GET_SET(bool        , UseHardware)

  CodecAS        mCodecVideo;
  CodecAS        mCodecAudio;
  ThumbnailS     mThumbnail;

public:
  const ThumbnailS& GetThumbnail() { return mThumbnail; }

public:
  /*override */virtual const char* Name() override { return "Decoder"; }
  /*override */virtual const char* ShortName() override { return "D"; }
protected:
//  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

protected:
  /*override */virtual bool ProcessFrame() override;

private:
  void InitCodecVideo();
  void InitCodecAudio();

  void RetriveDecodedFrames();
  void RetriveDecodedFramesForCodec(const CodecAS& codec);

public:
  Decoder(bool _UseThumbnail, bool _UseAudio, ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  /*override */virtual ~Decoder();
};

