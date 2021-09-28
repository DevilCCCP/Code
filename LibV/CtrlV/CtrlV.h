#pragma once

#include <QMap>
#include <QRect>
#include <QPoint>
#include <QSharedMemory>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Player/ArmState.h>


enum EArmState {
  eNormal,
  ePrime,
  eFullscreen,
  eModeIllegal
};

inline const char* EArmState_ToString(EArmState state)
{
  switch (state) {
  case eNormal:      return "Normal";
  case ePrime:       return "Prime";
  case eFullscreen:  return "Fullscreen";
  case eModeIllegal: return "ModeIllegal";
  default:           return "<illegal>";
  }
}

DefineClassS(CtrlV);
DefineClassS(Db);
DefineClassS(BackWnd);
DefineClassS(ToolWndA);

class CtrlV: public Imp
{
  DbS               mDb;
  ToolWndAS         mToolWnd;
  BackWndS          mBackWnd;
  QRect             mScreenRect;
  bool              mCtrlTools;

  EArmState         mArmMode;
  int               mSelectedPlayerId;
  int               mPrimePlayerId;
  QMap<int, QRect>  mPlayerWindows;
  int               mLayoutCameras;
  bool              mPrepared;

  QSharedMemory     mStateShmem;
  ArmState*         mArmState;

  Q_OBJECT

public:
  /*override */virtual const char* Name() override { return "Ctrl window"; }
  /*override */virtual const char* ShortName() override { return "C"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;
public:
  /*override */virtual void Stop() override;

public:
  void RegisterPlayerType(int id, bool isPrime);
  void RegisterPlayerTopLeft(int id, const QPoint& point);
  void RegisterPlayerBottomRight(int id, const QPoint& point);
  void MovePlayer(int id);

public: /*internal*/
  void DoExit();
  void DoLayout(ELayoutType id, int count, int monitor, int cameraGroup, qint64 ts = 0);
  void DoSwitchDesktop();
  void DoSwitchOther();

private:
  bool Prepare();
  bool InitLayoutsInfo();
  bool InitMonitorsInfo();
  void ApplyLayout();
  void ApplyCamera();
  void UpdatePlayerAll();
  void UpdatePlayer(int id);

signals:
  void OnStop();

public:
  CtrlV(bool _CtrlTools, int _Monitor = 0);
  ~CtrlV();
};

