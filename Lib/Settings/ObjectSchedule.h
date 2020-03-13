#pragma once

#include <Lib/Db/Db.h>

#include "Schedule.h"


class ObjectSchedule: public Schedule
{
  Db*      mDb;
  int      mId;

public:
  bool LoadSchedule();

public:
  ObjectSchedule(Db* _Db, int _Id);
};

