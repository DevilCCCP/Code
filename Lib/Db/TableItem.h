#pragma once

#include <Lib/Include/Common.h>


DefineClassS(TableItem);
DefineClassS(TableItemB);

class TableItem
{
public:
  int Id;

public:
  /*new*/virtual bool Equals(const TableItem& other) const { return Id == other.Id && Id != 0; }

public:
  TableItem()
    : Id(0)
  { }
  TableItem(int _Id)
    : Id(_Id)
  { }
  /*new*/virtual ~TableItem() { }
};

class TableItemB
{
public:
  qint64 Id;

public:
  /*new*/virtual bool Equals(const TableItemB& other) const { return Id == other.Id && Id != 0; }

public:
  TableItemB()
    : Id(0)
  { }
  TableItemB(qint64 _Id)
    : Id(_Id)
  { }
  /*new*/virtual ~TableItemB() { }
};
