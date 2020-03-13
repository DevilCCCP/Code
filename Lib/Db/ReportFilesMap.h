#pragma once

#include <Lib/Db/MapTableB.h>


class ReportFilesMap: public MapTableB
{
protected:
  /*override */virtual const char* GetTableName() Q_DECL_OVERRIDE;
  /*override */virtual const char* GetColumnKeyName() Q_DECL_OVERRIDE;
  /*override */virtual const char* GetColumnValueName() Q_DECL_OVERRIDE;

public:
  ReportFilesMap(const Db& _Db);
  /*override */~ReportFilesMap();
};

