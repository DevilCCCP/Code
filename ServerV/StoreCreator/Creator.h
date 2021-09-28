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
  /*override */virtual const char* Name() override { return "Creator"; }
  /*override */virtual const char* ShortName() override { return "C"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;
public:
//  /*override */virtual void Stop() override;

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

