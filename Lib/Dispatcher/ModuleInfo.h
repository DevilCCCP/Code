#pragma once

#include <QString>
#include <QStringList>
#include <QProcess>
#include <qsystemdetection.h>

#include <Lib/Include/Common.h>


DefineClassS(QProcess);

struct ModuleLoadInfo
{
  int         Id;
  QString     Path;
  QStringList Params;
};

struct ModuleInfo
{
  int           Id;
  int           Index;
  QString       Path;
  QStringList   Params;

#ifdef Q_OS_WIN32
  unsigned long Pid;
#else
  QProcessS     Process;
#endif
  qint64        LastCrush;
  int           CrushCount;
  bool          TimeoutWarned;
  bool          StartWarned;

  void Rewrite(const ModuleLoadInfo& _ModuleLoadInfo)
  {
    Path = _ModuleLoadInfo.Path;
    Params = _ModuleLoadInfo.Params;

#ifdef Q_OS_WIN32
    Pid = 0;
#else
    Process.reset(new QProcess());
#endif
    Clear();
  }

  void Update(const ModuleLoadInfo& _ModuleLoadInfo)
  {
    Path = _ModuleLoadInfo.Path;
    Params = _ModuleLoadInfo.Params;
  }

  void Clear()
  {
    CrushCount = 0;
    TimeoutWarned = false;
    StartWarned = false;
  }

  ModuleInfo()
    : Id(-1), Index(-1)
  { Clear(); }
  ModuleInfo(int _Id, int _Index, const QString& _Path, const QStringList& _Params)
    : Id(_Id), Index(_Index), Path(_Path), Params(_Params)
  { Clear(); }
  ModuleInfo(const ModuleLoadInfo& _ModuleLoadInfo, int _Index)
    : Id(_ModuleLoadInfo.Id), Index(_Index)
  { Rewrite(_ModuleLoadInfo); }
};

