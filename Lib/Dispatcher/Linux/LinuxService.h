#pragma once

#include <QString>

#include "Dispatcher.h"


class LinuxService
{
  QString                 mName;
  QString                 mViewname;
  QString                 mDescription;

  Dispatcher*             mDispatcher;

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

public:
  LinuxService(const QString& _Name, const QString& _Viewname, const QString& _Description, Dispatcher* _Dispatcher);
  ~LinuxService();
};

