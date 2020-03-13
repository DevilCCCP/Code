#pragma once

#include <QDateTime>
#include <QElapsedTimer>
#include <QVector>

#include <Lib/Settings/SettingsA.h>


DefineClassS(Schedule);
DefineClassS(SettingsA);

struct TimePeriod {
  int FromMs;
  int ToMs;

  static bool IsEqual(const TimePeriod& period1, const TimePeriod& period2) { return period1.FromMs == period2.FromMs && period1.ToMs == period2.ToMs; }
};
typedef QVector<TimePeriod> TimePeriodList;

struct DayWPeriod {
  int Days[7];
};

class Schedule
{
  TimePeriodList mTimePeriods;

  QElapsedTimer  mNextUpdateTimer;
  qint64         mNextUpdateMs;
  bool           mCurrentAlive;

public:
  qint64 NextUpdateMs();

public:
  bool Load(SettingsA* settings);

  bool IsActive();

  QString Dump();
  QString DumpNextUpdate();
private:
  QString DumpOneRange(const TimePeriod* period, int indexFrom, int indexTo);
public:
  static QString DumpOnePeriod(const TimePeriod* period);

public:
  static qint64 MaxNextUpdate();

  static bool ParseTimePeriod(const QString& text, TimePeriod& period);
  static bool ParseDayWPeriod(const QString& text, DayWPeriod& period);
private:
  static bool ParseTime(const QString& text, int& point, int defaultPoint);

  void Init();

public:
  Schedule();
  virtual ~Schedule();
};
