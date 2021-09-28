#pragma once

#include <QString>
#include <QByteArray>

#include <Lib/Include/Common.h>

#include "Channel.h"


DefineClassS(MediaPlayer);
DefineClassS(Media);
DefineClassS(Channel);
DefineClassS(MediaPlayerManager);

typedef QMap<QByteArray, QByteArray> ParamsMap;

/*abstract*/class MediaPlayer
{
  Channel* mOutChannel;

protected:
  Channel* GetOutChannel() { return mOutChannel; }
  Media* GetMedia() { return mOutChannel->GetMedia(); }

public:
  bool Open(Channel* _OutChannel) { mOutChannel = _OutChannel; return Open(); }

protected:
  /*new */virtual bool Open() = 0;
public:
  /*new */virtual bool Play(const ParamsMap& params, QByteArray& extraData) = 0;
  /*new */virtual bool Pause() = 0;
  /*new */virtual void Stop() = 0;
  /*new */virtual void Release() = 0;
  /*new */virtual bool Test() { return true; }
  /*new */virtual void GetParameters(const QByteArray& dataIn, QByteArray& dataOut, QByteArray& extraHeader)
  { Q_UNUSED(dataIn); Q_UNUSED(dataOut); Q_UNUSED(extraHeader); }

protected:
  void OutFrame(const TrFrameS& trFrame) { mOutChannel->InFrame(trFrame); }

public:
  explicit MediaPlayer() { }
  /*new */virtual ~MediaPlayer() { }
};

class MediaPlayerManager
{
public:
  /*new */virtual MediaPlayerS CreateMediaPlayer() = 0;

public:
  explicit MediaPlayerManager() { }
  /*new */virtual ~MediaPlayerManager() { }
};

template <typename MediaPlayerT>
class MediaPlayerManagerA: public MediaPlayerManager
{
public:
  /*override */virtual MediaPlayerS CreateMediaPlayer() override { return MediaPlayerS(new MediaPlayerT()); }

public:
  explicit MediaPlayerManagerA() { }
  /*override */virtual ~MediaPlayerManagerA() { }
};

template <typename MediaPlayerT, typename ParentT>
class MediaPlayerManagerB: public MediaPlayerManager
{
  ParentT mParent;

public:
  const ParentT& Parent() const { return mParent; }
  ParentT& Parent() { return mParent; }

public:
  /*override */virtual MediaPlayerS CreateMediaPlayer() override { return MediaPlayerS(new MediaPlayerT(mParent)); }

public:
  explicit MediaPlayerManagerB(ParentT _Parent)
    : mParent(_Parent)
  { }
  /*override */virtual ~MediaPlayerManagerB() { }
};
