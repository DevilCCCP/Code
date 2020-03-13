#include <QMutexLocker>
#include <QProcess>

#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Net/Chater.h>
#include <Lib/Log/Log.h>
#include <Lib/Db/ObjectType.h>
#include <LibV/Include/VideoMsg.h>
#include <LibV/Include/ModuleNames.h>

#include "Render.h"
#include "PlayerSettings.h"
#include "ArmState.h"
#include "FrameReceiver.h"
#include "Drawer.h"
#include "CameraPlayer.h"


#ifdef QT_NO_DEBUG
#define LogLive(X)
#else
#define LogLive Log.Trace
#endif

bool Render::DoInit()
{
  if (mType == eShmem) {
    if (mStateShmem.create(sizeof(ArmState))) {
      Log.Info(QString("Create new arm state shmem (size: %1, must: %2)").arg(mStateShmem.size()).arg(sizeof(ArmState)));
      memset(mStateShmem.data(), 0, sizeof(ArmState));
    } else if (!mStateShmem.attach()) {
      Log.Fatal("Can't attach StateShmem", true);
      return false;
    } else if (mStateShmem.size() < (int)sizeof(ArmState)) {
      Log.Fatal("StateShmem has invalid size", true);
      return false;
    } else if (mStateShmem.size() > (int)sizeof(ArmState)) {
      Log.Warning(QString("StateShmem has wrong size (shmem: %1, struct: %2)").arg(mStateShmem.size()).arg(sizeof(ArmState)));
    }
  }

  if (!mMonitorShmem.key().isEmpty()) {
    if (mMonitorShmem.attach()) {
      mMonitorState = (MonitorState*)mMonitorShmem.data();
    } else {
      Log.Warning(QString("Monitor shmem attach fail (shmem: '%1', err: '%2')").arg(mMonitorShmem.key()).arg(mMonitorShmem.errorString()));
    }
  }

  return true;
}

bool Render::DoCircle()
{
  switch (mState) {
  case eNone:
    if (!LoadScene()) {
      break;
    }
    mState = eSwitchCamera;
    // no break;

  case eSwitchCamera:
    if (!mPlayPause) {
      ConnectNextCamera();
    }
    mState = ePlayCamera;
    // no break;

  case ePlayCamera:
    if (mType == eShmem) {
      DoUserCamera();
    }
    ConfirmCamera();
    break;
  }

  if (mMonitorState) {
    UpdateMonitorState();
  }
  return true;
}

void Render::DoRelease()
{
  QMutexLocker lock(&mPlayerMutex);
  if (mCurrentPlayer) {
    mPlayerTrash.append(mCurrentPlayer);
    mCurrentPlayer.clear();
  }
  if (mNextPlayer) {
    mPlayerTrash.append(mNextPlayer);
    mNextPlayer.clear();
  }

  if (!mPlayerTrash.isEmpty()) {
    Log.Info(QString("Wait players finish"));
    while (!mPlayerTrash.isEmpty()) {
      lock.unlock();
      Rest(10);
      lock.relock();
    }
    Log.Info(QString("All players finished"));
  }
}

void Render::AttachToShmem(int shmemIndex)
{
  mMonitorShmem.setKey(GetMonitorShmemName(shmemIndex));
}

bool Render::InitSingleScene(int camId, int armId)
{
  mType          = eCustom;
  mArmId         = armId;
  mCustomCamera  = camId;
  mScene.Monitor = 0;

  mScene.Width = mScene.Height = 100;
  mScene.Box = QRect(QPoint(10, 10), QPoint(90, 90));
  mPrime = true;

  return true;
}

bool Render::InitCustomScene(int id, int type, int count, int monitor, int camId, int armId, const qint64& ts)
{
  mType         = eCustom;
  mArmId        = armId;
  mCustomCamera = camId;
  mTimestamp    = ts;
  mScene.Monitor = monitor;

  switch (type) {
  case eNoPrime:   InitTable(count, id); break;
  case ePrimeHv:   InitCorner(count, id); break;
  case ePrimeHorz: InitHorz(count, id); break;
  case ePrimeVert: InitVert(count, id); break;
  default:
    return false;
  }

  return true;
}

