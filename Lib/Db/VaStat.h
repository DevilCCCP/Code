#pragma once

#include <QString>
#include <QDateTime>
#include <QPoint>
#include <QRect>

#include <Lib/Db/DbTable.h>


DefineDbClassS(VaStat);

class VaStat: public DbItemT<int>
{
public:
  int        ObjectId;
  int        VstypeId;

public:
  /*override */virtual bool Equals(const DbItemT<int>& other) const override
  {
    const VaStat& vs = static_cast<const VaStat&>(other);
    return DbItemT<int>::Equals(other) && ObjectId == vs.ObjectId && VstypeId == vs.VstypeId;
  }

public:
  VaStat(): DbItemT<int>(), ObjectId(0), VstypeId(0) { }
  /*override */virtual ~VaStat() { }
};

class VaStatTable: public DbTableT<int, VaStat>
{
  QMap<QPair<int, int>, VaStatS> mItemsMap;

protected:
  /*override */virtual QString TableName() override;
  /*override */virtual QString Columns() override;

  /*override */virtual bool OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item) override;
  /*override */virtual bool OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item) override;

public:
  bool GetSimple(int objectId, int vstypeId, VaStatS& vaStatItem, bool useCache = true);

public:
  VaStatTable(const Db& _Db);
};
