#include <QApplication>
#include <QTimer>
#include <QEvent>

#include "UserIdleEventFilter.h"


bool UserIdleEventFilter::eventFilter(QObject* obj, QEvent* ev)
{
  if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::MouseMove || ev->type() == QEvent::MouseButtonPress) {
    mIdleTimer->start(mInterval);
  }

  return QObject::eventFilter(obj, ev);
}

void UserIdleEventFilter::Install()
{
  mIdleTimer->start();
  qApp->installEventFilter(this);
}

void UserIdleEventFilter::Remove()
{
  mIdleTimer->stop();
  qApp->removeEventFilter(this);
}


UserIdleEventFilter::UserIdleEventFilter(int _Interval, QObject* parent)
  : QObject(parent)
  , mInterval(_Interval)
  , mIdleTimer(new QTimer(this))
{
  mIdleTimer->setInterval(mInterval);
  mIdleTimer->setSingleShot(true);

  connect(mIdleTimer, &QTimer::timeout, this, &UserIdleEventFilter::Idle);
}

UserIdleEventFilter::~UserIdleEventFilter()
{
  Remove();
}