bool Render::InitUserCamera()
{
  mType = eShmem;

  return true;
}

void Render::InitTable(int count, int id)
{
  int n = (int)(sqrt(count + 0.0001) - 0.0001) + 1;
  mScene.Width = mScene.Height = n;
  int j = ((id - 1) / n);
  int i = ((id - 1) % n);
  mScene.Box = QRect(QPoint(i, j), QPoint(i, j));
  mPrime = false;
}

void Render::InitCorner(int count, int id)
{
  int n = qMax((count - 2) / 2, 2);
  mScene.Width = mScene.Height = n + 1;
  if (id == 1) {
    mPrime = true;
    mScene.Box = QRect(QPoint(0, 0), QPoint(n - 1, n - 1));
  } else if (id <= n + 2) {
    mPrime = false;
    mScene.Box = QRect(QPoint(id - 2, n), QPoint(id - 2, n));
  } else if (id <= 2 * n + 2) {
    mPrime = false;
    int v = (2 * n + 2) - id;
    mScene.Box = QRect(QPoint(n, v), QPoint(n, v));
  } else {
    GetOverseer()->Done();
  }
}

void Render::InitHorz(int count, int id)
{
  int n = qMax(count - 1, 2);
  mScene.Width = mScene.Height = n;
  if (id == 1) {
    mPrime = true;
    mScene.Box = QRect(QPoint(0, 0), QPoint(n - 1, n - 2));
  } else if (id <= n + 1) {
    mPrime = false;
    mScene.Box = QRect(QPoint(id - 2, n - 1), QPoint(id - 2, n - 1));
  } else {
    GetOverseer()->Done();
  }
}

void Render::InitVert(int count, int id)
{
  int n = qMax(count - 1, 2);
  mScene.Width = mScene.Height = n;
  if (id == 1) {
    mPrime = true;
    mScene.Box = QRect(QPoint(0, 0), QPoint(n - 2, n - 1));
  } else if (id <= n + 1) {
    mPrime = false;
    mScene.Box = QRect(QPoint(n - 1, id - 2), QPoint(n - 1, id - 2));
  } else {
    GetOverseer()->Done();
  }
}

bool Render::DoUserCamera()
{
  if (ReadShmem()) {
    Log.Info(QString("Update shmem (cam: %1, box: %2, %3, %4, %5)").arg(mArmState.CameraSimple).arg(mArmState.CameraSimpleX).arg(mArmState.CameraSimpleY)
      .arg(mArmState.CameraSimpleW).arg(mArmState.CameraSimpleH));

    UpdateShmemScene();
//    mDrawer->UpdateSceneBox(mScene.Box);
    UpdateShmemCameras();
  }
  return true;
}

void Render::DisconnectCurrentCamera()
{
  {
    QMutexLocker lock(&mPlayerMutex);
    ClearCurrentPlayer(true);
  }

  mDrawer->ClearConveyor();
}

void Render::ConnectNextCamera()
{
  mDrawer->StatusChanged((mCameras.size() > 0)? eConnecting: eProblem);

  for (auto itr = mCameras.begin(); itr != mCameras.end(); itr++) {
    CameraInfo& info = itr.value();
    ConnectCamera(info);
  }
}

void Render::ConfirmCamera()
{
  QMutexLocker lock(&mPlayerMutex);
  if (mCurrentPlayer) {
    if (mCurrentPlayer->NeedConfirm()) {
//      Log.Trace(QString("ConfirmCamera (camera: %1)").arg(mCurrentPlayer->CameraId()));
      mCurrentPlayer->Chat()->SendSimpleMessage(eMsgContinue);
    }
  } else if (mNextPlayer) {
    if (mNextPlayer->NeedTest()) {
      mNextPlayer->Chat()->SendSimpleMessage(eMsgContinue);
    }
  }
}

