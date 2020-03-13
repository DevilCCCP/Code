#pragma once

#include <QDataStream>
#include <QStringList>
#include <QSet>

#include <Lib/Db/Db.h>


DefineClassS(DbBackup);
DefineClassS(DbBackupController);

class DbBackupController
{
  PROPERTY_GET_SET(bool, Stop)
  ;
public:
  /*new */virtual bool QueryContinue();
  /*new */virtual void OnPercent(int perc);
  /*new */virtual void OnTable(int index);
  /*new */virtual void OnInfo(const QString& text);
  /*new */virtual void OnError(const QString& text);

public:
  DbBackupController(): mStop(false) { }
  virtual ~DbBackupController() { }
};

class DbBackup
{
  const Db&     mDb;
  PROPERTY_GET_SET(QString,     SchemaName)
  PROPERTY_GET_SET(QStringList, TableOrderList)
  PROPERTY_SGET(DbBackupController)

  int           mTableIndex;
  ;
public:
  bool BackupSet(const QSet<QString>& tableSet, QDataStream* writer);
  bool BackupList(const QStringList& tableList, QDataStream* writer);
  bool BackupOne(const QString& tableName, QDataStream* writer, qint64* topId = nullptr);
private:
  bool BackupTable(const QString& tableName, QDataStream* writer, qint64* topId = nullptr);

public:
  bool RestoreSet(const QSet<QString>& tableSet, QDataStream* reader);
  bool RestoreAll(QDataStream* reader);
  bool RestoreOne(const QString& tableName, QDataStream* reader, bool clear = true);
  bool ClearOne(const QString& tableName);
private:
  bool RestoreBegin(QDataStream* reader, const QByteArray& allowTypes, QStringList& tables);
  bool RestoreTable(const QString& tableName, QDataStream* reader, bool skip, bool clear = true);

public:
  DbBackup(const Db& _Db, const QStringList& _TableOrderList = QStringList(), const DbBackupControllerS& _DbBackupController = DbBackupControllerS(new DbBackupController));
};

