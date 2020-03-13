#include <Lib/Log/Log.h>

#include "DecodeReceiver.h"


DecodeReceiver::DecodeReceiver(bool _UseImage)
  : mUseImage(_UseImage)
{
}

DecodeReceiver::~DecodeReceiver()
{
}


bool DecodeReceiver::ProcessFrame()
{
  mLastFrame = CurrentVFrame();

  if (!mLastFrame->IsFrame()) {
    return false;
  }

  if (mUseImage) {
    QImage image = ImageFromRgba(mLastFrame->VideoData(), mLastFrame->VideoDataSize(), mLastFrame->GetHeader()->Width, mLastFrame->GetHeader()->Height);
    if (!image.isNull()) {
      emit OnDecodedImage(image);
    }
  } else {
    emit OnDecoded();
  }

  return false;
}

QImage DecodeReceiver::ImageFromRgba(const char* data, int dataSize, int width, int height)
{
  int stride = dataSize / height;
  QImage image(width, height, QImage::Format_RGB32);
  for (int j = 0; j < height; j++) {
    char* line = (char*)image.scanLine(j);
    const char* dataLine = data + j * stride;
    memcpy(line, dataLine, width * 4);
  }
  return image;
}