void Render::SwitchToNextCamera()
{
  QMutexLocker lock(&mPlayerMutex);
  ClearCurrentPlayer();
  mCurrentPlayer = mNextPlayer;
  mNextPlayer.clear();
  int newCameraId = mCurrentPlayer->CameraId();
  lock.unlock();
  mDrawer->CameraChanged(mCameras[newCameraId].Name);
}

void Render::PlayNextFrame(FrameS &frame)
{
//  Log.Trace(QString("Frame %1").arg(frame->GetHeader()->HeaderSize));
  EmergeVFrame(frame);
}

void Render::ClearCurrentPlayer(bool andNext)
{
  if (mCurrentPlayer) {
    mCurrentPlayer->Clear();
    mPlayerTrash.append(mCurrentPlayer);
    mCurrentPlayer.clear();
  }
  if (andNext && mNextPlayer) {
    mNextPlayer->Clear();
    mPlayerTrash.append(mNextPlayer);
    mNextPlayer.clear();
  }
}

bool Render::LoadScene()
{
  if (mPlayerSettings->LoadUserSettings()) {
    Log.Info(QString("Settings loaded for user"));
  } else if (mPlayerSettings->LoadDbSettings(mDb, mArmId)) {
    Log.Info(QString("Settings loaded for user"));
  } else {
    return false;
  }

  switch (mType) {
  case eDb:     return LoadTrueScene();
  case eCustom: return LoadCustomScene();
  case eShmem:  return LoadShmemScene();
  default:      return false;
  }
}

bool Render::LoadTrueScene()
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT m.num, m.size, l.place, l.flag, o._id, o.name, o.uri FROM arm_monitor_layouts l"
                     " INNER JOIN arm_monitors m ON m._id = l._amonitor"
                     " INNER JOIN arm_monitor_lay_cameras c ON c._amlayout = l._id"
                     " INNER JOIN object o ON o._id = c._camera"
                     " WHERE l._id = %1").arg(GetOverseer()->Id()));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  if (q->next()) {
    mScene.Monitor = q->value(0).toInt();
    QStringList point = q->value(1).toString().split(QRegExp("\\D"), QString::SkipEmptyParts);
    if (point.size() != 2) {
      return false;
    }
    mScene.Width = point[0].toInt();
    mScene.Height = point[1].toInt();
    QStringList box = q->value(2).toString().split(QRegExp("\\D"), QString::SkipEmptyParts);
    if (box.size() != 4) {
      return false;
    }
    mScene.Box.setRight(box[0].toInt());
    mScene.Box.setBottom(box[1].toInt());
    mScene.Box.setLeft(box[2].toInt());
    mScene.Box.setTop(box[3].toInt());
    if (mScene.Box.right() >= mScene.Width || mScene.Box.bottom() >= mScene.Height) {
      return false;
    }
    mScene.Flag = q->value(3).toInt();
    do {
      CameraInfo cam;
      cam.Id = q->value(4).toInt();
      cam.Name = q->value(5).toString();
      cam.VideoUri = Uri::FromString(q->value(6).toString());
      mCameras.insert(cam.Id, cam);
    } while (q->next());

    if (!mDrawer->CreateScene(this, mScene, mPlayerSettings->getStyle(), false)) {
      return false;
    }

    if (mCameras.size() == 1) {
      mDrawer->CameraChanged(mCameras.first().Name);
    }
    return true;
  }
  LOG_WARNING_ONCE("Scene empty");
  return false;
}

bool Render::LoadCustomScene()
{
  if (mCustomCamera) {
    if (!LoadCustomCamera()) {
      return false;
    }
  }

  SetSceneFlags();
  return mDrawer->CreateScene(this, mScene, mPlayerSettings->getStyle(), true);
}

