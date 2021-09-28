#pragma once

#include <QImage>
#include <atomic>

#include <LibV/Include/ConveyorV.h>


class DecodeReceiver: public ConveyorV
{
  PROPERTY_GET_SET(bool, UseImage)

  FrameS                 mLastFrame;
  std::atomic<bool>      mPaused;

  Q_OBJECT

public:
  explicit DecodeReceiver(bool _UseImage = false);
  ~DecodeReceiver();

public:
  FrameS LastFrame() { return mLastFrame; }

public:
  /*override */virtual const char* Name() override { return "DecodeReceiver"; }
  /*override */virtual const char* ShortName() override { return "Dr"; }

protected:
  /*override */virtual bool ProcessFrame() override;

public:
  void SetPause(bool pause);

private:
  QImage ImageFromY(const char* data, int dataSize, int width, int height);
  QImage ImageFromRgba(const char* data, int dataSize, int width, int height);
  QImage ImageFromNv12(const char* data, int dataSize, int width, int height);
  QImage ImageFromYuv(const char* data, int dataSize, int width, int height);

signals:
  void OnDecoded();
  void OnDecodedImage(QImage image);

public slots:
};

