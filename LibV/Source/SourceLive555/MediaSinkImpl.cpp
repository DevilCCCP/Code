#include <QDateTime>
#include <QList>
#include <QByteArray>

#include <Lib/Log/Log.h>
#include <LibV/MediaServer/H264/H264Sprop.h>

#include "MediaSinkImpl.h"
#include "SourceLive.h"


static const char kStartCode3[] = { 0x00, 0x00, 0x01 };
static const char kStartCode4[] = { 0x00, 0x00, 0x00, 0x01 };
const bool kUse3ByteStartCode = false;

void CallbackGetNextFrame(void* client, unsigned frameSize, unsigned truncatedBytesCount, struct timeval ts, unsigned)
{
  if (truncatedBytesCount) {
    LOG_WARNING_ONCE(QString("live555 mrdia sink truncatedBytesCount = %1").arg(truncatedBytesCount));
  }
  auto sink = static_cast<MediaSinkImpl*>(client);
  sink->OnFrame(frameSize, ts.tv_sec * 1000LL + ts.tv_usec / 1000LL);
}

Boolean MediaSinkImpl::continuePlaying()
{
  if (fSource) {
    mFrameBuffer.resize(mMaxFrameSize);
    fSource->getNextFrame((unsigned char*)mFrameBuffer.data() + mMaxHeaderSize, mMaxFrameSize - mMaxHeaderSize
                          , CallbackGetNextFrame, this, onSourceClosure, this);
    return true;
  } else {
    return false;
  }
}

void MediaSinkImpl::OnFrame(int frameSize, const qint64& ts)
{
  //Log.Trace(QString("new frame (compr: 0x%1, ts: %2, size: %3)").arg(mCompression, 0, 16).arg(ts).arg(frameSize));
  char* frameData = mFrameBuffer.data() + mMaxHeaderSize;
  bool key = true;
  if (mCompression == eH264) {
    if (!ModifyH264Frame(frameData, frameSize, key)) {
      frameSize = 0;
      key = false;
    }
  } else if (mCompression == eJpeg) {
    key = true;
  } else if (mCompression == eAac16b) {
    //char* frameData = mNextFrame->AudioData();
    //if (ModifyAacFrame(frameData, frameSize)) {
    //  mNextFrame->SetDisp(mNextFrame->GetDisp() + (frameData - mNextFrame->AudioData()));
    //}
    key = false;
  } else {
    LOG_WARNING_ONCE("live555: Frame type not defined for set key");
  }

  if (!mBaseTimestamp) {
    mBaseTimestamp = QDateTime::currentMSecsSinceEpoch() - ts;
  }

  mNextFrame = FrameS(new Frame());
  mNextFrame->ReserveData(frameSize);
  mNextFrame->GetHeader()->Key = key;

  mNextFrame->GetHeader()->Size = sizeof(Frame::Header) + frameSize;
  mNextFrame->GetHeader()->HeaderSize = sizeof(Frame::Header);
  mNextFrame->GetHeader()->Timestamp = ts + mBaseTimestamp;
  mNextFrame->GetHeader()->Width = mCurrentFrameSize.width();
  mNextFrame->GetHeader()->Height = mCurrentFrameSize.height();

//  static qint64 gBaseTs = mNextFrame->GetHeader()->Timestamp;
//  Log.Trace(QString("ts: %1").arg(mNextFrame->GetHeader()->Timestamp - gBaseTs));

  if (mCompression & eAnyAudio) {
    mNextFrame->GetHeader()->Compression = eCmprNone;
    mNextFrame->GetHeader()->CompressionAudio = mCompression;
    mNextFrame->GetHeader()->VideoDataSize = 0;
    mNextFrame->GetHeader()->AudioDataSize = frameSize;
    memcpy(mNextFrame->AudioData(), frameData, frameSize);
  } else {
    mNextFrame->GetHeader()->Compression = mCompression;
    mNextFrame->GetHeader()->CompressionAudio = eCmprNone;
    mNextFrame->GetHeader()->VideoDataSize = frameSize;
    mNextFrame->GetHeader()->AudioDataSize = 0;
    memcpy(mNextFrame->VideoData(), frameData, frameSize);
  }
  mNextFrame->GetHeader()->ObjectDataSize = 0;

  mSource->OnFrame(mNextFrame);

  if (!continuePlaying()) {
    onSourceClosure(this);
  }
}

