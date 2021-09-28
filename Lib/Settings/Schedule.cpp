#include <Lib/Log/Log.h>

#include "Schedule.h"


const qint64 kEndOfDay = 24*60*60*1000;

qint64 Schedule::NextUpdateMs()
{
  if (!mNextUpdateTimer.isValid() || mNextUpdateTimer.elapsed() >= mNextUpdateMs) {
    Init();
  }
  return mNextUpdateMs - mNextUpdateTimer.elapsed();
}

bool Schedule::Load(SettingsA* settings)
{
  TimePeriod period;
  QString daily = settings->GetValue("Dayly", QString()).toString();
  if (!daily.isEmpty()) {
    if (ParseTimePeriod(daily, period)) {
      mTimePeriods.fill(period, 7);
      Init();
      return true;
    } else {
      Log.Warning(QString("Schedule dayly period is invalid (text: '%1')").arg(daily));
    }
  }
  TimePeriod zeroPeriod;
  zeroPeriod.FromMs = -1;
  zeroPeriod.ToMs = -1;
  mTimePeriods.fill(zeroPeriod, 7);
  bool anyok = false;
  for (int i = 0; i < 7; i++) {
    QString weekly = settings->GetValue(QString("Weekly%1").arg(i), QString()).toString();
    if (weekly.isEmpty()) {
      continue;
    }
    TimePeriod period;
    if (ParseTimePeriod(weekly, period)) {
      mTimePeriods[i] = period;
      anyok = true;
    }
  }
  if (anyok) {
    Init();
  }
  return anyok;
}

bool Schedule::IsActive()
{
  if (!mNextUpdateTimer.isValid() || mNextUpdateTimer.elapsed() >= mNextUpdateMs) {
    Init();
  }
  return mCurrentAlive;
}

QString Schedule::Dump()
{
  if (mTimePeriods.isEmpty()) {
    return "empty";
  }

  QStringList ranges;
  int lastIndex = 0;
  const TimePeriod* lastPeriod = &mTimePeriods.at(0);
  for (int i = 1; i < mTimePeriods.size(); i++) {
    if (!TimePeriod::IsEqual(mTimePeriods[i], *lastPeriod)){
      ranges << DumpOneRange(lastPeriod, lastIndex, i - 1);
      lastPeriod = &mTimePeriods.at(i);
      lastIndex = i;
    }
  }
  ranges << DumpOneRange(lastPeriod, lastIndex, mTimePeriods.size() - 1);
  return ranges.join(", ");
}

QString Schedule::DumpNextUpdate()
{
  return QString("Schedule is %1, next switch after %2")
      .arg(mCurrentAlive? "active": "passive").arg(mNextUpdateMs);
}

QString Schedule::DumpOneRange(const TimePeriod* period, int indexFrom, int indexTo)
{
  if (indexFrom != indexTo) {
    return QString("%1-%2: %3").arg(indexFrom).arg(indexTo).arg(DumpOnePeriod(period));
  } else {
    return QString("%1: %2").arg(indexFrom).arg(DumpOnePeriod(period));
  }
}

QString Schedule::DumpOnePeriod(const TimePeriod* period)
{
  QTime from = QTime::fromMSecsSinceStartOfDay(period->FromMs);
  QTime to = QTime::fromMSecsSinceStartOfDay(qMin((qint64)period->ToMs, kEndOfDay - 1));
  return QString("%1-%2").arg(from.toString("hh:mm")).arg(to.toString("hh:mm"));
}

qint64 Schedule::MaxNextUpdate()
{
  QDateTime now = QDateTime::currentDateTime();
  QDateTime begin = now;
  begin.setTime(QTime(0, 0));
  int msecs = begin.msecsTo(now);
  return kEndOfDay - msecs;
//  return kEndOfDay;
}

bool Schedule::ParseTimePeriod(const QString& text, TimePeriod& period)
{
  period.FromMs = 0;
  period.ToMs   = kEndOfDay;

  QStringList pair = text.split('-', Qt::KeepEmptyParts);
  if (pair.size() != 2) {
    return false;
  }

  return ParseTime(pair.first(), period.FromMs, 0) && ParseTime(pair.last(), period.ToMs, kEndOfDay);
}

bool Schedule::ParseDayWPeriod(const QString& text, DayWPeriod& period)
{
  memset(period.Days, 0, sizeof(period.Days));

  int startRange = -1;
  QString dayText;
  int day = -1;
  for (int i = 0; i <= text.size(); i++) {
    QChar ch = (i < text.size())? text[i]: ',';
    if (ch >= 1 && ch <= 7) {
      if (!dayText.isEmpty() || day >= 0) {
        return false;
      } else {
        day = ch.toLatin1() - '1';
      }
    } else if (ch == ',') {
      if (!dayText.isEmpty()) {
        return false;
      }
      if (startRange < 0) {
        if (day >= 0) {
          period.Days[day] = 1;
          day = -1;
        }
      } else {
        if (day < 0) {
          day = 6;
        }
        for (int j = startRange; j <= day; j++) {
          period.Days[j] = 1;
        }
        startRange = day = -1;
      }
    } else if (ch == '-') {
      if (!dayText.isEmpty()) {
        return false;
      }
      startRange = (day >= 0)? day: 0;
      day = -1;
    } else if (ch.isSpace()) {
    } else {
      if (day >= 0) {
        return false;
      }
      dayText.append(ch);
    }
    if (!dayText.isEmpty()) {
      QString test = dayText.toLower();
      if (test == "пн") {
        day = 0;
      } else if (test == "вт") {
        day = 1;
      } else if (test == "ср") {
        day = 2;
      } else if (test == "чт") {
        day = 3;
      } else if (test == "пт") {
        day = 4;
      } else if (test == "сб") {
        day = 5;
      } else if (test == "вс") {
        day = 6;
      }
      if (day >= 0) {
        dayText.clear();
      }
    }
  }
  return true;
}

bool Schedule::ParseTime(const QString& text, int& point, int defaultPoint)
{
  QStringList time = text.split(':', Qt::KeepEmptyParts);
  if (time.isEmpty()) {
    point = defaultPoint;
    return true;
  }
  bool ok;
  int h = time[0].toInt(&ok);
  if (!ok) {
    return false;
  }
  int m = (time.size() > 1)? time[1].toInt(): 0;
  int s = (time.size() > 2)? time[2].toInt(): 0;
  point = ((h * 60 + m) * 60 + s) * 1000;
  return true;
}

void Schedule::Init()
{
  QDateTime now = QDateTime::currentDateTime();
  qint64 msecs = now.time().msecsSinceStartOfDay();
  int dayweek = now.date().dayOfWeek() - 1;
  mNextUpdateMs = MaxNextUpdate();
  if (dayweek >= 0 && dayweek < mTimePeriods.size()) {
    const TimePeriod& period = mTimePeriods[dayweek];
    if (msecs < period.FromMs) {
      mCurrentAlive = false;
      mNextUpdateMs = period.FromMs - msecs;
    } else if (msecs < period.ToMs) {
      mCurrentAlive = true;
      mNextUpdateMs = period.ToMs - msecs;
    } else {
      mCurrentAlive = false;
    }
  } else {
    mCurrentAlive = true;
  }
  mNextUpdateMs = qMin(mNextUpdateMs, kEndOfDay - msecs);
  mNextUpdateTimer.start();
}


Schedule::Schedule()
  : mCurrentAlive(false)
{

}

Schedule::~Schedule()
{

}
