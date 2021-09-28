#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/videodev2.h>

#include <Lib/Log/Log.h>
#include <LibV/Decoder/Thumbnail.h>

#include "V4lIn.h"
#include "../Linux/FileDescriptor.h"
#include "../Linux/MmapBuffer.h"


const int kMmapBufferCount = 4;

static void FileDescriptorDtor(FileDescriptor *pfd)
{
  close(*pfd);
}

bool V4lIn::Open(const QString &filename, const QString& resolution, const QString& fps)
{
  QStringList wh = resolution.split('x');
  if (wh.size() == 2) {
    mWidth  = wh.at(0).toInt();
    mHeight = wh.at(1).toInt();
  } else {
    mWidth = mHeight = 0;
  }
  int fpsValue = fps.toInt();

  int fd = open(filename.toUtf8().constData(), O_RDWR);
  if (fd == -1) {
    return OpenError(QString("Open file fail (file: '%1', errno: %2)").arg(filename).arg(errno));
  }

  FileDescriptorS fileDescriptor(new FileDescriptor(fd), FileDescriptorDtor);
  v4l2_capability cap;
  memset(&cap, 0, sizeof(cap));
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
    return OpenError(QString("Open file fail (file: '%1', errno: %2)").arg(filename).arg(errno));
  }

  if (!mOpenInfo) {
    Log.Info(QString("Opened device (card: '%1', driver: '%2', v: %3, cap: %4, dcap: %5)")
             .arg((const char*)cap.card).arg((const char*)cap.driver)
             .arg(cap.version, 0, 16).arg(cap.capabilities, 0, 16).arg(cap.device_caps, 0, 16));
    mOpenInfo = true;
  }

  int capability = (cap.capabilities & V4L2_CAP_DEVICE_CAPS)? cap.device_caps: cap.capabilities;
  if ((capability & V4L2_CAP_VIDEO_CAPTURE) == 0) {
    return OpenError(QString("Device has no capture video capability"));
  }

  v4l2_input input;
  memset(&input, 0, sizeof(input));
  input.index = 0;
  if (ioctl(fd, VIDIOC_ENUMINPUT, &input) < 0) {
    return OpenError(QString("Device has no video input"));
  }

  if (ioctl(fd, VIDIOC_S_INPUT, &input.index) < 0) {
    return OpenError(QString("Select video input fail"));
  }

  v4l2_format fmt;
  memset(&fmt, 0, sizeof (fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
    return OpenError(QString("Can't read video format"));
  }
  int pixelFormat = fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  int width  = fmt.fmt.pix.width;
  int height = fmt.fmt.pix.height;
  if (mWidth > 0 && mHeight > 0) {
    fmt.fmt.pix.width  = mWidth;
    fmt.fmt.pix.height = mHeight;
  } else {
    mWidth  = fmt.fmt.pix.width;
    mHeight = fmt.fmt.pix.height;
  }

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
    switch (pixelFormat) {
    case V4L2_PIX_FMT_NV12:   mCompression = eRawNv12; break;
    case V4L2_PIX_FMT_YUV420: mCompression = eRawYuv; break;
    case V4L2_PIX_FMT_YUYV:   mCompression = eRawYuvP; break;
    default: return OpenError(QString("Got unsupported pixel format ('%1')").arg(QByteArray((char*)&pixelFormat, 4).constData()));
    }

    mWidth  = width;
    mHeight = height;
    Log.Warning(QString("Select video format fail, using (w: %1, h: %2, f: '%3')").arg(mWidth).arg(mHeight)
                .arg(QByteArray((char*)&pixelFormat, 4).constData()));
  } else {
    int pixelFormat2 = fmt.fmt.pix.pixelformat;
    switch (pixelFormat2) {
    case V4L2_PIX_FMT_NV12:   mCompression = eRawNv12; break;
    case V4L2_PIX_FMT_YUV420: mCompression = eRawYuv; break;
    case V4L2_PIX_FMT_YUYV:   mCompression = eRawYuvP; break;
    case V4L2_PIX_FMT_MJPEG:  mCompression = eJpeg; break;
    default: return OpenError(QString("Got unsupported pixel format ('%1')").arg(QByteArray((char*)&pixelFormat2, 4).constData()));
    }

    Log.Info(QString("Set video format to %1 (w: %2, h: %3)").arg(CompressionToString(mCompression)).arg(mWidth).arg(mHeight));
  }

  v4l2_streamparm parm;
  memset(&parm, 0, sizeof (parm));
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(fd, VIDIOC_G_PARM, parm) < 0) {
    memset(&parm, 0, sizeof (parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  }
  parm.parm.capture.capturemode = 0;
  parm.parm.capture.extendedmode = 0;
  if (fpsValue > 0) {
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = fpsValue;
  }
  if (ioctl(fd, VIDIOC_S_PARM, parm) < 0) {
    Log.Warning(QString("Can't change stream params"));
  }

  if ((capability & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) {
    if (!OpenMmap(fileDescriptor)) {
      return false;
    }
  } else {
    return OpenError("No implemented streaming capabilities");
  }

  mFileDescriptor.swap(fileDescriptor);
  mOpenWarning = false;
  mAbort = false;
  return true;
}

bool V4lIn::ReadNext(FrameS& frame)
{
  switch (mIoMethod) {
  case eIoNone: break;
  case eMmap:   return ReadNextMmap(frame);
  }
  return false;
}

void V4lIn::Close()
{
  switch (mIoMethod) {
  case eIoNone: break;
  case eMmap:   CloseMmap(); break;
  }
  mFileDescriptor.clear();
}

void V4lIn::Abort()
{
  mAbort = true;
}

bool V4lIn::OpenError(const QString& warning)
{
  if (!mOpenWarning) {
    Log.Warning(warning);
    mOpenWarning = true;
  }
  return false;
}

bool V4lIn::OpenMmap(const FileDescriptorS& fileDescriptor)
{
  v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count  = kMmapBufferCount;
  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (ioctl(fileDescriptor->Descriptor(), VIDIOC_REQBUFS, &req) < 0) {
    return OpenError("Request mmap streaming fail");
  }

  if (req.count < 2) {
    return OpenError("Request mmap streaming not enough buffers");
  }

  for (int i = 0; i < (int)req.count; i++) {
    v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (ioctl(fileDescriptor->Descriptor(), VIDIOC_QUERYBUF, &buf) < 0) {
      return OpenError("Request mmap buffer fail");
    }

    mMmapBuffers.append(MmapBufferS(new MmapBuffer(fileDescriptor, buf.length, buf.m.offset)));

    if (ioctl(fileDescriptor->Descriptor(), VIDIOC_QBUF, &buf) < 0) {
      return OpenError("Request mmap buffer 2 fail");
    }
  }

  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(fileDescriptor->Descriptor(), VIDIOC_STREAMON, &type) < 0) {
    return OpenError("Start mmap streaming fail");
  }

  mIoMethod = eMmap;
  return true;
}

void V4lIn::CloseMmap()
{
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(mFileDescriptor->Descriptor(), VIDIOC_STREAMOFF, &type) < 0) {
    Log.Warning("Close mmap streaming fail");
  }

  mMmapBuffers.clear();
}

