#include <QMutexLocker>
#include <QImage>
#include <QImageWriter>
#include <QBuffer>

#include <Lib/Log/Log.h>
#include <LibV/Include/Frame.h>

#include "Thumbnail.h"


const qint64 kMaxValidPeriodMs = 5*60*1000;
const qint64 kMinValidPeriodMs = 13*1000;

void Thumbnail::Create(const char *data, int size)
{
  Create(QByteArray(data, size));
}

void Thumbnail::Create(const QByteArray &_Data)
{
  mNextCreate = QDateTime::currentMSecsSinceEpoch() + kMaxValidPeriodMs;
  QMutexLocker lock(&mMutex);
  mData = _Data;
  mWait.wakeAll();
  Log.Trace("Create new Thumbnail");
}

void Thumbnail::Create(const FrameS& frame)
{
  const Frame::Header* header = frame->GetHeader();
  if ((header->Compression & eTypeMask) == eRawVideo) {
    QByteArray jpegData;
    if (EncodeJpeg(frame, jpegData)) {
      Create(jpegData);
    } else {
      Create(QByteArray());
    }
  } else if (header->Compression == eJpeg) {
    Create(QByteArray((const char*)frame->VideoData(), frame->VideoDataSize()));
  } else {
    Create(QByteArray());
  }
}

const QByteArray &Thumbnail::GetThumbnail(QMutexLocker *lock)
{
  if (mNextCreate < QDateTime::currentMSecsSinceEpoch() + kMaxValidPeriodMs - kMinValidPeriodMs) {
    mNextCreate = QDateTime::currentMSecsSinceEpoch();
    mWait.wait(lock->mutex(), 1000);
  }

  return mData;
}

bool Thumbnail::EncodeJpeg(const FrameS& frame, QByteArray& jpegData)
{
  const Frame::Header* header = frame->GetHeader();
  int stride = (3 * header->Width + 3) & 0xfffffffc;
  QByteArray dataRgb;

  switch (header->Compression) {
  case eRawRgb:
    dataRgb.resize(stride * header->Height + 4);
    for (int j = 0; j < header->Height; j++) {
      const char* src = frame->VideoData() + j * stride;
      char* dest = dataRgb.data() + j * stride;
      for (int i = 0; i < header->Width; i++) {
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
      }
    }
    break;

  case eRawRgba:
    dataRgb.resize(stride * header->Height + 4);
    for (int j = 0; j < header->Height; j++) {
      const char* src = frame->VideoData() + 4 * j;
      char* dest = dataRgb.data() + j * stride;
      for (int i = 0; i < header->Width; i++) {
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        src++;
      }
    }
    break;

  case eRawYuv:
  case eRawNv12:
  case eRawNv12A:
    dataRgb.resize(stride * header->Height + 4);
    for (int j = 0; j < header->Height; j++) {
      const char* src = frame->VideoData() + j * header->Width;
      char* dest = dataRgb.data() + j * stride;
      for (int i = 0; i < header->Width; i++) {
        *dest++ = *src;
        *dest++ = *src;
        *dest++ = *src++;
      }
    }
    break;

  case eRawYuvP:
    dataRgb.resize(stride * header->Height + 4);
    for (int j = 0; j < header->Height; j++) {
      const char* src = frame->VideoData() + j * 2 * header->Width;
      char* dest = dataRgb.data() + j * stride;
      for (int i = 0; i < header->Width; i++) {
        *dest++ = *src;
        *dest++ = *src;
        *dest++ = *src;
        src += 2;
      }
    }
    break;

  default:
    return false;
  }

  QImage image((const uchar*)dataRgb.constData(), header->Width, header->Height, QImage::Format_RGB888);
  QBuffer jpegBuffer(&jpegData);
  QImageWriter imageWriter(&jpegBuffer, QByteArray("jpg"));
  imageWriter.setQuality(90);
  imageWriter.write(image);
  return true;
}


Thumbnail::Thumbnail()
  : mNextCreate(0)
{
}
