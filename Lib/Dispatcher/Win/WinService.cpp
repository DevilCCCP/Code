#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

#include <Lib/Log/Log.h>

#include "WinService.h"


static WinService* gWinService = nullptr;
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPWSTR*);

class ServiceConfig
{
  QString     mServiceName;
  SC_HANDLE   mScManager;
  SC_HANDLE   mService;
  DWORD       mLastError;

public:
  ServiceConfig(const QString& _ServiceName, DWORD access)
    : mServiceName(_ServiceName), mScManager(nullptr), mService(nullptr)
  {
    mScManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (mScManager) {
      mService = OpenService(mScManager, (const wchar_t*)mServiceName.utf16(), access);
      if (!mService) {
        mLastError = GetLastError();
      }
    } else {
      mService = nullptr;
      mLastError = GetLastError();
    }
  }

  ~ServiceConfig()
  {
    if (mService) {
      CloseServiceHandle(mService);
    }
    if (mScManager) {
      CloseServiceHandle(mScManager);
    }
  }


  QString Error()
  {
    if (!mScManager) {
      return QString("OpenSCManager fail (code: %1)").arg(mLastError);
    } else if (!mService) {
      return QString("OpenService fail (code: %1)").arg(mLastError);
    } else {
      return "No errors";
    }
  }

  bool IsOk()
  { return !!mService; }

  bool IsError()
  { return !mService; }

  SC_HANDLE operator*()
  { return mService; }
};

bool WinService::Install()
{
  TCHAR path[MAX_PATH];
  if (!GetModuleFileName(NULL, path, MAX_PATH)) {
    Log.Error(QString("GetModuleFileName fail (last error code: %1)").arg(GetLastError()));
    return false;
  }

  SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (!scManager) {
    Log.Error(QString("OpenSCManager fail (last error code: %1)").arg(GetLastError()));
    return false;
  }

  DWORD desiredAccess;
  if ((ServiceCapabilities() & ePauseContinue) == ePauseContinue) {
    desiredAccess = SERVICE_ALL_ACCESS;
  } else {
    desiredAccess = SERVICE_ALL_ACCESS ^ SERVICE_PAUSE_CONTINUE;
  }
  // Create the service
  SC_HANDLE service = CreateService(
    scManager,                                                            // SCM database
    (const wchar_t*)mName.utf16(),                                        // name of service
    (const wchar_t*)mViewname.utf16(),                                    // service name to display
    desiredAccess,                                                        // desired access
    SERVICE_WIN32_OWN_PROCESS,                                            // service type
    SERVICE_AUTO_START,                                                   // start type
    SERVICE_ERROR_NORMAL,                                                 // error control type
    path,                                                                 // path to service's binary
    NULL,                                                                 // no load ordering group
    NULL,                                                                 // no tag identifier
    NULL,                                                                 // no dependencies
    NULL,                                                                 // LocalSystem account
    NULL);                                                                // no password

  if (service == NULL) {
    Log.Error(QString("Create service fail (last error code: %1)").arg(GetLastError()));
    CloseServiceHandle(scManager);
    return false;
  }
  SERVICE_DESCRIPTION sd = { const_cast<LPWSTR>((const wchar_t*)mDescription.utf16()) };
  ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &sd);

  const int kActionCount = 1;
  SC_ACTION action[kActionCount] = { { SC_ACTION_RESTART, 0 } };
  SERVICE_FAILURE_ACTIONS actions = { 0, nullptr, nullptr, kActionCount, action };
  ChangeServiceConfig2(service, SERVICE_CONFIG_FAILURE_ACTIONS, &actions);

  CloseServiceHandle(service);
  CloseServiceHandle(scManager);
  return true;
}

bool WinService::Uninstall()
{
  ServiceConfig config(mName, SC_MANAGER_ALL_ACCESS);
  if (config.IsOk()) {
    return DeleteService(*config);
  } else {
    Log.Error(config.Error());
    return false;
  }
}

bool WinService::Start()
{
  ServiceConfig config(mName, SERVICE_QUERY_STATUS | SERVICE_START);
  if (config.IsOk()) {
    return StartService(*config, 0, NULL);
  } else {
    Log.Error(config.Error());
    return false;
  }
}

bool WinService::Stop()
{
  ServiceConfig config(mName, SERVICE_QUERY_STATUS | SERVICE_STOP);

  if (config.IsOk()) {
    SERVICE_STATUS ss = {};
    ss.dwWaitHint = StopTime();
    return ControlService(*config, SERVICE_CONTROL_STOP, &ss);
  } else {
    Log.Error(config.Error());
    return false;
  }
}

