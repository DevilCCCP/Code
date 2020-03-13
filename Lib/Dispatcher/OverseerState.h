#pragma once

#include <Lib/Db/StateInformer.h>
#include <Lib/Include/StateNotifier.h>

#include "Overseer.h"


DefineClassS(OverseerState);
DefineClassS(DbStateNotifier);

class DbStateNotifier: public StateNotifier
{
  OverseerState* mOverseerState;

public:
  /*override */ virtual void NotifyGood() Q_DECL_OVERRIDE;
  /*override */ virtual void NotifyNothing() Q_DECL_OVERRIDE;
  /*override */ virtual void NotifyWarning() Q_DECL_OVERRIDE;
  /*override */ virtual void NotifyError() Q_DECL_OVERRIDE;

public:
  DbStateNotifier(OverseerState* _OverseerState): mOverseerState(_OverseerState) { }
  /*override */ virtual ~DbStateNotifier() { }
};

class OverseerState: public Overseer
{
  StateInformer    mStateInformer;
  DbStateNotifierS mNotifier;

public:
  bool SetState(int state) { return mStateInformer.SetState(state); }
  DbStateNotifierS GetNotifier() const { return mNotifier; }

protected:
  /*override */virtual bool InitReport() Q_DECL_OVERRIDE
  {
    if (!Quiet()) {
      mStateInformer.Init(Id());
    }
    return Overseer::InitReport();
  }

  /*override */virtual bool DoReport() Q_DECL_OVERRIDE
  {
    if (!Quiet()) {
      mStateInformer.UpdateState();
    }
    return Overseer::DoReport();
  }

  /*override */virtual void FinalReport() Q_DECL_OVERRIDE
  {
    if (!Quiet()) {
      mStateInformer.End();
    }
    return Overseer::FinalReport();
  }

public:
  static int ParseMainWithDb(const char* _DaemonName, int argc, char *argv[], OverseerStateS& overseer, DbS& db)
  {
    if (int ret = OpenDb(db)) {
      return ret;
    }

    return ParseMain(_DaemonName, argc, argv, overseer);
  }

  static int ParseMain(const char* _DaemonName, int argc, char *argv[], OverseerStateS& overseer)
  {
    int     id;
    bool    debug;
    bool    detached;
    bool    quiet;
    QString params;
    QString uri;

    if (int ret = ParseCmdLine(argc, argv, id, debug, detached, quiet, params, uri)) {
      return ret;
    }

    overseer = OverseerStateS(new OverseerState(_DaemonName, id, debug, detached, quiet, params, uri));
    return 0;
  }

public:
  OverseerState(const char* _DaemonName, int _Id, bool _Debug, bool _Detached, bool _Quiet, const QString& _Params, const QString& _Uri)
    : Overseer(_DaemonName, _Id, _Debug, _Detached, _Quiet, _Params, _Uri)
  {
    mNotifier.reset(new DbStateNotifier(this));
  }

  /*override */virtual ~OverseerState()
  { }
};


inline void DbStateNotifier::NotifyGood()    { mOverseerState->SetState(1); }
inline void DbStateNotifier::NotifyNothing() { mOverseerState->SetState(0); }
inline void DbStateNotifier::NotifyWarning() { mOverseerState->SetState(-1); }
inline void DbStateNotifier::NotifyError()   { mOverseerState->SetState(-2); }