bool Render::LoadCustomCamera()
{
  mCameras.clear();

  if (mCustomCamera) {
    auto q = mDb.MakeQuery();
    q->prepare(QString("SELECT name, uri FROM object WHERE _id = %1").arg(mCustomCamera));
    if (!mDb.ExecuteQuery(q)) {
      return false;
    }

    while (q->next()) {
      CameraInfo cam;
      cam.Id        = mCustomCamera;
      cam.Name      = q->value(0).toString();
      cam.VideoUri  = Uri::FromString(q->value(1).toString());
      cam.Timestamp = mTimestamp;
      mCameras.insert(cam.Id, cam);
    }
  }
  return true;
}

bool Render::LoadShmemScene()
{
  ReadShmem();
  UpdateShmemScene();

  if (!mDrawer->CreateScene(this, mScene, mPlayerSettings->getStyle(), true)) {
    return false;
  }
  UpdateShmemCameras();
  return true;
}

void Render::SetSceneFlags()
{
  mScene.Flag = 0;
  if (mPlayerSettings->getScaleBest()) {
    mScene.Flag |= eScaleBest;
  } if (mPlayerSettings->getShowMouse()) {
    mScene.Flag |= eShowMouse;
  } if (mPlayerSettings->getAutoHideMouse()) {
    mScene.Flag |= eAutoHideMouse;
  }
}

void Render::UpdateMonitorState()
{
  EMonitorPlace monitorPlace = mMonitorState->FullScreenLayout
      ? (mMonitorState->FullScreenLayout == GetOverseer()->Id()? eFullScreen: eHidden)
      : eWindow;
  if (monitorPlace != mScene.Place) {
    mScene.Place = monitorPlace;
    mDrawer->Show(mScene.Place);
  }
}

bool Render::ReadShmem()
{
  mArmState = *static_cast<ArmState*>(mStateShmem.data());

  if (mArmState.CameraCounter == mCameraCounter) {
    return false;
  }

  do {
    mCameraCounter = mArmState.CameraCounter;
    mArmState = *static_cast<ArmState*>(mStateShmem.data());
  } while (mCameraCounter != mArmState.CameraCounter || mCameraCounter != mArmState.CameraEndCounter);

  return true;
}

void Render::UpdateShmemScene()
{
  mScene.Monitor = -1;
  mScene.Width   = 0;
  mScene.Height  = 0;
  mScene.Box     = QRect(mArmState.CameraSimpleX, mArmState.CameraSimpleY, mArmState.CameraSimpleW, mArmState.CameraSimpleH);
  SetSceneFlags();
}

void Render::UpdateShmemCameras()
{
  if (mCustomCamera != mArmState.CameraSimple) {
    DisconnectCurrentCamera();
    mCameras.clear();
    mCustomCamera = mArmState.CameraSimple;
    mTimestamp    = mArmState.TimestampSimple;
    const ObjectItem* objItem = static_cast<ObjectItem*>(mObjectTable->GetItem(mCustomCamera).data());
    if (objItem) {
      CameraInfo cam;
      cam.Id        = objItem->Id;
      cam.Name      = objItem->Name;
      cam.VideoUri  = Uri::FromString(objItem->Uri);
      cam.Timestamp = mTimestamp;

      mCameras[cam.Id] = cam;
      //ConnectNextCamera();
      mState = eSwitchCamera;
    }
    if (mCameras.size() == 1) {
      mDrawer->CameraChanged(mCameras.first().Name);
    } else {
      mDrawer->CameraChanged("");
    }
  } else if (mTimestamp != mArmState.TimestampSimple || mArmState.CameraSimpleChange) {
    DisconnectCurrentCamera();
    mTimestamp    = mArmState.TimestampSimple;
    for (auto itr = mCameras.begin(); itr != mCameras.end(); itr++) {
      CameraInfo* info = &itr.value();
      info->Timestamp = mTimestamp;
    }
    ConnectNextCamera();
  }
}

