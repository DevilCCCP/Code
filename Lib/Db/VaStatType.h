#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(VaStatType);

class VaStatType: public DbItemT<int>
{
public:
  QString    Abbr;
  QString    Name;
  QString    Descr;

public:
  /*override */virtual bool Equals(const DbItemT<int>& other) const override
  {
    const VaStatType& vs = static_cast<const VaStatType&>(other);
    return DbItemT<int>::Equals(other) && Abbr == vs.Abbr && Name == vs.Name && Descr == vs.Descr;
  }

public:
  VaStatType(): DbItemT<int>() { }
  /*override */virtual ~VaStatType() { }
};

class VaStatTypeTable: public DbTableT<int, VaStatType>
{
  QMap<QString, VaStatTypeS>    mItemsCache;

protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item) override;

public:
  bool GetByAbbr(const QString& abbr, VaStatTypeS& item, bool useCache = true);

public:
  VaStatTypeTable(const Db& _Db);
};
