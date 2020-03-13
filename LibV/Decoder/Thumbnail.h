#pragma once

#include <QDateTime>
#include <QMutex>
#include <QWaitCondition>

#include <Lib/Include/Common.h>


DefineClassS(Thumbnail);
DefineClassS(Frame);

class Thumbnail
{
  qint64          mNextCreate;
  QByteArray      mData;
  QMutex          mMutex;
  QWaitCondition  mWait;

public:
  QMutex& GetMutex() { return mMutex; }
//  QByteArray& GetData() { return mData; }

public:
  bool IsTimeToCreate() { return QDateTime::currentMSecsSinceEpoch() >= mNextCreate; }

  void Create(const char* data, int size);
  void Create(const QByteArray& _Data);
  void Create(const FrameS& frame);
  const QByteArray& GetThumbnail(QMutexLocker* lock);

private:
  bool EncodeJpeg(const FrameS& frame, QByteArray& jpegData);
  void EncodeJpegRgb(const FrameS& frame, QByteArray& jpegData);
  void EncodeJpegY(const FrameS& frame, QByteArray& jpegData);
  void EncodeJpegY2(const FrameS& frame, QByteArray& jpegData);

public:
  Thumbnail();
};

