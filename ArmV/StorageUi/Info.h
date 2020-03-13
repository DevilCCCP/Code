#pragma once

#include <QString>
#include <QVector>
#include <QList>


struct CellInfo {
  int    Id;
  int    UnitId;
  qint64 StartTime;
};

typedef QList<CellInfo> CellInfoList;

struct CellInfoEx {
  int    SourceId;
  int    Id;
  int    UnitExportId;
  int    UnitImportId;
  qint64 StartTime;
};

typedef QVector<CellInfoEx> CellsVector;

struct ContInfo {
  QString     Path;
  int         CellSize;
  int         CellPageSize;
  int         Capacity;

  ContInfo()
    : CellSize(0), CellPageSize(0), Capacity(0)
  { }
  ContInfo(const ContInfo& _ContInfo)
    : Path(_ContInfo.Path), CellSize(_ContInfo.CellSize), CellPageSize(_ContInfo.CellPageSize), Capacity(_ContInfo.Capacity)
  { }
  ContInfo(const QString& _Path, int _CellSize, int _CellPageSize, int _Capacity)
    : Path(_Path), CellSize(_CellSize), CellPageSize(_CellPageSize), Capacity(_Capacity)
  { }
};
