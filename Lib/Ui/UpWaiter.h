#pragma once

#include <QObject>

#include <Lib/Include/Common.h>


DefineClassS(UpWaiter);
DefineClassS(FormUpdateSync);
DefineClassS(UpInfo);
DefineClassS(QTimer);
DefineClassS(QWidget);

class UpWaiter: public QObject
{
  UpInfo*               mUpInfo;
  QWidget*              mParent;
  PROPERTY_GET_SET(int, LockIntervalMs)

  QTimer*               mTimer;
  FormUpdateSync*       mFormUpdateSync;

  int                   mUpdateSeconds;
  int                   mUpUserSeconds;
  bool                  mUserDialog;

  Q_OBJECT

private:
  void CreateDialog();
  bool CheckUpStart();
  bool CheckUserStart();

private:
  void OnTimer();
  void OnUserSetUpStart(int timeoutMs);

signals:
  void UpdateSecs(int secs);
  void CloseDialog();

public:
  explicit UpWaiter(UpInfo* _UpInfo, QWidget* parent = 0);
  ~UpWaiter();
};
