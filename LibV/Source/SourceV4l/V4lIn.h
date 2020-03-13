#pragma once

#include <QVector>

#include <LibV/Include/Frame.h>

#include "../Source.h"


DefineClassS(V4lIn);
DefineClassS(FileDescriptor);
DefineClassS(MmapBuffer);
DefineClassS(Thumbnail);

class V4lIn
{
  Source*              mSource;
  FileDescriptorS      mFileDescriptor;
  bool                 mOpenInfo;
  bool                 mOpenWarning;

  ECompression         mCompression;
  int                  mWidth;
  int                  mHeight;

  enum IoMethod {
    eIoNone,
    eMmap,
  };

  volatile bool        mAbort;
  IoMethod             mIoMethod;
  QVector<MmapBufferS> mMmapBuffers;

  ThumbnailS           mThumbnail;

public:
  bool Open(const QString& filename, const QString& resolution, const QString& fps);
  bool ReadNext(FrameS& frame);
  void Close();
  void Abort();

private:
  inline bool OpenError(const QString& warning);
  bool OpenMmap(const FileDescriptorS& fileDescriptor);
  void CloseMmap();
  bool ReadNextMmap(FrameS& frame);

  Frame::Header* InitFrame(FrameS& frame, int fullSize);
  void CreateThumbnail(const FrameS& frame);

public:
  V4lIn(Source* _Source, const ThumbnailS& _Thumbnail);
  ~V4lIn();
};

