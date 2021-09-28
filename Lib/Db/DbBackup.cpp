#include <Lib/Log/Log.h>

#include "DbBackup.h"


const int kVersion = 1;
const int kRecordsAtOnce = 200;

bool DbBackupController::QueryContinue()
{
  return true;
}

void DbBackupController::OnPercent(int perc)
{
  Q_UNUSED(perc);
}

void DbBackupController::OnTable(int index)
{
  Q_UNUSED(index);
}

void DbBackupController::OnInfo(const QString& text)
{
  Log.Info(QString("Db backup: %1").arg(text));
}

void DbBackupController::OnError(const QString& text)
{
  Log.Warning(QString("Db backup: %1").arg(text));
}


bool DbBackup::BackupSet(const QSet<QString>& tableSet, QDataStream* writer)
{
  *writer << QByteArray("Set");
  *writer << kVersion;
  foreach (const QString& tableName, tableSet) {
    *writer << tableName;
  }
  *writer << QString();
  if (writer->status() != QDataStream::Ok) {
    return false;
  }

  mTableIndex = 0;

  for (auto itr = mTableOrderList.begin(); itr != mTableOrderList.end(); itr++) {
    const QString& nextTable = *itr;
    if (tableSet.contains(nextTable)) {
      if (!BackupTable(nextTable, writer)) {
        return false;
      }
    }
  }

  mDbBackupController->OnTable(mTableIndex);
  return true;
}

bool DbBackup::BackupList(const QStringList& tableList, QDataStream* writer)
{
  *writer << QByteArray("List");
  *writer << kVersion;
  foreach (const QString& tableName, tableList) {
    *writer << tableName;
  }
  *writer << QString();
  if (writer->status() != QDataStream::Ok) {
    return false;
  }

  mTableIndex = 0;

  QStringList backupList = tableList;
  if (!mTableOrderList.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14,0)
    QSet<QString> backupSet(backupList.begin(), backupList.end());
#else
    QSet<QString> backupSet = QSet<QString>::fromList(backupList);
#endif
    for (auto itr = mTableOrderList.begin(); itr != mTableOrderList.end(); itr++) {
      const QString& nextTable = *itr;
      if (backupSet.remove(nextTable)) {
        if (!BackupTable(nextTable, writer)) {
          return false;
        }
      }
    }

    for (auto itr = backupList.begin(); itr != backupList.end(); ) {
      const QString& nextTable = *itr;
      if (backupSet.contains(nextTable)) {
        itr++;
      } else {
        itr = backupList.erase(itr);
      }
    }
  }

  for (auto itr = backupList.begin(); itr != backupList.end(); ) {
    const QString& nextTable = *itr;
    if (!BackupTable(nextTable, writer)) {
      return false;
    }
  }

  mDbBackupController->OnTable(mTableIndex);
  return true;
}

bool DbBackup::BackupOne(const QString& tableName, QDataStream* writer, qint64* topId)
{
  *writer << QByteArray("One");
  *writer << kVersion;
  *writer << tableName;
  *writer << QString();
  if (writer->status() != QDataStream::Ok) {
    return false;
  }
  return BackupTable(tableName, writer, topId);
}

bool DbBackup::BackupTable(const QString& tableName, QDataStream* writer, qint64* topId)
{
  mDbBackupController->OnInfo(QString("Begin backup table '%1'").arg(tableName));
  mDbBackupController->OnPercent(0);
  mDbBackupController->OnTable(mTableIndex);

  *writer << tableName;
  if (writer->status() != QDataStream::Ok) {
    return false;
  }

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT column_name, data_type FROM information_schema.columns"
                     " WHERE table_name = %1 AND table_schema = %2")
             .arg(ToSql(tableName)).arg(ToSql(mSchemaName)));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  bool hasId = false;
  QStringList columns;
  while (q->next()) {
    QString name = q->value(0).toString();
    if (name != "_id") {
      columns.append(name);
    } else {
      hasId = true;
    }
  }
  if (!hasId) {
    mDbBackupController->OnError(QString("column '_id' not found for table '%1'").arg(tableName));
    return false;
  }
  *writer << (int)columns.size();
  for (int i = 0; i < columns.size(); i++) {
    *writer << columns.at(i);
  }
  if (writer->status() != QDataStream::Ok) {
    return false;
  }

  q->prepare(QString("SELECT COUNT(_id) FROM %1;").arg(tableName));
  if (!mDb.ExecuteQuery(q) || !q->next()) {
    mDbBackupController->OnError(QString("Retrive records count fail for table '%1'").arg(tableName));
    return false;
  }
  qint64 currentRecords = 0;
  qint64 totalRecords = q->value(0).toLongLong();
  qint64 denumRecords = totalRecords/100 + 1;
  int lastPercent = 0;
  *writer << totalRecords;
  if (writer->status() != QDataStream::Ok) {
    return false;
  }

  qint64 lastId = topId? *topId: 0;
  if (!lastId) {
    q->prepare(QString("SELECT _id FROM %1 ORDER BY _id LIMIT 1").arg(tableName));

    if (!mDb.ExecuteQuery(q)) {
      mDbBackupController->OnError(QString("SELECT '_id' fail for table '%1'").arg(tableName));
      return false;
    }

    lastId = (q->next())? q->value(0).toLongLong() - 1: 0;
  }

  forever {
    int currentPercent = (int)(currentRecords / denumRecords);
    if (currentPercent != lastPercent) {
      mDbBackupController->OnPercent(currentPercent);
      lastPercent = currentPercent;
    }
    if (!mDbBackupController->QueryContinue()) {
      mDbBackupController->OnError(QString("User break"));
      return false;
    }

    qint64 nextId = lastId;
    q->prepare(QString("SELECT _id, %2 FROM %1 WHERE _id > %3 ORDER BY _id LIMIT %4")
               .arg(tableName).arg(columns.join(',')).arg(lastId).arg(kRecordsAtOnce));
    if (!mDb.ExecuteQuery(q)) {
      mDbBackupController->OnError(QString("SELECT fail for table '%1'").arg(tableName));
      return false;
    }
    while (q->next()) {
      nextId = q->value(0).toLongLong();
      if (nextId == 0) {
        *writer << (qint64)0;
      }
      *writer << nextId;
      for (int i = 1; i <= columns.size(); i++) {
        *writer << q->value(i);
      }
      if (writer->status() != QDataStream::Ok) {
        return false;
      }
      currentRecords++;
    }
    if (nextId == lastId) {
      break;
    }
    lastId = nextId;
  }

  *writer << (qint64)0 << (qint64)-1;
  if (writer->status() != QDataStream::Ok) {
    return false;
  }

  if (topId) {
    *topId = lastId;
  }

  mDbBackupController->OnInfo(QString("Done backup table '%1'").arg(tableName));
  mTableIndex++;
  return true;
}

