#pragma once

#include <QWidget>
#include <QStandardItemModel>

#include "Core.h"


DefineClassS(Core);

namespace Ui {
class FormSystem;
}

typedef QStandardItem* StateMl;
typedef QPair<QBrush, QBrush> ItemView;

class FormSystem: public QWidget
{
  Ui::FormSystem*     ui;

  Core*               mCore;

  QStandardItemModel* mStateModel;
  QMap<int, StateMl>  mStateMap;
  ItemView            mItemEnabled;
  ItemView            mItemDisabled;
  ItemView            mItemNotExists;
  int                 mHourEvents;
  bool                mHaveObject;

  Q_OBJECT

public:
  void Init(Core* _Core);

public:
  void ReloadState();
  bool UpdateState();

private:
  QStandardItem* CreateBadStateItem();

public:
  explicit FormSystem(QWidget *parent = 0);
  ~FormSystem();
};

