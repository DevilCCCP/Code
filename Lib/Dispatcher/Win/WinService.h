#pragma once

#include <QString>
#include <windows.h>

#include "Dispatcher.h"


class WinService
{
  QString                 mName;
  QString                 mViewname;
  QString                 mDescription;

  volatile bool           mStopped;
  Dispatcher*             mDispatcher;
  SERVICE_STATUS_HANDLE   mSvcStatusHandle;
  SERVICE_STATUS          mSvcStatus;
  volatile DWORD          mLastState;

protected:
  enum EServiceCapabilities {
    eNone     = 0,
    ePause    = 1 << 0,
    eContinue = 1 << 1,
    eStop     = 1 << 2,
    eIllegal  = 1 << 3,

    ePauseContinue = ePause | eContinue
  };

public:
  bool Install();
  bool Uninstall();
  bool Start();
  bool Stop();
  bool Restart();
  bool Run();

private:
  void SetStatus(DWORD currentState, DWORD exitCode, DWORD waitHint);
  void RevertStatus();
  void UpdateStatus();

protected:
  /*new */virtual int InitTime();
  /*new */virtual int PauseTime();
  /*new */virtual int ContinueTime();
  /*new */virtual int StopTime();

  /*new */virtual EServiceCapabilities ServiceCapabilities();
  /*new */virtual bool DoPause();
  /*new */virtual bool DoContinue();
  /*new */virtual bool DoStop();

  /*new */virtual void SvcMain();
  /*new */virtual void ConsoleMain();

public: /*internal*/
  void CtrlMain();
  void CtrlPause();
  void CtrlContinue();
  void CtrlStop();
  void CtrlRestart();
  void CtrlInterrogate();

public:
  WinService(const QString& _Name, const QString& _Viewname, const QString& _Description, Dispatcher* _Dispatcher);
  ~WinService();
};

