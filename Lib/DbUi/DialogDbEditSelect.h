#pragma once

#include <QDialog>

#include <Lib/Db/DbTable.h>


class TreeModelB;
class DbTreeSchema;

namespace Ui {
class DialogDbEditSelect;
}

class DialogDbEditSelect: public QDialog
{
  Ui::DialogDbEditSelect* ui;

  TreeModelB*             mTreeModel;
  bool                    mSingleCheck;
  bool                    mHasCheck;

  Q_OBJECT

public:
  explicit DialogDbEditSelect(QWidget* parent = 0);
  ~DialogDbEditSelect();

public:
  void Init(const QVector<DbItemBS>& itemsList, DbTreeSchema* schema);
  void SetSingleItem();
  void GetResult(QVector<DbItemBS>& itemsList);

private:
  void OnItemDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
};