void Render::ConnectCamera(CameraInfo& info)
{
  LogLive(QString("ConnectCamera (camera: %1, ts: %2)").arg(info.Id).arg(info.Timestamp));

  ReceiverS receiver = ReceiverS(new FrameReceiver<Render>(this, info.Id));
  ChaterS chater = Chater::CreateChater(GetManager(), info.VideoUri, receiver);
  QMutexLocker lock(&mPlayerMutex);
  mNextPlayer = CameraPlayerS(new CameraPlayer(chater, info.Id));
  static_cast<FrameReceiver<Render>*>(receiver.data())->ConnectPlayer(mNextPlayer.data());
  lock.unlock();

  if (!info.Timestamp) {
    LiveRequest* req;
    if (chater->PrepareMessage(eMsgLiveRequest, req)) {
      req->CameraId = info.Id;
      req->Priority = 0;
      chater->SendMessage();
    }
  } else {
    ArchRequest* req;
    if (chater->PrepareMessage(eMsgArchRequest, req)) {
      req->CameraId   = info.Id;
      req->Priority   = 0;
      req->Timestamp  = info.Timestamp;
      req->SpeedNum   = 1;
      req->SpeedDenum = 1;
      chater->SendMessage();
    }
  }
}

void Render::VideoGranted(CameraPlayer *player)
{
  Log.Info(QString("VideoGranted (camera: %1)").arg(player->CameraId()));
  if (OnReceiveNew(player)) {
    mDrawer->StatusChanged(eConnected);
  }
}

void Render::VideoDenied(CameraPlayer *player)
{
  Log.Info(QString("VideoDenied (camera: %1)").arg(player->CameraId()));
  if (OnReceiveNew(player)) {
    mDrawer->StatusChanged(eProblem);
  }
}

void Render::VideoNoStorage(CameraPlayer* player)
{
  Log.Info(QString("Video no storage (camera: %1)").arg(player->CameraId()));
  if (OnReceiveNew(player)) {
    mDrawer->StatusChanged(eProblem);
  }
}

void Render::VideoDropped(CameraPlayer *player)
{
  Log.Info(QString("VideoDropped (camera: %1)").arg(player->CameraId()));
  if (OnReceiveNew(player)) {
    mDrawer->StatusChanged(eProblem);
    Disconnected(player);
  }
}

void Render::VideoAborted(CameraPlayer *player)
{
  Log.Info(QString("VideoDropped (camera: %1)").arg(player->CameraId()));
  if (OnReceiveNew(player)) {
    mDrawer->StatusChanged(eProblem);
    Disconnected(player);
  }
}

void Render::VideoInfo(CameraPlayer *player, const qint64 &timestamp)
{
  LogLive(QString("VideoInfo from '%2' (camera: %1)").arg(player->CameraId()).arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate)));
  QMutexLocker lock(&mPlayerMutex);
  if (OnReceiveNew(player)) {
    mCurrentPlayer->StartTimestamp() = timestamp;
  }
}

void Render::VideoFrame(CameraPlayer *player, FrameS &frame)
{
  //LogLive(QString("VideoFrame size %2 (camera: %1)").arg(player->CameraId()).arg(frame->Size()));
  if (OnReceiveNew(player)) {
    PlayNextFrame(frame);
  }
}

void Render::Disconnected(CameraPlayer *player)
{
  LogLive(QString("Disconnected (camera: %1)").arg(player->CameraId()));
  QMutexLocker lock(&mPlayerMutex);
  if (mCurrentPlayer.data() == player) {
    if (mCameras.size() > 1) {
      if (!mNextPlayer) {
        mState = eSwitchCamera;
      }
      ClearCurrentPlayer();
      mCurrentPlayer = mNextPlayer;
    } else {
      mState = eSwitchCamera;
    }
    mDrawer->StatusChanged(eNotConnected);
  } else if (mNextPlayer.data() == player) {
    mNextPlayer.clear();
    mState = eSwitchCamera;
    mDrawer->StatusChanged(eNotConnected);
  } else {
    for (auto itr = mPlayerTrash.begin(); itr != mPlayerTrash.end(); itr++) {
      const CameraPlayerS& p = *itr;
      if (p.data() == player) {
        mPlayerTrash.erase(itr);
        if (IsStop()) {
          WakeUp();
        }
        break;
      }
    }
  }
}

