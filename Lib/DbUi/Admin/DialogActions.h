#pragma once

#include <QDialog>

#include <Lib/Db/Db.h>

#include "ToolAction.h"


namespace Ui {
class DialogActions;
}

class ActionIcons;
class QStandardItemModel;
class QStandardItem;

class DialogActions: public QDialog
{
  Ui::DialogActions*  ui;

  QStandardItemModel* mItemModel;

  Q_OBJECT

public:
  explicit DialogActions(QWidget* parent = 0);
  ~DialogActions();

public:
  void Init(const ActionIcons* actionIcons, const ToolActionList& actions);
  QList<int> SelectedActions();

private slots:
  void on_labelAll_linkActivated(const QString&);
  void on_labelNone_linkActivated(const QString&);
};
