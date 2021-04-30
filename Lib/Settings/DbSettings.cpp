#include <Lib/Log/Log.h>

#include "DbSettings.h"


bool DbSettings::Open(const QString& _ObjectId)
{
  mObjectId = _ObjectId;
  mPrefix.clear();
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT column_name FROM information_schema.columns WHERE table_name='%1' AND column_name='%2';")
             .arg(mTableName).arg(mObjectColumnName));
  return !mDb.ExecuteScalar(q).isNull();
}

bool DbSettings::Sync()
{
  return true;
}

bool DbSettings::BeginGroup(const QString &prefix)
{
  mPrefix += prefix + "/";
  return true;
}

void DbSettings::EndGroup()
{
  int pos = mPrefix.lastIndexOf("/", -2);
  if (pos > 0) {
    mPrefix = mPrefix.left(pos);
  } else {
    mPrefix.clear();
  }
}

QVariant DbSettings::GetMandatoryValue(const QString &key, bool fatal)
{
  auto q = PrepareGetValueQuery(key);

  QVariant value = mDb.ExecuteScalar(q);
  if (value == QVariant()) {
    Log.Fatal(QString("Read setting '%2%1' fail").arg(key).arg(mPrefix), fatal);
  } else {
    if (!IsSilent()) {
      Log.Info(QString("Read setting '%3%1' = '%2'").arg(key).arg(value.toString()).arg(mPrefix));
    }
  }
  return value;
}

QVariant DbSettings::GetValue(const QString &key, const QVariant &defaultValue)
{
  auto q = PrepareGetValueQuery(key);

  QVariant value = mDb.ExecuteScalar(q);
  if (value == QVariant()) {
    value = defaultValue;
    if (IsAutoCreate()) {
      SetValue(key, value);
      if (!mError && !IsSilent()) {
        Log.Info(QString("Created setting '%1' = '%2'").arg(key).arg(value.toString()));
      }
    } else if (!IsSilent()) {
      Log.Warning(QString("Use default setting '%1' = '%2'").arg(key).arg(value.toString()));
    }
  } else {
    if (!IsSilent()) {
      Log.Info(QString("Read setting '%1' = '%2'").arg(key).arg(value.toString()));
    }
  }
  return value;
}

void DbSettings::SetValue(const QString &key, const QVariant &value)
{
  mError = false;
  if (!IsSilent()) {
    Log.Info(QString("Set '%3%1' = '%2'").arg(key).arg(value.toString()).arg(mPrefix));
  }
  auto q = mDb.MakeQuery();
  q->prepare(QString("UPDATE %1 SET value = '%5' WHERE %2 = '%3' AND key = '%6%4' RETURNING _id;")
             .arg(mTableName, mObjectColumnName, mObjectId, key, value.toString(), mPrefix));
  if (!mDb.ExecuteNonQuery(q)) {
    Log.Warning(QString("Set setting fail (key: '%3%1', value: '%2'").arg(key).arg(value.toString()).arg(mPrefix));
    mError = true;
  }
  if (q->next()) {
    return;
  }
  q->prepare(QString("INSERT INTO %1 (%2, key, value) VALUES ('%3', '%6%4', '%5');")
             .arg(mTableName).arg(mObjectColumnName).arg(mObjectId).arg(key).arg(value.toString()).arg(mPrefix));
  if (!mDb.ExecuteNonQuery(q)) {
    Log.Warning(QString("Set setting fail (key: '%3%1', value: '%2'").arg(key).arg(value.toString()).arg(mPrefix));
    mError = true;
  }
}

bool DbSettings::Open(int objectId)
{
  return Open(QString::number(objectId));
}

bool DbSettings::ExportAsDictionary(QMap<QString, QString> &dictionary)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT key, value FROM %1 WHERE %2 = '%3'")
             .arg(mTableName).arg(mObjectColumnName).arg(mObjectId));

  if (!mDb.ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    QString key = q->value(0).toString();
    QString value = q->value(1).toString();
    dictionary.insert(key, value);
  }
  return true;
}

bool DbSettings::Clean()
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM %1 WHERE %2 = '%3'")
             .arg(mTableName).arg(mObjectColumnName).arg(mObjectId));

  return mDb.ExecuteNonQuery(q);
}

bool DbSettings::Clean(const QString& where)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("DELETE FROM %1 WHERE %2 = '%3' AND %4")
             .arg(mTableName).arg(mObjectColumnName).arg(mObjectId).arg(where));

  return mDb.ExecuteNonQuery(q);
}

QueryS DbSettings::PrepareGetValueQuery(const QString &key)
{
  auto q = mDb.MakeQuery();
  q->prepare(QString("SELECT value FROM %1 WHERE %2 = '%3' AND key = '%5%4'")
             .arg(mTableName).arg(mObjectColumnName).arg(mObjectId).arg(key).arg(mPrefix));
  return q;
}

DbSettings::DbSettings(const Db& _Db, const QString& _TableName, const QString& _ObjectColumnName)
  : mDb(_Db), mTableName(_TableName), mObjectColumnName(_ObjectColumnName)
  , mError(false)
{
}

DbSettings::DbSettings(const Db& _Db)
  : mDb(_Db), mTableName("object_settings"), mObjectColumnName("_object")
  , mError(false)
{
}

DbSettings::~DbSettings()
{
}