bool DbBackup::RestoreSet(const QSet<QString>& tableSet, QDataStream* reader)
{
  QStringList tables;
  if (!RestoreBegin(reader, "Set", tables)) {
    return false;
  }

  mTableIndex = 0;

  while (!reader->atEnd()) {
    QString tableName;
    *reader >> tableName;
    bool skip = !tableSet.contains(tableName);
    if (!RestoreTable(tableName, reader, skip)) {
      return false;
    }
  }

  mDbBackupController->OnTable(mTableIndex);
  return true;
}

bool DbBackup::RestoreAll(QDataStream* reader)
{
  QStringList tables;
  if (!RestoreBegin(reader, "Set|List", tables)) {
    return false;
  }

  mTableIndex = 0;

  while (!reader->atEnd()) {
    QString tableName;
    *reader >> tableName;
    if (!RestoreTable(tableName, reader, false)) {
      return false;
    }
  }

  mDbBackupController->OnTable(mTableIndex);
  return true;
}

bool DbBackup::RestoreOne(const QString& tableName, QDataStream* reader, bool clear)
{
  QStringList tables;
  if (!RestoreBegin(reader, "One", tables)) {
    return false;
  }

  if (reader->atEnd()) {
    mDbBackupController->OnError(QString("Restore bad format at begining"));
    return false;
  }

  QString storeTableName;
  *reader >> storeTableName;
  if (!tableName.isEmpty() && storeTableName != tableName) {
    mDbBackupController->OnError(QString("Restore table not equals requested (store: '%1', requst: '%2')").arg(storeTableName).arg(tableName));
    return false;
  }
  if (!RestoreTable(storeTableName, reader, false, clear)) {
    return false;
  }

  return true;
}

bool DbBackup::ClearOne(const QString& tableName)
{
  auto q = mDb.MakeQuery();
  mDbBackupController->OnInfo(QString("Begin clear table '%1'").arg(tableName));
  q->prepare(QString("DELETE FROM %1").arg(tableName));
  if (!mDb.ExecuteNonQuery(q)) {
    mDbBackupController->OnError(QString("Clear table '%1' fail").arg(tableName));
    return false;
  }
  mDbBackupController->OnInfo(QString("Done clear table '%1'").arg(tableName));
  return true;
}

bool DbBackup::RestoreBegin(QDataStream* reader, const QByteArray& allowTypes, QStringList& tables)
{
  QByteArray storeType;
  *reader >> storeType;
  if (!allowTypes.contains(storeType)) {
    mDbBackupController->OnError(QString("Restore bad type '%1'").arg(storeType.constData()));
    return false;
  }

  int version;
  *reader >> version;
  if (version != kVersion) {
    mDbBackupController->OnError(QString("Restore unsupported version '%1'").arg(version));
    return false;
  }

  QString nextTable;
  forever {
    *reader >> nextTable;
    if (nextTable.isEmpty()) {
      break;
    }
    tables.append(nextTable);
  }
  if (reader->status() != QDataStream::Ok) {
    return false;
  }
  return true;
}

