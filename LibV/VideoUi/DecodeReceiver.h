#pragma once

#include <QImage>

#include <LibV/Include/ConveyorV.h>


class DecodeReceiver: public ConveyorV
{
  PROPERTY_GET_SET(bool, UseImage)

  FrameS  mLastFrame;

  Q_OBJECT

public:
  explicit DecodeReceiver(bool _UseImage = false);
  ~DecodeReceiver();

public:
  FrameS LastFrame() { return mLastFrame; }

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "DecodeReceiver"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "Dr"; }

protected:
  /*override */virtual bool ProcessFrame() Q_DECL_OVERRIDE;

private:
  QImage ImageFromRgba(const char* data, int dataSize, int width, int height);

signals:
  void OnDecoded();
  void OnDecodedImage(QImage image);

public slots:
};

