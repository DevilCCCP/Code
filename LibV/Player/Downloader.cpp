#include <QMutexLocker>
#include <QSettings>
#include <QDir>

#include <Lib/Include/Names.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Net/Chater.h>
#include <Lib/Log/Log.h>
#include <LibV/Include/VideoMsg.h>
#include <LibV/Player/FrameReceiver.h>
#include <LibV/Player/CameraPlayer.h>

#include "Downloader.h"
#include "FileSaver.h"


const int kDownFrameOverflowCount = 800;
const int kReconnectDelayMs = 2000;

bool Downloader::DoCircle()
{
  switch (mState) {
  case eNone:
    mState = eSeekCamera;
    /* fall through */
  case eSeekCamera:
    if (LoadCameraInfo()) {
      mState = eConnectCamera;
    } else {
      break;
    }
    /* fall through */
  case eConnectCamera:
    if (ConnectCamera()) {
      mState = ePlayCamera;
    }
    break;

  case ePlayCamera:
    ConfirmCamera();
    break;

  case eReconnectCamera:
    if (ReconnectCamera()) {
      mState = ePlayCamera;
    }
    break;

  case eDelayedReconnectCamera:
    DisconnectCamera();
    mReconnectTimer.start();
    mState = eWaitReconnectCamera;
    break;

  case eWaitReconnectCamera:
    DelayedReconnect();
    break;

  case eDone:
  case eFail:
    return false;
  }
  return true;
}

void Downloader::DoRelease()
{
  if (mCameraPlayer)
    mCameraPlayer->Chat()->Close();
  if (mCloseSaver && mCurrentSaver) {
    mCurrentSaver->Stop();
  }
}

//void Downloader::OnOverflowWarn()
//{
//  if (mState != eDelayedReconnectCamera) {
//    Log.Warning(QString("Overflow warning, disconnecting"));
//  }
//  mState = eDelayedReconnectCamera;
//}

int Downloader::GetPercent()
{
  return (mEndTime > mStartTime)? (int)((qint64)100 * (mCurrentTime - mStartTime) / (mEndTime - mStartTime)): 100;
}

bool Downloader::LoadCameraInfo()
{
  if (!mDb) {
    mDb.reset(new Db());
    if (!mDb->OpenDefault()) {
      mDb.reset();
      return false;
    }
  }
  if (!mDb->Connect()) {
    return false;
  }

  auto q = mDb->MakeQuery();
  q->prepare(QString("SELECT name, uri FROM object WHERE _id = %1;").arg(mCameraId));
  if (!mDb->ExecuteQuery(q) || !q->next()) {
    return false;
  }

  mCameraInfo.Id = mCameraId;
  mCameraInfo.Name = q->value(0).toString();
  mCameraInfo.VideoUri = Uri::FromString(q->value(1).toString());
  if (mCameraInfo.VideoUri.Type() == Uri::eTypeIllegal) {
    return false;
  }

  if (mCurrentSaver) {
    Log.Warning(QString("File saver already exists"));
    return true;
  }

  QString filename = (mFullname)? mSavePath: GenerateFilename();
  mCurrentSaver = FileSaverS(new FileSaver(filename, this));
  GetManager()->RegisterWorker(mCurrentSaver);
  ConnectModule(mCurrentSaver.data());
  return true;
}

bool Downloader::ConnectCamera()
{
  mConnecting = true;
  mTooOldFrames = 0;
  Log.Info(QString("ConnectCamera (camera: %1)").arg(mCameraInfo.Id));

  ReceiverS receiver = ReceiverS(new FrameReceiver<Downloader>(this, mCameraInfo.Id));
  ChaterS chater = Chater::CreateChater(GetManager(), mCameraInfo.VideoUri, receiver);
  mCameraPlayer = CameraPlayerS(new CameraPlayer(chater, mCameraInfo.Id));
  static_cast<FrameReceiver<Downloader>* >(receiver.data())->ConnectPlayer(mCameraPlayer.data());

  ArchRequest* req;
  if (chater->PrepareMessage(eMsgArchRequest, req)) {
    req->CameraId   = mCameraInfo.Id;
    req->Priority   = 0;
    req->Timestamp  = mCurrentTime;
    req->SpeedNum   = mRequestSpeed;
    req->SpeedDenum = 1;
    return chater->SendMessage();
  }
  return false;
}

void Downloader::DisconnectCamera()
{
  if (mCameraPlayer) {
    mCameraPlayer->Chat()->Close();
  }
  mCameraPlayer.clear();
  mState = eConnectCamera;
}

bool Downloader::ReconnectCamera()
{
  DisconnectCamera();
  return ConnectCamera();
}

void Downloader::DelayedReconnect()
{
  if (mReconnectTimer.elapsed() >= kReconnectDelayMs) {
    if (mCurrentSaver && mCurrentSaver->HasOverflow()) {
      mReconnectTimer.start();
    } else {
      Log.Info(QString("Overflow ends, reconnecting"));
      mState = eReconnectCamera;
    }
  }
}

void Downloader::ConfirmCamera()
{
  if (mCameraPlayer) {
    if (mCameraPlayer->NeedConfirm()) {
//      Log.Trace(QString("ConfirmCamera (camera: %1)").arg(mCameraPlayer->CameraId()));
      mCameraPlayer->Chat()->SendSimpleMessage(eMsgContinue);
    }
  }
}

