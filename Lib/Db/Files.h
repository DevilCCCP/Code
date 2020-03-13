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
  /*override */virtual bool Equals(const DbItemT<qint64>& other) Q_DECL_OVERRIDE
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
  /*override */virtual QString TableName() Q_DECL_OVERRIDE;
  /*override */virtual QString Columns() Q_DECL_OVERRIDE;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item) Q_DECL_OVERRIDE;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item) Q_DECL_OVERRIDE;

public:
  FilesTable(const Db& _Db);
};