bool MediaSinkImpl::ModifyH264Frame(char* &frameData, int &frameSize, bool &key)
{
  int headerDisp = 0;
  if (frameSize < (int)sizeof(kStartCode4) || memcmp(kStartCode4, frameData, sizeof(kStartCode4)) != 0)
  {
    if (frameSize < (int)sizeof(kStartCode3) || memcmp(kStartCode3, frameData, sizeof(kStartCode3)) != 0)
    {
      if (kUse3ByteStartCode)
      {
        headerDisp = sizeof(kStartCode3);

        frameSize += headerDisp;
        frameData -= headerDisp;
        memcpy(frameData, kStartCode3, headerDisp);
      }
      else
      {
        headerDisp = sizeof(kStartCode4);

        frameSize += headerDisp;
        frameData -= headerDisp;
        memcpy(frameData, kStartCode4, headerDisp);
      }
    }
  }

  H264Data pd;
  ParseH264Frame(frameData, frameSize, pd);

  if (mFrameSize.width() == 0) {
    if (!pd.sps.is) {
      return false;
    }

    H264Sprop sprop;
    sprop.Parse(frameData + pd.sps.pos, frameSize - pd.sps.pos);
    mCurrentFrameSize = QSize(sprop.Width(), sprop.Height());
  } else {
    mCurrentFrameSize = mFrameSize;
  }

  if (!pd.data)
  {
    if (!mSprop)
    {
      if (pd.sps.is || pd.pps.is)
      {
        if (pd.sps.is && !mFrameSPS) {
          mExHeader.append(kStartCode3, sizeof(kStartCode3));
          mExHeader.append(frameData + pd.sps.pos, pd.sps.len);
          mFrameSPS = true;
        }
        if (pd.pps.is && !mFramePPS) {
          mExHeader.append(kStartCode3, sizeof(kStartCode3));
          mExHeader.append(frameData + pd.pps.pos, pd.pps.len);
          mFramePPS = true;
        }
      }
    }

    return false;
  }

  if (!mHasKey) {
    if (!pd.key) {
      key = false;
      return false;
    }

    mHasKey = true;

    // для Axis создаем заголовок
    if (mSprop && !pd.sps.is) {
      QList<QByteArray> parts = QByteArray(mSprop).split(',');
      mExHeader.resize(0);
      for (auto itr = parts.begin(); itr != parts.end(); itr++) {
        if (kUse3ByteStartCode) {
          mExHeader.append(kStartCode3, sizeof(kStartCode3));
        } else {
          mExHeader.append(kStartCode4, sizeof(kStartCode4));
        }
        mExHeader.append(QByteArray::fromBase64(*itr));
      }
    }
  }

  if (pd.key && !pd.sps.is && mExHeader.size() > 0) { // добавляем заголовок
    if (mExHeader.size() + headerDisp <= mMaxHeaderSize) {
      frameSize += mExHeader.size();
      frameData -= mExHeader.size();
      memcpy(frameData, mExHeader.data(), mExHeader.size());
    } else if (mExHeader.size() + frameSize <= mMaxHeaderSize + mMaxFrameSize) {
      QByteArray totalFrame = mExHeader;
      totalFrame.append(frameData, frameSize);
      frameData -= mMaxHeaderSize - headerDisp;
      frameSize = totalFrame.size();
      memcpy(frameData, totalFrame.constData(), totalFrame.size());
      mMaxHeaderSize = mExHeader.size();
    } else {
      return false;
    }
  }
  key = pd.key;
  return true;
}

