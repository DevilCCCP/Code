#pragma once

#include <QElapsedTimer>

#include <Lib/Dispatcher/Imp.h>
#include <Lib/Include/License_h.h>

DefineClassS(Db);
DefineClassS(Storage);
DefineClassS(Creator);

class Creator: public Imp
{
  DbS           mDb;
  StorageS      mStorage;

  QElapsedTimer mWorkTimer;
  int           mLastPeriodMs;
  bool          mDbOk;
  bool          mFileOk;

  LICENSE_HEADER
  ;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Creator"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "C"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

private:
  bool CreateStorageTable();
  bool CreateStorageFile();
  bool ExecStringScript(const QString& script);
  bool ExecFileScript(const QString& filename, const QString& username);
  bool UpdateStorageStatus();

public:
  Creator();
  /*override */virtual ~Creator();
};

