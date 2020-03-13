#pragma once

#include <QSharedPointer>
#include <QWeakPointer>

struct TimePeriod {
  int BeginSec;
  int EndSec;
  TimePeriod(int _BeginSec, int _EndSec): BeginSec(_BeginSec), EndSec(_EndSec) { }
  TimePeriod(): BeginSec(0), EndSec(0) { }
};
typedef QVector<TimePeriod> TimePeriodList;
typedef QVector<TimePeriodList> Schedule;
typedef QSharedPointer<Schedule> ScheduleS;
typedef QWeakPointer<Schedule> ScheduleW;
