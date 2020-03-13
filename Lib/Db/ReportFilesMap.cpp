#include "ReportFilesMap.h"


const char* ReportFilesMap::GetTableName()
{
  return "report_files";
}

const char* ReportFilesMap::GetColumnKeyName()
{
  return "_report";
}

const char* ReportFilesMap::GetColumnValueName()
{
  return "_files";
}


ReportFilesMap::ReportFilesMap(const Db& _Db)
  : MapTableB(_Db)
{
}

ReportFilesMap::~ReportFilesMap()
{
}
