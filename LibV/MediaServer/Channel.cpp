#include "Channel.h"
#include "Media.h"
#include "MediaPlayer.h"


bool Channel::OnStart()
{
  return true;
}

bool Channel::OnTest()
{
  return true;
}

void Channel::OnStop()
{
}

bool Channel::TestChannel()
{
  if (mMediaPlayer) {
    if (!mMediaPlayer->Test()) {
      return false;
    }
  }
  return OnTest();
}

void Channel::StopChannel()
{
  if (mMediaPlayer) {
    mMediaPlayer->Release();
  }
  return OnStop();
}

const MediaPlayerS& Channel::GetMediaPlayer()
{
  if (mMediaPlayer) {
    return mMediaPlayer;
  } else if (mMedia) {
    return mMedia->GetMediaPlayer();
  } else {
    static MediaPlayerS noneMedia;
    return noneMedia;
  }
}


Channel::Channel(Media* _Media, const MediaPlayerS& _MediaPlayer)
  : mMedia(_Media), mMediaPlayer(_MediaPlayer)
{
}

Channel::~Channel()
{
}

