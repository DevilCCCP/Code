#pragma once

#include <QWidget>

#include <Lib/Db/Db.h>

#include "LogSchema.h"


namespace Ui {
class FormObjectLog;
}

class FormObjectLog: public QWidget
{
  Ui::FormObjectLog*     ui;

  Db*                    mDb;

  QDateTime              mFromTime;
  QDateTime              mToTime;
  int                    mPeriodSecs;
  QList<ObjectItemS>     mObjectList;

  Q_OBJECT

public:
  explicit FormObjectLog(QWidget* parent = 0);
  ~FormObjectLog();

public:
  void Init(Db* _Db, const LogSchema& _LogSchema);

  void SetTimePeriod(const QDateTime& startTime, int periodSecs);
  void SetObjectsList(const QList<ObjectItemS>& objectList);
  void Update();

private slots:
  void on_dateTimeEditFrom_dateTimeChanged(const QDateTime& dateTime);
  void on_dateTimeEditTo_dateTimeChanged(const QDateTime& dateTime);
  void on_comboBoxTimePeriod_editTextChanged(const QString& value);
};
