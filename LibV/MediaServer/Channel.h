#pragma once

#include <QList>
#include <QMutex>

#include <Lib/Include/Common.h>


DefineClassS(Channel);
DefineClassS(Media);
DefineClassS(MediaPlayer);
DefineClassS(TrFrame);

class Channel
{
  Media*       mMedia;
  MediaPlayerS mMediaPlayer;

public:
  Media* GetMedia() { return mMedia; }

protected:
  /*new */virtual bool OnStart();
  /*new */virtual bool OnTest();
  /*new */virtual void OnStop();

  /*new */virtual void OnFrame(const TrFrameS& frame) = 0;
  /*new */virtual void OnClearFrames() { }

public:
  bool TestChannel();
  void StopChannel();

  void InFrame(const TrFrameS& frame) { OnFrame(frame); }
  void ClearFrames() { OnClearFrames(); }

  const MediaPlayerS& GetMediaPlayer();

public:
  explicit Channel(Media* _Media, const MediaPlayerS& _MediaPlayer = MediaPlayerS());
  /*new */virtual ~Channel();
};