QString Downloader::GenerateFilename()
{
  QString downDir = mSavePath;
  QDir dd(downDir);
  if (downDir.isEmpty() || (!dd.exists() && !dd.mkpath(downDir))) {
    Log.Fatal(QString("Invalid path '%1'").arg(downDir));
    GetManager()->Stop();
    return QString();
  }

  QString filenamePattern = QString("%1/%2 (%3 %4)").arg(downDir).arg(mCameraInfo.Name)
      .arg(QDateTime::fromMSecsSinceEpoch(mStartTime).toString(Qt::ISODate).replace(QChar(':'), "-"))
      .arg(QDateTime::fromMSecsSinceEpoch(mEndTime).toString(Qt::ISODate).replace(QChar(':'), "-"));
  QString filename = filenamePattern + ".mp4";
  for (int i = 0; dd.exists(filename); i++) {
    filename = QString("%1.%2.mp4").arg(filenamePattern).arg(i, 3, 10, QChar('0'));
  }
  return filename;
}

void Downloader::ProcessOverflow()
{
  if (mState != eDelayedReconnectCamera && mState != eWaitReconnectCamera) {
    Log.Warning(QString("Overflow warning, disconnecting"));
    mState = eDelayedReconnectCamera;
  }
}

void Downloader::VideoGranted(CameraPlayer *player)
{
  Log.Info(QString("VideoGranted (camera: %1)").arg(player->CameraId()));
  if (player == mCameraPlayer.data()) {
    ;
  }
}

void Downloader::VideoDenied(CameraPlayer *player)
{
  Log.Info(QString("VideoDenied (camera: %1)").arg(player->CameraId()));
  if (player == mCameraPlayer.data()) {
    mState = eDelayedReconnectCamera;
  }
}

void Downloader::VideoNoStorage(CameraPlayer* player)
{
  Log.Info(QString("Video no storage (camera: %1)").arg(player->CameraId()));
  EmergeVFrame(FrameS());
  if (player == mCameraPlayer.data()) {
    mState = eFail;
  }
}

void Downloader::VideoDropped(CameraPlayer *player)
{
  Log.Info(QString("VideoDropped (camera: %1)").arg(player->CameraId()));
  if (player == mCameraPlayer.data()) {
    mState = eDelayedReconnectCamera;
  }
}

void Downloader::VideoAborted(CameraPlayer *player)
{
  Log.Info(QString("VideoAborted (camera: %1)").arg(player->CameraId()));
  if (player == mCameraPlayer.data()) {
    mState = eReconnectCamera;
  }
}

void Downloader::VideoInfo(CameraPlayer *player, const qint64 &timestamp)
{
  Log.Trace(QString("VideoInfo from '%2' (camera: %1)").arg(player->CameraId())
            .arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate)));
  if (player == mCameraPlayer.data()) {
    mCameraPlayer->StartTimestamp() = timestamp;
  }
}

void Downloader::VideoFrame(CameraPlayer *player, FrameS &frame)
{
//  Log.Trace(QString("VideoFrame size %2 (camera: %1)").arg(player->CameraId()).arg(frame->Size()));
  if (player != mCameraPlayer.data()) {
    return;
  }
  if (frame->GetHeader()->Timestamp < mCurrentTime) {
    if (mConnecting) {
      if (frame->GetHeader()->Timestamp < mStartTime) {
        mStartTime = frame->GetHeader()->Timestamp;
      } else {
        return;
      }
    } else {
      if (!mTooOldFrames) {
        Log.Warning(QString("Too old frame received (cur: %1, got: %2)")
                    .arg(QDateTime::fromMSecsSinceEpoch(mCurrentTime).toString())
                    .arg(QDateTime::fromMSecsSinceEpoch(frame->GetHeader()->Timestamp).toString()));
      }
      mTooOldFrames++;
      return;
    }
  }

  mCurrentTime = frame->GetHeader()->Timestamp;
  if (mTooOldFrames) {
    Log.Info(QString("Too old frame skip ended (skipped: %1, cur: %2)").arg(mTooOldFrames)
             .arg(QDateTime::fromMSecsSinceEpoch(mCurrentTime).toString()));
    mTooOldFrames = 0;
  }

  int perc = GetPercent();
  if (perc > mCurrentPercent) {
    mCurrentPercent = perc;
    if (mDownPercent) {
      mDownPercent->OnPercent(mTaskId, mCurrentPercent);
    }
  }
  if (mCurrentTime <= mEndTime) {
    if (!EmergeVFrame(frame)) {
      ProcessOverflow();
    }
  } else {
    EmergeVFrame(FrameS());
  }
}

void Downloader::Disconnected(CameraPlayer *player)
{
  Log.Trace(QString("Disconnected (camera: %1)").arg(player->CameraId()));
  if (player == mCameraPlayer.data()) {
    mState = eReconnectCamera;
  }
}


Downloader::Downloader(const DbS& _Db, const int _CameraId, const qint64& _StartTime, const qint64& _EndTime
                       , const int& _RequestSpeed, const QString& _SavePath, bool _Fullname, bool _ExitOnDone)
  : ConveyorV(-1, kDownFrameOverflowCount)
  , mDb(_Db), mCameraId(_CameraId), mRequestSpeed(_RequestSpeed), mSavePath(_SavePath), mFullname(_Fullname), mExitOnDone(_ExitOnDone)
  , mDownPercent(nullptr), mTaskId(0)
  , mState(eSeekCamera), mStartTime(_StartTime), mEndTime(_EndTime), mCurrentTime(mStartTime), mCurrentPercent(-1)
  , mCloseSaver(false)
{
}

Downloader::~Downloader()
{
}
