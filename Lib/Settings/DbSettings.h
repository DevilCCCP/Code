#pragma once

#include <QMap>

#include <Lib/Db/Db.h>
#include <Lib/Include/Common.h>

#include "SettingsA.h"


DefineClassS(DbSettings);

class DbSettings: public SettingsA
{
  const Db&      mDb;
  const QString  mTableName;
  const QString  mObjectColumnName;
  QString        mObjectId;
  QString        mPrefix;

  bool           mError;

public:
  bool HasError() const { return mError; }

public:
  /*override */virtual bool Open(const QString& _ObjectId) override;
  /*override */virtual bool Sync() override;

  /*override */virtual bool BeginGroup(const QString& prefix) override;
  /*override */virtual void EndGroup() override;
  /*override */virtual QVariant GetMandatoryValue(const QString& key, bool fatal = false) override;
  /*override */virtual QVariant GetValue(const QString& key, const QVariant& defaultValue = QVariant()) override;
  /*override */virtual void SetValue(const QString& key, const QVariant& value) override;

public:
  bool Open(int objectId);
  bool ExportAsDictionary(QMap<QString, QString>& dictionary);
  bool Clean();
  bool Clean(const QString& where);

private:
  QueryS PrepareGetValueQuery(const QString & key);

public:
  DbSettings(const Db& _Db, const QString& _TableName, const QString& _ObjectColumnName);
  DbSettings(const Db& _Db);
  /*override */~DbSettings();
};

