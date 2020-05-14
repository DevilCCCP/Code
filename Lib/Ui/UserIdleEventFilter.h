#pragma once

#include <QObject>


class QTimer;

class UserIdleEventFilter: public QObject
{
  int         mInterval;

  QTimer*     mIdleTimer;

  Q_OBJECT

protected:
  /*override */bool eventFilter(QObject* obj, QEvent* ev) override;

public:
  void Install();
  void Remove();

public:
  UserIdleEventFilter(int _Interval, QObject* parent = nullptr);
  ~UserIdleEventFilter();

signals:
  void Idle();
};