bool MediaSinkImpl::ModifyAacFrame(char*& frameData, int& frameSize)
{
  if (mExHeader.size() <= mMaxHeaderSize) {
    frameSize += mExHeader.size();
    frameData -= mExHeader.size();
    memcpy(frameData, mExHeader.data(), mExHeader.size());
    return true;
  }
  return false;
}

void MediaSinkImpl::ParseH264Frame(char* frame, int len, H264Data& pd)
{
  memset(&pd, 0, sizeof(pd));

  static const char kStartCode[] = {0x00, 0x00, 0x01};

  static const int iCheckHeadLen = 1024;
  static const int iCorrectHeadLen = sizeof(kStartCode) + 2;

  if(len <= iCorrectHeadLen)
    return;

  int to = qMin(len, iCheckHeadLen) - iCorrectHeadLen;

  static const int chTotal = 4;
  int chCount = 0;

  H264Data::PS* ps = NULL;

  for(int i = 0; i != to; i++)
  {
    if(!memcmp(frame + i, kStartCode, sizeof(kStartCode)))
    {
      if(ps)
      {
        ps->len = i - ps->pos;
        ps = NULL;

        chCount++;
      }

      int bn = i + sizeof(kStartCode);

      int fragment_type = frame[bn] & 0x1F; // nal_unit_type
      int nal_type      = frame[bn + 1] & 0x1F; // nal_unit_type if fragment_type == 28 || 29
      int start_bit     = frame[bn + 1] & 0x80;
      //int end_bit       = frame[bn + 1] & 0x40;

      if(((fragment_type == 28 || fragment_type == 29) && nal_type == 5 && start_bit == 128) || fragment_type == 5)
      {
        pd.key = true;
        chCount++;

        if(!pd.data)
        {
          pd.data = true;
          chCount++;
        }
      }
      else
      {
        nal_type = fragment_type;

        if(nal_type == 7) // SPS
        {
          ps = &pd.sps;
          ps->is = true;
          ps->pos = i + sizeof(kStartCode);
        }
        else if(nal_type == 8) // PPS
        {
          ps = &pd.pps;
          ps->is = true;
          ps->pos = i + sizeof(kStartCode);
        }
        else if(nal_type < 6)
        {
          if(!pd.data)
          {
            pd.data = true;
            chCount++;
          }
        }
      }
    }

    if(chCount >= chTotal)
      break;
  }

  if(ps)
    ps->len = to - ps->pos + iCorrectHeadLen;
}

MediaSinkImpl::MediaSinkImpl(EventLoop* _Source, UsageEnvironment &env, int _MaxFrameSize, ECompression _Compression, const char *_Sprop)
  : MediaSink(env)
  , mSource(_Source), mMaxFrameSize(_MaxFrameSize), mMaxHeaderSize(0)
  , mCompression(_Compression), mSprop(_Sprop)
  , mHasKey(false), mFrameSPS(false), mFramePPS(false)
  , mBaseTimestamp(0)
{
  if (mCompression == eH264) {
    mMaxHeaderSize = (kUse3ByteStartCode? sizeof(kStartCode3): sizeof(kStartCode4)) + mExHeader.size();

    mCurrentFrameSize = mFrameSize = QSize(0, 0);
    if (mSprop && *mSprop) {
      QList<QByteArray> parts = QByteArray(mSprop).split(',');
      for (auto itr = parts.begin(); itr != parts.end(); itr++) {
        QByteArray spropData = (QByteArray::fromBase64(parts.first()));
        H264Sprop sprop;
        sprop.Parse(spropData.constData(), spropData.size());
        if (sprop.HasSps()) {
          mCurrentFrameSize = mFrameSize = QSize(sprop.Width(), sprop.Height());
          Log.Info(QString("Read size from subsession: %1x%2").arg(mFrameSize.width()).arg(mFrameSize.height()));
        }
      }
    }
  //} else if (mCompression == eAac16b) {
  //  mExHeader.clear();
  //  mExHeader.append(0x14);
  //  mExHeader.append(0x8);
  //  mMaxHeaderSize = mExHeader.size();
  }
}

MediaSinkImpl::~MediaSinkImpl()
{
}
