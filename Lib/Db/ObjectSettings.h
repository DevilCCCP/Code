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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE;
  /*override */virtual const char* Select() Q_DECL_OVERRIDE;
  /*override */virtual const char* Insert() Q_DECL_OVERRIDE;
  /*override */virtual const char* Update() Q_DECL_OVERRIDE;
  /*override */virtual const char* Delete() Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowFillItem(QueryS& q, TableItemS& unit) Q_DECL_OVERRIDE;
  /*override */virtual bool OnSetItem(QueryS& q, const TableItem& unit) Q_DECL_OVERRIDE;
  /*override */virtual void CreateIndexes() Q_DECL_OVERRIDE;
  /*override */virtual void ClearIndexes() Q_DECL_OVERRIDE;

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
