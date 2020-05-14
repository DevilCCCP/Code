#pragma once

#include <QString>

#include "DbTable.h"


DefineDbClassS(Variables);

class Variables: public DbItem
{
public:
  int        Object;
  QString    Key;
  QString    Value;

public:
  /*override */virtual bool Equals(const DbItem& other) const Q_DECL_OVERRIDE
  {
    const Variables& vs = static_cast<const Variables&>(other);
    return DbItem::Equals(other) && Object == vs.Object && Key == vs.Key && Value == vs.Value;
  }

public:
  Variables(): DbItem(), Object(0) { }
  /*override */virtual ~Variables() { }
};

class VariablesTable: public DbTableT<int, Variables>
{
protected:
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItem>& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItem& item) Q_DECL_OVERRIDE;

public:
  bool SelectVariables(int objectId, const QString& keyLike, QMap<QString, VariablesS>& variableMap);
  bool SelectVariables(int objectId, const QString& keyLike, QList<VariablesS>& variables);
  bool SelectVariables(int objectId, QList<VariablesS>& variables);
  bool SelectVariables(const QString& key, QList<VariablesS>& variables);

  bool SelectVariable(int objectId, const QString& key, VariablesS& variable);
  bool SelectVariable(const QString& key, VariablesS& variable);

  bool UpdateVariable(int objectId, const QString& key, const QString& value);
  bool UpdateVariable(const QString& key, const QString& value);
  bool UpdateVariable(VariablesS& variable)
  {
    return variable->Object
        ? UpdateVariable(variable->Object, variable->Key, variable->Value)
        : UpdateVariable(variable->Key, variable->Value);
  }

  bool InsertVariable(int objectId, const QString& key, const QString& value);
  bool InsertVariable(const QString& key, const QString& value);

  bool DeleteVariable(int objectId, const QString& key);
  bool DeleteVariable(const QString& key);

public:
  VariablesTable(const Db& _Db);
};
