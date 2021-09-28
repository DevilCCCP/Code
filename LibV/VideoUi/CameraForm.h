#pragma once

#include <QWidget>
#include <QTimer>

#include <Lib/Db/Db.h>

#include "ImageWithPoints.h"


DefineClassS(CtrlManager);
DefineClassS(Decoder);
DefineClassS(DecodeReceiver);
DefineClassS(Chater);
DefineClassS(Frame);

namespace Ui {
class CameraForm;
}

class CameraForm: public QWidget
{
  Ui::CameraForm* ui;

  CtrlManager*    mManager;
  ObjectItemS     mCameraObject;
  ObjectItemS     mAnalObject;
  int             mArmObjectId;
  EPointsType     mPointsType;

  ChaterS         mChater;
  DecoderS        mDecoder;
  DecodeReceiverS mDecodeReceiver;
  bool            mScaled;
  QTimer*         mAutoRefresh;
  int             mAutoRefreshCount;

  FrameS          mSourceFrame;
  QImage          mSourceImg;
  QIcon           mNoVideo;
  bool            mStatusFrame;
  bool            mNeedSave;
  QProcess*       mPlayProcess;
  QTimer*         mPlayConfirmTimer;
  bool            mPlaying;

  Q_OBJECT

public:
  explicit CameraForm(CtrlManager* _Manager, const ObjectItemS& _Object, int _ArmObjectId, QWidget *parent = 0);
  explicit CameraForm(CtrlManager* _Manager, const ObjectItemS& _CameraObject, const ObjectItemS& _AnalObject
                      , const Db& _Db, EPointsType _PointsType, QWidget *parent = 0);
  ~CameraForm();
private:
  void Init();

protected:
  /*override */virtual void closeEvent(QCloseEvent* event) override;
  /*override */virtual void showEvent(QShowEvent* event) override;

public:
  void GetThumbnailOk(const char* data, int size);
  void GetThumbnailFail(const QString& msg);
  void CreateChat();
  void CreateDecoder();
  void Disconnected();
  void SetPlay(bool playing);

private:
  void LoadThumbnail();
  bool DrawFrame();
  bool DrawImage();
  void ResizeImage();
  void DrawStatusFrame();

  void OnAutoRefreshTimer();
  void OnPlayStarted();
  void OnPlayFinished(int exitCode, QProcess::ExitStatus exitStatus);

  void OnPlayContinue();

private slots:
  void OnThumbnailImage(QImage image);
  void OnThumbnailDecoded();
  void OnThumbnailFail(const QString& msg);
  void OnThumbnailFrame(bool hasVideo);
  void OnNeedSave();
  void on_actionRefresh_triggered();
  void on_actionScale_triggered(bool checked);
  void on_actionSave_triggered();
  void on_actionCancel_triggered();
  void on_actionPlay_triggered();
  void on_actionStop_triggered();
};
