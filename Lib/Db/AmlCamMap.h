#pragma once

#include <Lib/Db/MapTable.h>


class AmlCamMapTable: public MapTable
{
protected:
  /*override */virtual const char* GetTableName() Q_DECL_OVERRIDE { return "arm_monitor_lay_cameras"; }
  /*override */virtual const char* GetColumnKeyName() Q_DECL_OVERRIDE { return "_amlayout"; }
  /*override */virtual const char* GetColumnValueName() Q_DECL_OVERRIDE { return "_camera"; }

public:
  AmlCamMapTable(const Db& _Db);
  /*override */~AmlCamMapTable();
};

