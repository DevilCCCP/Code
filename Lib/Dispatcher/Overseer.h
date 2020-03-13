#pragma once

#include <QSharedPointer>
#include <QSharedMemory>

#include <Lib/Ctrl/CtrlManager.h>
#include "MainInfo.h"


DefineClassS(Db);
DefineClassS(Overseer);

// Модуль отвечает за взаимодействие с диспетчером на стороне рабочего модуля
class Overseer: public CtrlManager
{
  const char*   mDaemonName;
  const int     mId;
  const bool    mDebug;
  const QString mParams;
  const QString mUri;
  const bool    mDetached;
  const bool    mQuiet;

  QSharedMemory mMainShmem;
  ProcessInfo*  mProcessInfo;

public:
  int            Id()       const { return mId; }
  const QString& Params()   const { return mParams; }
  const QString& Uri()      const { return mUri; }
  bool           Detached() const { return mDetached; }
  bool           Quiet()    const { return mQuiet; }

public:
  /*override */virtual void Stop() Q_DECL_OVERRIDE;

protected:
  /// return continue or end work
  /*override */virtual bool InitReport() Q_DECL_OVERRIDE;
  /*override */virtual bool DoReport() Q_DECL_OVERRIDE;
  /*override */virtual void FinalReport() Q_DECL_OVERRIDE;
  /*override */virtual void CriticalFail() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool IsPublicStats() Q_DECL_OVERRIDE;
  /*override */virtual void PublishStats(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList) Q_DECL_OVERRIDE;

public:
  void Restart();
  void Done();

private:
  bool OpenShmem();
  void ChangeStatus(EProcStatus newStatus);

public:
  static int ParseMainWithDb(const char* _DaemonName, int argc, char *argv[], OverseerS& overseer, DbS& db);
  static int ParseMain(const char* _DaemonName, int argc, char *argv[], OverseerS& overseer);
  static int OpenDb(DbS& db);
protected:
  static int ParseCmdLine(int argc, char *argv[], int& id, bool& debug, bool& detached, bool& quiet, QString& params, QString& uri);

public:
  template<typename ImpT>
  static int RunWithOneImp(const char* _DaemonName, int argc, char *argv[])
  {
    OverseerS overseer;
    if (!ParseMain(_DaemonName, argc, argv, overseer)) {
      return -2001;
    }
    QSharedPointer<ImpT> _Imp(new ImpT);
    overseer->RegisterWorker(_Imp);
    return overseer->Run();
  }

  Overseer(const char* _DaemonName, int _Id, bool _Debug, bool _Detached, bool _Quiet, const QString& _Params, const QString& _Uri);
  /*override */virtual ~Overseer();
};

