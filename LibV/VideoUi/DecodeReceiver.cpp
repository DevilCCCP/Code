#include <Lib/Log/Log.h>
#include <LibV/Decoder/Ffmpeg/Convert.h>

#include "DecodeReceiver.h"


ConvertS Converter(new Convert());

DecodeReceiver::DecodeReceiver(bool _UseImage)
  : mUseImage(_UseImage)
  , mPaused(false)
{
}

DecodeReceiver::~DecodeReceiver()
{
}


bool DecodeReceiver::ProcessFrame()
{
  mLastFrame = CurrentVFrame();

  if (!mLastFrame->IsFrame() || mPaused) {
    return false;
  }

  if (mUseImage) {
    const Frame::Header* header = mLastFrame->GetHeader();
    QImage image;
    switch (header->Compression) {
    case eRawY:
      image = ImageFromY(mLastFrame->VideoData(), mLastFrame->VideoDataSize()
                         , mLastFrame->GetHeader()->Width, mLastFrame->GetHeader()->Height);
      break;
    case eRawRgba:
      image = ImageFromRgba(mLastFrame->VideoData(), mLastFrame->VideoDataSize()
                            , mLastFrame->GetHeader()->Width, mLastFrame->GetHeader()->Height);
      break;

    case eRawNv12:
      image = ImageFromNv12(mLastFrame->VideoData(), mLastFrame->VideoDataSize()
                            , mLastFrame->GetHeader()->Width, mLastFrame->GetHeader()->Height);
      break;

    case eRawYuvP:
      image = ImageFromYuv(mLastFrame->VideoData(), mLastFrame->VideoDataSize()
                           , mLastFrame->GetHeader()->Width, mLastFrame->GetHeader()->Height);
      break;

    default:
      LOG_WARNING_ONCE(QString("Draw format is unsupported %1 (%2)").arg(CompressionToString(header->Compression)).arg(header->Compression));
      break;
    }
    if (!image.isNull()) {
      emit OnDecodedImage(image);
    }
  } else {
    emit OnDecoded();
  }

  return false;
}

void DecodeReceiver::SetPause(bool pause)
{
  mPaused = pause;
}

QImage DecodeReceiver::ImageFromY(const char* data, int dataSize, int width, int height)
{
  int stride = dataSize / height;
  QImage image(width, height, QImage::Format_Grayscale8);
  for (int j = 0; j < height; j++) {
    char* line = (char*)image.scanLine(j);
    const char* dataLine = data + j * stride;
    memcpy(line, dataLine, width);
  }
  return image;
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

QImage DecodeReceiver::ImageFromNv12(const char* data, int dataSize, int width, int height)
{
  int stride = dataSize / height * 2/3;
  QImage image(width, height, QImage::Format_RGB32);
  for (int j = 0; j < height; j++) {
    const char* dataLine = data + j * stride;
    char* line = (char*)image.scanLine(j);
    for (int i = 0; i < width; i++) {
      *line = *dataLine; line++;
      *line = *dataLine; line++;
      *line = *dataLine; line++;
      *line = (char)(uchar)255; line++;
      dataLine++;
    }
  }
  return image;
}

QImage DecodeReceiver::ImageFromYuv(const char* data, int dataSize, int width, int height)
{
  int stride = dataSize / height;
  QImage image(width, height, QImage::Format_RGB32);
  for (int j = 0; j < height; j++) {
    const char* dataLine = data + j * stride;
    char* line = (char*)image.scanLine(j);
    for (int i = 0; i < width; i++) {
      *line = *dataLine; line++;
      *line = *dataLine; line++;
      *line = *dataLine; line++;
      *line = (char)(uchar)255; line++;
      dataLine++;
      dataLine++;
    }
  }
  return image;
}
