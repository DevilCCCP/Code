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
  /*override*/virtual bool Equals(const TableItem &other) const override;

  NamedItem();
  NamedItem(int _Id, const QString& _Name, const QString& _Descr);

  /*override*/virtual ~NamedItem();
};

class TableNamed: public Table
{
  bool                                 mNameIndexed;
  QMultiMap<QString, const NamedItem*> mNameIndex;

protected:
  ///*skip */virtual const char* Select() = 0;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;
  /*override */virtual void CreateIndexes() override;
  /*override */virtual void ClearIndexes() override;

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
