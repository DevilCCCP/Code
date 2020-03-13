#pragma once

#include "ColumnEdit/ColumnEditA.h"


struct TableEditSchema
{
  struct ColumnSchema
  {
    int          Key;
    QString      Lable;
    ColumnEditAS ColumnEdit;

    ColumnSchema(QString _Lable, const ColumnEditAS& _ColumnEdit)
      : Key(0), Lable(_Lable), ColumnEdit(_ColumnEdit)
    { }
    ColumnSchema(QString _Lable, int _Key, const ColumnEditAS& _ColumnEdit)
      : Key(_Key), Lable(_Lable), ColumnEdit(_ColumnEdit)
    { }
  };

  QString             Name;
  QList<ColumnSchema> Columns;
  bool                CanClone;
  bool                Compact;

  TableEditSchema()
    : CanClone(false), Compact(true)
  { }
};