void Render::OnMute(bool mute)
{
  mDrawer->Mute(mute);
}

void Render::OnPlay()
{
  QMutexLocker lock(&mPlayerMutex);
  mPlayPause = false;
  if (!mCurrentPlayer) {
    mState = eSwitchCamera;
  }
  lock.unlock();
  mDrawer->Play();
}

void Render::OnPause()
{
  QMutexLocker lock(&mPlayerMutex);
  mPlayPause = true;
  lock.unlock();
  mDrawer->Pause();
}

void Render::OnSwitchLive()
{
  QMutexLocker lock(&mPlayerMutex);
  if (mCurrentPlayer) {
    ChaterS chater = mCurrentPlayer->Chat();
    LiveRequest* req;
    if (chater->PrepareMessage(eMsgLiveRequest, req)) {
      req->CameraId = mCurrentPlayer->CameraId();
      req->Priority = 0;
      chater->SendMessage();
    }
  }
}

void Render::OnSwitchArchive(const QDateTime& timestamp, int speedNum, int speedDenum)
{
  QMutexLocker lock(&mPlayerMutex);
  if (mCurrentPlayer) {
    ChaterS chater = mCurrentPlayer->Chat();
    ArchRequest* req;
    if (chater->PrepareMessage(eMsgArchRequest, req)) {
      req->CameraId   = mCurrentPlayer->CameraId();
      req->Priority   = 0;
      req->Timestamp  = timestamp.toMSecsSinceEpoch();
      req->SpeedNum   = speedNum;
      req->SpeedDenum = speedDenum;
      chater->SendMessage();
    }
  }
}

void Render::OnDownload(const QDateTime &tsFrom, const QDateTime &tsTo)
{
  QMutexLocker lock(&mPlayerMutex);
  if (mCurrentPlayer) {
    int id = mCurrentPlayer->CameraId();
    QStringList params;
    params << "-t";
    params << (QString("--id=-%1").arg(id));
    params << (QString("-pdownload;%1;%2;%3;%4")
               .arg(id).arg(tsFrom.toString(Qt::ISODate)).arg(tsTo.toString(Qt::ISODate)).arg(32));
    QProcess::startDetached(kPlayerExe, params);
  }
}

void Render::OnScreenPlaceChange()
{
  if (!mMonitorState) {
    return;
  }

  if (mMonitorState->FullScreenLayout) {
    mMonitorState->FullScreenLayout = 0;
  } else {
    mMonitorState->FullScreenLayout = GetOverseer()->Id();
  }
}

bool Render::OnReceiveNew(CameraPlayer *player)
{
  if (mCurrentPlayer.data() == player) {
    return true;
  } else if (mNextPlayer.data() == player) {
    SwitchToNextCamera();
    return true;
  }
  return false;
}


Render::Render(Db &_Db, DrawerS &_Drawer)
  : ConveyorV(1)
  , mDb(_Db), mObjectTable(new ObjectTable(mDb))
  , mDrawer(_Drawer), mType(eDb), mCustomCamera(0), mTimestamp(0), mPrime(false), mPlayerSettings(new PlayerSettings())
  , mState(eNone), mPlayLive(true), mPlayPause(false)
  , mStateShmem(kArmDaemon), mCameraCounter(0)
  , mMonitorShmem(), mMonitorState(nullptr)
{
  memset(&mArmState, 0, sizeof(mArmState));
  memset(&mScene, 0, sizeof(mScene));
}

Render::~Render()
{
}
