#include <Lib/Db/DbBackup.h>

#include "DbBackupThread.h"
#include "FormDbBackup.h"


bool DbBackupTc::QueryContinue()
{
  return mHost->OnQueryContinue();
}

void DbBackupTc::OnPercent(int perc)
{
  mHost->OnPercent(perc);
}

void DbBackupTc::OnTable(int index)
{
  mHost->OnTable(index);
}

void DbBackupTc::OnInfo(const QString& text)
{
  mHost->OnInfo(text);
}

void DbBackupTc::OnError(const QString& text)
{
  mHost->OnError(text);
}

DbBackupTc::DbBackupTc(FormDbBackup* _Host)
  : mHost(_Host)
{
}

DbBackupTc::~DbBackupTc()
{
}


void DbBackupThread::run()
{
  do {
    QFile file(mFile);
    bool open = mBackup? file.open(QFile::WriteOnly): file.open(QFile::ReadOnly);
    if (!open) {
      mHost->OnError(QString("Open storage file fail"));
      break;
    }

    Db db(false);
    if (!db.OpenDefault()) {
      mHost->OnError(QString("Open DB fail"));
      break;
    }
    if (!db.Connect()) {
      mHost->OnError(QString("Connect DB fail"));
      break;
    }
    DbBackupTcS controller(new DbBackupTc(mHost));
    DbBackup dbBackup(db, mTableOrder, controller);

    QDataStream stream(&file);
    bool backup = mBackup
        ? dbBackup.BackupSet(mTableSet, &stream)
        : dbBackup.RestoreSet(mTableSet, &stream);

    if (backup) {
      mHost->OnInfo(mBackup? QString("Backup done"): QString("Restore done"));
    } else {
      mHost->OnError(mBackup? QString("Backup fail"): QString("Restore fail"));
    }
  } while(false);

  mHost->Done();
}

void DbBackupThread::Start(const QString& file, const QSet<QString>& tableSet, bool backup)
{
  mFile = file;
  mTableSet = tableSet;
  mBackup = backup;

  this->start();
}


DbBackupThread::DbBackupThread(FormDbBackup* _Host, QStringList _TableOrder)
  : mHost(_Host), mTableOrder(_TableOrder)
{
}

DbBackupThread::~DbBackupThread()
{
}
