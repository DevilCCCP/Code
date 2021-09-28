#pragma once

#include <Lib/Db/MapTable.h>


class AmlCamMapTable: public MapTable
{
protected:
  /*override */virtual const char* GetTableName() override { return "arm_monitor_lay_cameras"; }
  /*override */virtual const char* GetColumnKeyName() override { return "_amlayout"; }
  /*override */virtual const char* GetColumnValueName() override { return "_camera"; }

public:
  AmlCamMapTable(const Db& _Db);
  /*override */~AmlCamMapTable();
};

