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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Decoder"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "D"; }
protected:
//  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;

private:
  void InitCodecVideo();
  void InitCodecAudio();

  void RetriveDecodedFrames();
  void RetriveDecodedFramesForCodec(const CodecAS& codec);

public:
  Decoder(bool _UseThumbnail, bool _UseAudio, ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  /*override */virtual ~Decoder();
};