bool WinService::Restart()
{
  ServiceConfig config(mName, SERVICE_QUERY_STATUS | SERVICE_STOP | SERVICE_START);

  if (config.IsOk()) {
    SERVICE_STATUS ss = {};
    ss.dwWaitHint = StopTime();
    return ControlService(*config, SERVICE_CONTROL_STOP, &ss) && StartService(*config, 0, NULL);
  } else {
    Log.Error(config.Error());
    return false;
  }
}

bool WinService::Run()
{
  const SERVICE_TABLE_ENTRY entry[] = {
    { const_cast<LPWSTR>((const wchar_t*)mName.utf16()), ::SvcMain },
    { NULL, NULL }
  };

  if (!StartServiceCtrlDispatcher(entry)) {
    if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
      ConsoleMain();
    }
  }
  return true;
}

void WinService::SetStatus(DWORD currentState, DWORD exitCode, DWORD waitHint)
{
  mLastState = mSvcStatus.dwCurrentState;

  // Fill in the SERVICE_STATUS structure.
  mSvcStatus.dwCurrentState = currentState;
  mSvcStatus.dwWin32ExitCode = exitCode;
  mSvcStatus.dwWaitHint = waitHint;
  UpdateStatus();
}

void WinService::RevertStatus()
{
  mSvcStatus.dwCurrentState = mLastState;
  UpdateStatus();
}

void WinService::UpdateStatus()
{
  static DWORD checkPoint = 1;

  DWORD currentState = mSvcStatus.dwCurrentState;

  if (currentState == SERVICE_START_PENDING) {
    mSvcStatus.dwControlsAccepted = 0;
  } else {
    mSvcStatus.dwControlsAccepted = 0;
    if (ServiceCapabilities() & (ePauseContinue)) {
      mSvcStatus.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
    }
    if (ServiceCapabilities() & eStop) {
      mSvcStatus.dwControlsAccepted |= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    }
  }

  if (currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED) {
    mSvcStatus.dwCheckPoint = 0;
  } else {
    mSvcStatus.dwCheckPoint = checkPoint++;
  }

  // Report the status of the service to the SCM.
  SetServiceStatus(mSvcStatusHandle, &mSvcStatus);
}

int WinService::InitTime()
{
  return 1000;
}

int WinService::PauseTime()
{
  return 1000;
}

int WinService::ContinueTime()
{
  return 2000;
}

int WinService::StopTime()
{
  return 10000;
}

WinService::EServiceCapabilities WinService::ServiceCapabilities()
{
  return eStop;
}

bool WinService::DoPause()
{
  return false;
}

bool WinService::DoContinue()
{
  return false;
}

bool WinService::DoStop()
{
  Log.Info(QString("Doing stop (PID %1)").arg(GetCurrentProcessId()));
  mDispatcher->Stop();
  while (!mStopped) {
    Sleep(10);
  }
  return true;
}

void WinService::SvcMain()
{
  mSvcStatusHandle = RegisterServiceCtrlHandler((const wchar_t*)mName.utf16(), SvcCtrlHandler);

  // These SERVICE_STATUS members remain as set here
  mSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  mSvcStatus.dwServiceSpecificExitCode = 0;

  SetStatus(SERVICE_START_PENDING, NO_ERROR, InitTime());

  if (mDispatcher->Init(false)) {
    SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
    mStopped = false;
    int retCode = mDispatcher->Run();
    SetStatus(SERVICE_STOPPED, retCode, 0);
    mStopped = true;
  } else {
    SetStatus(SERVICE_STOPPED, ERROR_BAD_ENVIRONMENT, 0);
  }
}

void WinService::ConsoleMain()
{
  if (mDispatcher->Init(true)) {
    mDispatcher->Run();
  }
}

void WinService::CtrlMain()
{
  SvcMain();
}

void WinService::CtrlPause()
{
  if (ServiceCapabilities() & ePause)
  {
    Log.Info("Process WinService 'Pause' command");
    SetStatus(SERVICE_PAUSE_PENDING, NO_ERROR, PauseTime());

    if (DoPause()) {
      SetStatus(SERVICE_PAUSED, NO_ERROR, 0);
    } else {
      RevertStatus();
    }
  }
}

void WinService::CtrlContinue()
{
  if (ServiceCapabilities() & eContinue)
  {
    Log.Info("Process WinService 'Continue' command");
    SetStatus(SERVICE_CONTINUE_PENDING, NO_ERROR, ContinueTime());

    if (DoContinue()) {
      SetStatus(SERVICE_RUNNING, NO_ERROR, 0);
    } else {
      RevertStatus();
    }
  }
}

