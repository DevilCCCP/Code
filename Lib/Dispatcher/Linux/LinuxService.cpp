#include <QDir>
#include <QProcess>
#include <QFileInfo>
//#include <unistd.h>

#include <Lib/Log/Log.h>
#include <Local/ModuleNames.h>

#include "LinuxService.h"


const char kUnitName[] = MAKE_STRING(PROGRAM_ABBR);
static volatile LinuxService* gLinuxService = nullptr;
static volatile bool gStopped = 0;

class ServiceConfig
{
  QString  mServiceName;

public:
  ServiceConfig(const QString& _ServiceName)
    : mServiceName(_ServiceName)
  {
  }

  ~ServiceConfig()
  {
  }


  static bool ExecuteUpdate()
  {
    return QProcess::startDetached(QString("systemctl daemon-reload"));
  }

  bool ExecuteUserCommand(const QString& command)
  {
    return QProcess::startDetached(QString("sudo service %2 %1").arg(command).arg(mServiceName));
  }

  bool ExecuteCommand(const QString& command)
  {
    return QProcess::startDetached(QString("systemctl %1 %2").arg(command).arg(mServiceName));
  }
};


bool LinuxService::Install()
{
  QFile serviceCfg(QString("/etc/systemd/system/%1.service").arg(mName));
  if (serviceCfg.exists()) {
    Log.Warning(QString("%1 is already installed").arg(mName));
    return false;
  }

  if (!serviceCfg.open(QFile::WriteOnly)) {
    Log.Warning(QString("%1 install fail; %2").arg(mName).arg(serviceCfg.errorString()));
    return false;
  }
  serviceCfg.write(QString("[Unit]\n"
                           "Description=%2\n"
                           "After=network.target\n"
                           "Requires=postgresql.service\n"
                           "After=postgresql.service\n"
                           "\n"
                           "[Service]\n"
                           "Type=simple\n"
                           "WorkingDirectory=/opt/%1\n"
                           "Restart=always\n"
                           "\n"
                           "User=%1\n"
                           "Group=%1\n"
                           "\n"
                           "Environment=XAUTHORITY=/tmp/.Xauth\n"
                           "Environment=LD_LIBRARY_PATH=/opt/%1/\n"
                           "Environment=DISPLAY=:0\n"
                           "\n"
                           "ExecStart=/opt/%1/%3.exe run\n"
                           "TimeoutStartSec=10\n"
                           "TimeoutStopSec=7\n"
                           "\n"
                           "[Install]\n"
                           "WantedBy=multi-user.target\n")
                   .arg(kUnitName).arg(mViewname).arg(mName).toUtf8());
  serviceCfg.close();

  ServiceConfig cfg(mName);
  if (!cfg.ExecuteCommand("enable")) {
    return false;
  }
  QThread::msleep(500);

  if (!cfg.ExecuteUpdate()) {
    return false;
  }
  return true;
}

bool LinuxService::Uninstall()
{
  QFile serviceCfg(QString("/etc/systemd/system/%1.service").arg(mName));
  if (!serviceCfg.exists()) {
    Log.Warning(QString("%1 is not installed").arg(mName));
    return false;
  }

  ServiceConfig cfg(mName);
  if (!cfg.ExecuteCommand("disable")) {
    return false;
  }
  QThread::msleep(500);

  if (!cfg.ExecuteUpdate()) {
    return false;
  }

  if (!serviceCfg.remove()) {
    Log.Warning(QString("%1 uninstall fail; %2").arg(mName).arg(serviceCfg.errorString()));
    return false;
  }
  return true;
}

bool LinuxService::Start()
{
  ServiceConfig cfg(mName);
  return cfg.ExecuteUserCommand("start");
}

bool LinuxService::Stop()
{
  ServiceConfig cfg(mName);
  return cfg.ExecuteUserCommand("stop");
}

bool LinuxService::Restart()
{
  ServiceConfig cfg(mName);
  return cfg.ExecuteUserCommand("restart");
}

bool LinuxService::Run()
{
  Log.SetFileLogging();
  Log.Info("Running Linux service");
//  daemon(1, 0);
  if (mDispatcher->Init(false)) {
    mDispatcher->SetConsoleBreak();
    return mDispatcher->Run() == 0;
  }
  return false;
}


LinuxService::LinuxService(const QString &_Name, const QString &_Viewname, const QString &_Description, Dispatcher *_Dispatcher)
  : mName(_Name), mViewname(_Viewname), mDescription(_Description)
  , mDispatcher(_Dispatcher)
{
  gLinuxService = this;
}

LinuxService::~LinuxService()
{ }
