#pragma once

#include <QMap>
#include <QRect>
#include <QPoint>
#include <QSharedMemory>

#include <ArmV/ArmD/ArmState.h>
#include <Lib/Db/Db.h>


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

DefineClassS(BackWnd);
DefineClassS(CtrlWnd);

class CtrlWnd
{
  const Db&         mDb;
  ObjectTableS      mObjectTable;
  ArmMonitorsTableS mArmMonitorsTable;

  BackWndS          mBackWnd;
  QRect             mScreenRect;

  EArmState         mArmMode;
  int               mSelectedPlayerId;
  int               mPrimePlayerId;
  QMap<int, QRect>  mPlayerWindows;

  QSharedMemory     mStateShmem;
  ArmState*         mArmState;

public:
  void Init(Overseer* overseer);

public:
  void RegisterPlayerType(int id, bool isPrime);
  void RegisterPlayerTopLeft(int id, const QPoint& point);
  void RegisterPlayerBottomRight(int id, const QPoint& point);
  void MovePlayer(int id);

public: /*internal*/
  void DoExit();
  void DoLayout(int id, int count);
  void DoSwitchDesktop();
  void DoSwitchOther();

private:
  void UpdatePlayerAll();
  void UpdatePlayer(int id);

  void UpdateMonitorsInfo();

public:
  CtrlWnd(const Db& _Db);
  ~CtrlWnd();
};