bool DbBackup::RestoreTable(const QString& tableName, QDataStream* reader, bool skip, bool clear)
{
  if (!skip) {
    mDbBackupController->OnInfo(QString("Begin restore table '%1'").arg(tableName));
  }
  mDbBackupController->OnPercent(0);
  mDbBackupController->OnTable(mTableIndex);

  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT column_name, data_type FROM information_schema.columns"
                     " WHERE table_name = %1 AND table_schema = %2")
             .arg(ToSql(tableName)).arg(ToSql(mSchemaName)));
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  QStringList columns;
  bool hasId = false;
  while (q->next()) {
    QString name = q->value(0).toString();
    if (name != "_id") {
      columns.append(name);
    } else {
      hasId = true;
    }
  }
  if (!hasId) {
    mDbBackupController->OnError(QString("column '_id' not found for table '%1'").arg(tableName));
    return false;
  }

  int storeColumnsSize;
  *reader >> storeColumnsSize;
  if (reader->status() != QDataStream::Ok) {
    return false;
  }
  if (storeColumnsSize != columns.size()) {
    mDbBackupController->OnError(QString("store column count not equals to DB's for table '%1'").arg(tableName));
    return false;
  }

  QVector<int> columnsMap(storeColumnsSize);
  QStringList queryList;
  for (int i = 0; i < columnsMap.size(); i++) {
    QString columnName;
    *reader >> columnName;
    int index = columns.indexOf(columnName);
    if (index < 0) {
      mDbBackupController->OnError(QString("store column '%2' not found in table '%1'").arg(tableName).arg(columnName));
      return false;
    }
    columnsMap[i] = index;
    queryList << "?";
  }
  queryList << "?";

  if (!skip && clear) {
    if (!ClearOne(tableName)) {
      return false;
    }
  }

  QString queryLine = QString("(") + queryList.join(',') + QString(")");
  qint64 currentRecords = 0;
  qint64 totalRecords;
  *reader >> totalRecords;
  qint64 denumRecords = totalRecords/100 + 1;
  int lastPercent = 0;
  if (reader->status() != QDataStream::Ok) {
    return false;
  }

  QVector<QVector<QVariant> > rows(kRecordsAtOnce);
  int index = 0;
  bool eot = false;
  qint64 topId = 1;
  while (!eot && !reader->atEnd()) {
    int currentPercent = (int)(currentRecords / denumRecords);
    if (currentPercent != lastPercent) {
      mDbBackupController->OnPercent(currentPercent);
      lastPercent = currentPercent;
    }
    if (!mDbBackupController->QueryContinue()) {
      mDbBackupController->OnError(QString("User break"));
      return false;
    }

    for (index = 0; index < kRecordsAtOnce - 1 && !reader->atEnd(); index++) {
      qint64 id;
      *reader >> id;
      if (id == 0) {
        *reader >> id;
        if (id < 0) {
          eot = true;
          break;
        }
      }

      if (!skip) {
        topId = qMax(topId, id);
        QVector<QVariant>& row = rows[index];
        if (row.isEmpty()) {
          row.resize(columns.size() + 1);
        }
        row[0] = id;
        for (int i = 0; i < columns.size(); i++) {
          QVariant v;
          *reader >> v;
          row[columnsMap[i] + 1] = v;
        }
      } else {
        for (int i = 0; i < columns.size(); i++) {
          QVariant v;
          *reader >> v;
        }
        index--;
      }
      if (reader->status() != QDataStream::Ok) {
        return false;
      }
    }

    if (index == 0) {
      break;
    }

    QString query = QString("INSERT INTO %1 (_id,%2) VALUES ").arg(tableName).arg(columns.join(','));
    query.append(queryLine);
    for (int i = 1; i < index; i++) {
      query.append(QString(", ") + queryLine);
    }
    q->prepare(query);

    for (int i = 0; i < index; i++) {
      const QVector<QVariant>& row = rows.at(i);
      foreach (const QVariant& cell, row) {
        q->addBindValue(cell);
      }
    }

    if (!mDb.ExecuteQuery(q)) {
      mDbBackupController->OnError(QString("INSERT fail for table '%1'").arg(tableName));
      return false;
    }
    currentRecords += index;
  }

  q->prepare(QString("SELECT setval('%1__id_seq', %2)").arg(tableName).arg(topId));
//  q->prepare(QString("SELECT setval('%1__id_seq', (SELECT _id FROM %1 ORDER BY _id DESC LIMIT 1))").arg(tableName));
  if (!mDb.ExecuteNonQuery(q)) {
    mDbBackupController->OnError(QString("Set sequence fail for table '%1'").arg(tableName));
    return false;
  }

  mDbBackupController->OnInfo(QString("Done %2 table '%1'").arg(tableName).arg(skip? "skip": "restore"));
  mTableIndex++;
  return true;
}


DbBackup::DbBackup(const Db& _Db, const QStringList& _TableOrderList, const DbBackupControllerS& _DbBackupController)
  : mDb(_Db), mSchemaName("public"), mTableOrderList(_TableOrderList), mDbBackupController(_DbBackupController)
  , mTableIndex(0)
{
}

