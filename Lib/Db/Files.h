#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(Files);

class Files: public DbItemT<qint64>
{
public:
  int        ObjectId;
  QString    Name;
  QString    MimeType;
  QByteArray Data;

public:
  /*override */virtual bool Equals(const DbItemT<qint64>& other) const override
  {
    const Files& vs = static_cast<const Files&>(other);
    return DbItemT<qint64>::Equals(other) && ObjectId == vs.ObjectId && Name == vs.Name && MimeType == vs.MimeType && Data == vs.Data;
  }

public:
  Files(): DbItemT<qint64>(), ObjectId(0) { }
  /*override */virtual ~Files() { }
};

class FilesTable: public DbTableT<qint64, Files>
{
protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) override;

public:
  FilesTable(const Db& _Db);
};
