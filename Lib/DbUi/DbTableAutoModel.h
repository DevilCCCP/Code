#pragma once

#include "DbTableModel.h"


template <typename DbItemT, typename DbTableT>
class DbTableAutoModel: public DbTableModel<DbItemT>
{
public:
  DbTableAutoModel(const DbTableT& table, QObject* parent = 0)
    : DbTableModel<DbItemT>(table.Headers(), table.Icon(), parent)
  {
  }
};