void WinService::CtrlStop()
{
  if (ServiceCapabilities() & eStop)
  {
    Log.Info("Process WinService 'Stop' command");
    SetStatus(SERVICE_STOP_PENDING, NO_ERROR, StopTime());

    if (DoStop()) {
    } else {
      RevertStatus();
    }
  }
}

void WinService::CtrlInterrogate()
{
  SetServiceStatus(mSvcStatusHandle, &mSvcStatus);
}

QString ControlCodeToString(DWORD code)
{
  switch (code) {
  case SERVICE_CONTROL_STOP:                   return "SERVICE_CONTROL_STOP";
  case SERVICE_CONTROL_PAUSE:                  return "SERVICE_CONTROL_PAUSE";
  case SERVICE_CONTROL_CONTINUE:               return "SERVICE_CONTROL_CONTINUE";
  case SERVICE_CONTROL_INTERROGATE:            return "SERVICE_CONTROL_INTERROGATE";
  case SERVICE_CONTROL_SHUTDOWN:               return "SERVICE_CONTROL_SHUTDOWN";
  case SERVICE_CONTROL_PARAMCHANGE:            return "SERVICE_CONTROL_PARAMCHANGE";
  case SERVICE_CONTROL_NETBINDADD:             return "SERVICE_CONTROL_NETBINDADD";
  case SERVICE_CONTROL_NETBINDREMOVE:          return "SERVICE_CONTROL_NETBINDREMOVE";
  case SERVICE_CONTROL_NETBINDENABLE:          return "SERVICE_CONTROL_NETBINDENABLE";
  case SERVICE_CONTROL_NETBINDDISABLE:         return "SERVICE_CONTROL_NETBINDDISABLE";
  case SERVICE_CONTROL_DEVICEEVENT:            return "SERVICE_CONTROL_DEVICEEVENT";
  case SERVICE_CONTROL_HARDWAREPROFILECHANGE:  return "SERVICE_CONTROL_HARDWAREPROFILECHANGE";
  case SERVICE_CONTROL_POWEREVENT:             return "SERVICE_CONTROL_POWEREVENT";
  case SERVICE_CONTROL_SESSIONCHANGE:          return "SERVICE_CONTROL_SESSIONCHANGE";
#ifdef SERVICE_CONTROL_PRESHUTDOWN
  case SERVICE_CONTROL_PRESHUTDOWN:            return "SERVICE_CONTROL_PRESHUTDOWN";
  case SERVICE_CONTROL_TIMECHANGE:             return "SERVICE_CONTROL_TIMECHANGE";
  case SERVICE_CONTROL_TRIGGEREVENT:           return "SERVICE_CONTROL_TRIGGEREVENT";
#endif
  default:                                     return QString::number(code);
  }
}

VOID WINAPI SvcCtrlHandler(DWORD ctrl)
{
  if (!gWinService) {
    Log.Warning(QString("Service Ctrl handle %1, but service is uninitialized").arg(ControlCodeToString(ctrl)));
    return;
  }
  Log.Info(QString("Service Ctrl handle %1").arg(ControlCodeToString(ctrl)));
  switch (ctrl)
  {
  case SERVICE_CONTROL_PAUSE:       return gWinService->CtrlPause();
  case SERVICE_CONTROL_CONTINUE:    return gWinService->CtrlContinue();
  case SERVICE_CONTROL_STOP:        return gWinService->CtrlStop();
  case SERVICE_CONTROL_SHUTDOWN:    return gWinService->CtrlStop();
  case SERVICE_CONTROL_INTERROGATE: return gWinService->CtrlInterrogate();
  default: break;
  }
}

VOID WINAPI SvcMain(DWORD, LPWSTR*)
{
  if (!gWinService) {
    Log.Warning(QString("Service SvcMain entered, but service is uninitialized"));
    return;
  }

  int argc = 0;
  char** argv = nullptr;
  QCoreApplication app(argc, argv);
  Log.SetFileLogging();
  Log.Info("Running Windows service");
  return gWinService->CtrlMain();
}

WinService::WinService(const QString &_Name, const QString &_Viewname, const QString &_Description, Dispatcher *_Dispatcher)
  : mName(_Name), mViewname(_Viewname), mDescription(_Description)
  , mStopped(true), mDispatcher(_Dispatcher), mSvcStatusHandle(nullptr), mLastState(SERVICE_STOPPED)
{
  gWinService = this;
}

WinService::~WinService()
{ }
