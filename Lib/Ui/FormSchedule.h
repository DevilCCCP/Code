#pragma once

#include <QWidget>

#include <Lib/Include/Schedule.h>


namespace Ui {
class FormSchedule;
}

class FormSchedule: public QWidget
{
  Ui::FormSchedule* ui;

  QStringList       mInfo;
  int               mTimePrecisionSec;
  QList<QColor>     mInfoColorList;

  Schedule          mSchedule;
  QList<Schedule>   mUndo;
  int               mUndoIndex;

  Q_OBJECT

public:
  explicit FormSchedule(QWidget* parent = 0);
  ~FormSchedule();

public:
  void SetScheduleInfo(const QStringList& _Info, int _TimePrecisionSec);
  void SetSchedule(const Schedule& _Schedule);
  const Schedule& GetSchedule();

private:
  void Clear();
  void PushUndo();

  void DoUndo();
  void DoRedo();

private:
  void OnScheduleChanged();
  void OnCurrentTimeChanged(int secs);
  void OnCurrentObjectChanged(int object);

private slots:
  void on_timeEditPrecision_timeChanged(const QTime& time);
  void on_actionUndo_triggered();
  void on_actionRedo_triggered();
};
