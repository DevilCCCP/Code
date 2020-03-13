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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE;
  /*override */virtual const char* Select() Q_DECL_OVERRIDE;
//  /*override */virtual const char* Update() Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) Q_DECL_OVERRIDE;
//  /*override */virtual bool OnSetItem(QueryS& q, const TableItem& unit) Q_DECL_OVERRIDE;

  /*override */virtual void CreateIndexes() Q_DECL_OVERRIDE;
  /*override */virtual void ClearIndexes() Q_DECL_OVERRIDE;

public:
  const ObjectSettingsType* GetObjectTypeSettingsType(int typeId, const QString& key);

public:
  ObjectSettingsTypeTable(const Db& _Db);
  /*override*/virtual ~ObjectSettingsTypeTable() { }
};
