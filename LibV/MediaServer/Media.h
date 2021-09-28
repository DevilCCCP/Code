#pragma once

#include <QList>
#include <QString>

#include <Lib/Include/Common.h>

#include "Channel.h"


DefineClassS(Media);
DefineClassS(MediaPlayer);
DefineClassS(MediaPlayerManager);
DefineClassS(Channel);
DefineClassS(TrFrame);

typedef QList<ChannelS> ListChannel;

class Media: public Channel
{
  enum EState {
    eNotStart,
    eStarted,
    eStopped,
    eIllegal
  };

  QString      mId;

  EState       mState;
  ListChannel  mChannels;
  QMutex       mMutex;

public:
  const QString& Id() { return mId; }

protected:
  /*override */virtual void OnFrame(const TrFrameS& frame) override;

  /*new */virtual void OnChannelAdd(const ChannelS& channel);
  /*new */virtual void OnChannelRemove(const ChannelS& channel);

public:
  bool Start();
  bool Test();
  void Stop();

  bool AddChannel(const ChannelS& channel);
  void RemoveChannel(const ChannelS& channel);

private:
  void TestChannels();
  void StopChannels();
  ListChannel::iterator EraseChannel(const ListChannel::iterator& itr);
  const char* StateToString();

public:
  explicit Media(const QString& _Id, const MediaPlayerS& _MediaPlayer = MediaPlayerS());
  /*new */virtual ~Media();
};

