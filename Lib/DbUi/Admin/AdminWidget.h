#pragma once

#include <QWidget>

#include <Lib/Include/Common.h>


class AdminWidget: public QWidget
{
  PROTECTED_GET(bool, IsActive)
  bool                mLoad;

  Q_OBJECT
  ;
public:
  void Activate();
  void Deactivate();

protected:
  /*new */virtual void Prepare();
  /*new */virtual void Activated();
  /*new */virtual void Deactivated();

public:
  explicit AdminWidget(QWidget* parent = 0);
};
