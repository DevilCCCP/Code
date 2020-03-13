#pragma once

#include <QObject>

#include <Lib/Db/Db.h>


struct Statistics {
  qint64    Red;
  qint64    Gray;
  qint64    Green;

  Statistics(): Red(0), Gray(0), Green(0) { }
};
typedef QMap<QDate, Statistics> StatisticsMap;

class StatisticsLoader: public QObject
{
  Db            mDb;
  ObjectStateS  mObjectStateTable;
  ObjectStateHoursTableS mObjectStateHoursTable;

  int           mDays;
  qint64        mCountMax;
  volatile bool mStop;

  struct State {
    int       ObjectId;
    QDateTime LastChange;
    int       NewState;

    State(int _ObjectId, QDateTime _LastChange): ObjectId(_ObjectId), LastChange(_LastChange), NewState(0) { }
    State(int _ObjectId): ObjectId(_ObjectId), NewState(0) { }
    State(): ObjectId(0), NewState(0) { }
  };
  QMap<int, State>         mCurrentMap;

  QMap<int, StatisticsMap> mResultsMap;

  Q_OBJECT

public:
  const QMap<int, StatisticsMap>& ResultsMap() const { return mResultsMap; }

public:
  void Load(int days, const QDateTime& from, const QDateTime& to);
  void LoadFast(int days, const QDateTime& from, const QDateTime& to);
  void Stop();

private:
  bool PrepareLogs(const QDateTime& from);
  bool PrepareFastLogs();
  void LoadOne(int type, int oldState, int newState, const QDateTime& ts);
  void FinishOne(int type, const QDateTime& ts);
  void LoadFastOne(int id, int goodState, int badState, const QDateTime& ts);

signals:
  void CountCanged(qint64 count, qint64 countMax);

public:
  StatisticsLoader(QObject* parent = 0);
};
