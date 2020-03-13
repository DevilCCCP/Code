#pragma once

#include <QThread>
#include <QSet>

#include <Lib/Db/Db.h>
#include <Lib/Db/DbBackup.h>


DefineClassS(DbBackupThread);
DefineClassS(DbBackupTc);
DefineClassS(FormDbBackup);
DefineClassS(DbBackup);

class DbBackupTc: public DbBackupController
{
  FormDbBackup* mHost;

public:
  /*override */virtual bool QueryContinue() Q_DECL_OVERRIDE;
  /*override */virtual void OnPercent(int perc) Q_DECL_OVERRIDE;
  /*override */virtual void OnTable(int index) Q_DECL_OVERRIDE;
  /*override */virtual void OnInfo(const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual void OnError(const QString& text) Q_DECL_OVERRIDE;

public:
  DbBackupTc(FormDbBackup* _Host);
  ~DbBackupTc();
};

class DbBackupThread: public QThread
{
  FormDbBackup* mHost;
  QStringList   mTableOrder;

  bool          mBackup;
  QSet<QString> mTableSet;
  QString       mFile;

public:
  /*override */virtual void run() Q_DECL_OVERRIDE;

public:
  void Start(const QString& file, const QSet<QString>& tableSet, bool backup);

public:
  DbBackupThread(FormDbBackup* _Host, QStringList _TableOrder);
  ~DbBackupThread();
};

