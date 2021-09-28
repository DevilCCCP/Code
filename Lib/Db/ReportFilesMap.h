#pragma once

#include <Lib/Db/MapTableB.h>


class ReportFilesMap: public MapTableB
{
protected:
  /*override */virtual const char* GetTableName() override;
  /*override */virtual const char* GetColumnKeyName() override;
  /*override */virtual const char* GetColumnValueName() override;

public:
  ReportFilesMap(const Db& _Db);
  /*override */~ReportFilesMap();
};