bool V4lIn::ReadNextMmap(FrameS& frame)
{
  pollfd pfd;
  pfd.fd     = mFileDescriptor->Descriptor();
  pfd.events = POLLIN;

  while (!mAbort) {
    if (poll(&pfd, 1, 1000) < 0) {
      if (errno != EINTR) {
        LOG_WARNING_ONCE(QString("V4l poll fail (code: %1)").arg(errno));
      }
      continue;
    }

    if (pfd.revents) {
      v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;

      if (ioctl(mFileDescriptor->Descriptor(), VIDIOC_DQBUF, &buf) < 0) {
        if (errno != EAGAIN) {
          LOG_WARNING_ONCE(QString("V4l wait mmap fail (code: %1)").arg(errno));
        }
        continue;
      }

      Frame::Header* header = InitFrame(frame, buf.bytesused);
      header->Compression = mCompression;
      header->Width       = mWidth;
      header->Height      = mHeight;
      header->Timestamp   = QDateTime::currentMSecsSinceEpoch();

      header->VideoDataSize = buf.bytesused;
      memcpy(frame->VideoData(), mMmapBuffers[buf.index]->data(), buf.bytesused);
      Log.Trace(QString("sz: %1").arg(buf.bytesused));

      if (ioctl(mFileDescriptor->Descriptor(), VIDIOC_QBUF, &buf) < 0) {
        LOG_WARNING_ONCE(QString("V4l release mmap fail (code: %1)").arg(errno));
        return false;
      }
      CreateThumbnail(frame);
      return true;
    }
  }
  return false;
}

Frame::Header* V4lIn::InitFrame(FrameS& frame, int fullSize)
{
  frame = FrameS(new Frame());
  frame->ReserveData(fullSize);
  Frame::Header* header = frame->InitHeader();
  header->Key  = true;
  header->Size = sizeof(Frame::Header) + fullSize;
  return header;
}

void V4lIn::CreateThumbnail(const FrameS& frame)
{
  if (mThumbnail && mThumbnail->IsTimeToCreate()) {
    mThumbnail->Create(frame);
  }
}


V4lIn::V4lIn(Source *_Source, const ThumbnailS& _Thumbnail)
  : mSource(_Source), mOpenInfo(false), mOpenWarning(false)
  , mCompression(eRawNv12), mWidth(0), mHeight(0)
  , mAbort(true), mIoMethod(eIoNone)
  , mThumbnail(_Thumbnail)
{
}

V4lIn::~V4lIn()
{
}
