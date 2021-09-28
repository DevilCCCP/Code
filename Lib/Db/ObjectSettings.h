#pragma once

#include <QString>
#include <QMap>

#include "Table.h"
#include "Db.h"

DefineDbClassS(ObjectSettings);

class ObjectSettings: public TableItem
{
public:
  int     ObjectId;
  QString Key;
  QString Value;

public:
  ObjectSettings() { }
  /*override*/virtual ~ObjectSettings() { }
};

class ObjectSettingsTable: public Table
{
  QMap<int, QList<ObjectSettingsS> > mObjectSettingsIndex;
  bool                               mIndexed;

protected:
  /*override */virtual const char* Name() override;
  /*override */virtual const char* Select() override;
  /*override */virtual const char* Insert() override;
  /*override */virtual const char* Update() override;
  /*override */virtual const char* Delete() override;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) override;
  /*override */virtual bool OnSetItem(QueryS& q, const TableItem& unit) override;
  /*override */virtual void CreateIndexes() override;
  /*override */virtual void ClearIndexes() override;

public:
  bool GetObjectSettings(int objectId, QList<ObjectSettingsS>& settings, bool useCache = false);
  bool GetObjectSettings(QString key, QString value, QList<ObjectSettingsS>& settings);
  bool GetObjectSettings(int objectId, QString key, QString value, QList<ObjectSettingsS>& settings);
  bool DeleteObjectSettings(int objectId, const QString& pattern);

private:
  void UpdateSettings(QList<ObjectSettingsS>& settings);

public:
  ObjectSettingsTable(const Db& _Db);
  /*override*/virtual ~ObjectSettingsTable() { }
};
