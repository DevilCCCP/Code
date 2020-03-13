#pragma once

#include <QDir>
#include <QDateTime>

#include <Lib/Dispatcher/ImpD.h>


DefineClassS(BackupA);
DefineClassS(Db);
DefineClassS(DbBackup);

class BackupA: public ImpD
{
  const Db&         mDb;
  DbBackupS         mDbBackup;
  SettingsAS        mVariables;

  QString           mBackupPath;
  bool              mAutoRestore;
  bool              mUseServer;
  QDir              mBackupDir;
  QDir              mBackupStatic;
  QDir              mBackupStaticSub;
  QDir              mBackupLive;
  QDir              mBackupLiveSub;
  QDir              mBackupVar;
  QTime             mDaylyTime;
  qint64            mBackupPeriod;
  QString           mBackupUuid;
  SettingsAS        mBackupSettings;

  QDateTime         mLastBackup;
  QDateTime         mNextBackupPeriodic;
  QDateTime         mNextBackupDayly;
  qint64            mNextBackupMs;

  typedef QMap<QString, QByteArray> HashMap;
  HashMap           mHashMap;
  bool              mBackupReady;
  bool              mRestoreStarted;
  bool              mBackupPrepareWarning;
  bool              mBackupWriteWarning;
  bool              mRestoreWarning;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Backup"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "B"; }
protected:
  /*override */virtual bool LoadSettings(SettingsA* settings) Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
//  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

protected:
  /*new */virtual int GetVersion();
  /*new */virtual void GetDaemons(QStringList& daemonsNames, QStringList& daemonsExes) = 0;
  /*new */virtual void GetStaticTables(int version, QStringList& tables) = 0;
  /*new */virtual void GetLiveTables(int version, QStringList& tables) = 0;
  /*new */virtual bool AfterBackup();
  /*new */virtual bool AfterRestore();

protected:
  QStringList DefaultStaticTables();
  QStringList EventStaticTables();
  QStringList DefaultLiveTables();
  QStringList EventLiveTables();
  QStringList ReportLiveTables();

private:
  void CalcNextBackup();
  bool PrapareBackup();
  bool PrapareRestore();
  bool PrepareSubdir(const QDir& root, const QString& subpath, QDir& dir);
  bool UpdateBackupInfo(const QString& currentUuid);

  QString GetStaticRecentFilename(const QString& table);
  QByteArray GetStaticFileHash(const QString& table);
  void UpdateStaticFileHash(const QString& table, const QByteArray& hash);

  void MakeBackup();
  bool MakeStaticBackup();
  bool MakeStaticTable(const QString& table);
  bool MakeLiveBackup();
  bool MakeLiveTable(const QString& table);
  bool MakeVarBackup();

  bool StartRestore();

public:
  bool Restore(const QString& path);
private:
  bool RestoreStaticBackup();
  bool ClearStaticTable(const QString& table);
  bool RestoreStaticTable(const QString& table, const QString& filename);
  bool RestoreLiveBackup();
  bool RestoreLiveTable(const QString& table);
  bool RestoreVarBackup();

  bool StopDaemons();
  bool StartDaemons();

public:
  BackupA(const Db& _Db);
  /*override */virtual ~BackupA();
};

