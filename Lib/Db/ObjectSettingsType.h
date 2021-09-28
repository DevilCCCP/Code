#pragma once

#include <QString>
#include <QMap>

#include "TableNamed.h"
#include "Db.h"

DefineDbClassS(ObjectSettingsType);

class ObjectSettingsType: public NamedItem
{
public:
  int     ObjectTypeId;
  QString Key;
  QString Type;
  QString MinValue;
  QString MaxValue;

public:
  ObjectSettingsType() { }
  /*override*/virtual ~ObjectSettingsType() { }
};

class ObjectSettingsTypeTable: public TableNamed
{
  QMap<int, QMap<QString, const ObjectSettingsType*> > mTypeKeyIndexs;

protected:
  /*override */virtual const char* Name() override;
  /*override */virtual const char* Select() override;
//  /*override */virtual const char* Update() override;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;
//  /*override */virtual bool OnSetItem(QueryS& q, const TableItem& unit) override;

  /*override */virtual void CreateIndexes() override;
  /*override */virtual void ClearIndexes() override;

public:
  const ObjectSettingsType* GetObjectTypeSettingsType(int typeId, const QString& key);

public:
  ObjectSettingsTypeTable(const Db& _Db);
  /*override*/virtual ~ObjectSettingsTypeTable() { }
};
