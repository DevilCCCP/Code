#pragma once

#include <QString>
#include "Table.h"

DefineClassS(NamedItem);

class NamedItem: public TableItem
{
public:
  QString Name;
  QString Descr;

public:
  /*override*/virtual bool Equals(const TableItem &other) Q_DECL_OVERRIDE;

  NamedItem();
  NamedItem(int _Id, const QString& _Name, const QString& _Descr);

  /*override*/virtual ~NamedItem();
};

class TableNamed: public Table
{
  bool                            mNameIndexed;
  QMap<QString, const NamedItem*> mNameIndex;

protected:
  /*skip */virtual const char* Select() = 0;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) Q_DECL_OVERRIDE;
  /*override */virtual void CreateIndexes() Q_DECL_OVERRIDE;
  /*override */virtual void ClearIndexes() Q_DECL_OVERRIDE;

protected:
  void SelectOneByName(const QString& name, QString& queryText);

public:
  void Clear();

  const NamedItem* GetItemByName(const QString& name);
  bool GetTypeIdByName(const QString& name, int& id);

private:
  void CreateNameIndex();

public:
  TableNamed(const Db& _Db, bool _NameIndexed = false);
  /*override*/virtual ~TableNamed();
};
