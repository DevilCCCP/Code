#pragma once

#include <QString>


struct FilterInfo {
  QString        Name;
  QString        Param1Name;
  int            Param1Min;
  int            Param1Max;
  int            Param1Default;
  QString        Param2Name;
  int            Param2Min;
  int            Param2Max;
  int            Param2Default;

  FilterInfo()
    : Param1Min(0), Param1Max(0), Param1Default(0)
    , Param2Min(0), Param2Max(0), Param2Default(0)
  { }
};
