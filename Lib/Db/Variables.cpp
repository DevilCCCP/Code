#include <QSqlQuery>
#include <QVariant>

#include <Lib/Log/Log.h>

#include "Variables.h"


QString VariablesTable::TableName()
{
  return "variables";
}

QString VariablesTable::Columns()
{
  return "_object,key,value";
}

bool VariablesTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItem>& item)
{
  Variables* variable;
  item.reset(variable = new Variables());
  variable->Object = q->value(index++).toInt();
  variable->Key    = q->value(index++).toString();
  variable->Value  = q->value(index++).toString();
  return true;
}

bool VariablesTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItem& item)
{
  const Variables& variable = static_cast<const Variables&>(item);
  q->bindValue(index++, Db::ToKey(variable.Object));
  q->bindValue(index++, variable.Key);
  q->bindValue(index++, variable.Value);
  return true;
}

bool VariablesTable::SelectVariables(int objectId, const QString& keyLike, QMap<QString, VariablesS>& variableMap)
{
  QString where = QString("WHERE _object = %1 AND key LIKE %2").arg(objectId).arg(ToSql(keyLike));
  QList<VariablesS> list;
  if (!Select(where, list)) {
    return false;
  }
  foreach (const VariablesS& item, list) {
    variableMap[item->Key] = item;
  }
  return true;
}

bool VariablesTable::SelectVariables(int objectId, const QString& keyLike, QList<VariablesS>& variables)
{
  QString where = QString("WHERE _object = %1 AND key LIKE %2").arg(objectId).arg(ToSql(keyLike));
  if (!Select(where, variables)) {
    return false;
  }
  return true;
}

bool VariablesTable::SelectVariables(int objectId, QList<VariablesS>& variables)
{
  QString where = QString("WHERE _object = %1").arg(objectId);
  if (!Select(where, variables)) {
    return false;
  }
  return true;
}

bool VariablesTable::SelectVariables(const QString& key, QList<VariablesS>& variables)
{
  QString where = QString("WHERE _object IS NULL AND key = %1").arg(ToSql(key));
  if (!Select(where, variables)) {
    return false;
  }
  return true;
}

bool VariablesTable::SelectVariable(int objectId, const QString& key, VariablesS& variable)
{
  QString where = QString("WHERE _object = %1 AND key = %2").arg(objectId).arg(ToSql(key));
  QList<VariablesS> list;
  if (!Select(where, list)) {
    return false;
  }
  if (list.isEmpty()) {
    variable.clear();
    return true;
  }

  variable = list.first();
  return true;
}

bool VariablesTable::SelectVariable(const QString& key, VariablesS& variable)
{
  QString where = QString("WHERE _object IS NULL AND key = %1").arg(ToSql(key));
  QList<VariablesS> list;
  if (!Select(where, list)) {
    return false;
  }
  if (list.isEmpty()) {
    variable.clear();
    return true;
  }

  variable = list.first();
  return true;
}

bool VariablesTable::UpdateVariable(int objectId, const QString& key, const QString& value)
{
  auto q = getDb().MakeQuery();
  q->prepare("UPDATE variables SET value = ? WHERE key = ? AND _object = ? RETURNING _id");
  q->addBindValue(value);
  q->addBindValue(key);
  q->addBindValue(objectId);

  if (!this->getDb().ExecuteQuery(q)) {
    return false;
  }
  if (q->next()) {
    return true;
  }
  return InsertVariable(objectId, key, value);
}

bool VariablesTable::UpdateVariable(const QString& key, const QString& value)
{
  auto q = getDb().MakeQuery();
  q->prepare("UPDATE variables SET value = ? WHERE key = ? AND _object IS NULL");
  q->addBindValue(value);
  q->addBindValue(key);

  if (!this->getDb().ExecuteQuery(q)) {
    return false;
  }
  if (q->next()) {
    return true;
  }
  return InsertVariable(key, value);
}

bool VariablesTable::InsertVariable(int objectId, const QString& key, const QString& value)
{
  auto q = getDb().MakeQuery();
  q->prepare("INSERT INTO variables(_object, key, value) VALUES (?, ?, ?);");
  q->addBindValue(objectId);
  q->addBindValue(key);
  q->addBindValue(value);

  if (!this->getDb().ExecuteQuery(q)) {
    return false;
  }
  return true;
}

bool VariablesTable::InsertVariable(const QString& key, const QString& value)
{
  auto q = getDb().MakeQuery();
  q->prepare("INSERT INTO variables(key, value) VALUES (?, ?);");
  q->addBindValue(key);
  q->addBindValue(value);

  if (!this->getDb().ExecuteQuery(q)) {
    return false;
  }
  return true;
}

bool VariablesTable::DeleteVariable(int objectId, const QString& key)
{
  auto q = getDb().MakeQuery();
  q->prepare("DELETE FROM variables WHERE _object = ? AND key = ?;");
  q->addBindValue(objectId);
  q->addBindValue(key);

  if (!this->getDb().ExecuteQuery(q)) {
    return false;
  }
  return true;
}

bool VariablesTable::DeleteVariable(const QString& key)
{
  auto q = getDb().MakeQuery();
  q->prepare("DELETE FROM variables WHERE key = ? AND _object IS NULL;");
  q->addBindValue(key);

  if (!this->getDb().ExecuteQuery(q)) {
    return false;
  }
  return true;
}


VariablesTable::VariablesTable(const Db& _Db)
  : DbTableT<int, Variables>(_Db)
{
}
