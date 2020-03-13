#pragma once

#include <QTreeView>


class CheckItemDelegate;

/*
QTreeView with non standard feature:
 * check/uncheck all selected items
   - when space button pushed
   - when clicked at checkbox of selected item
 * check applied when mouse press, not mouse released as standard
 * Features applies only if CheckItemDelegate used as ItemDelegate for column 0
*/
class CheckableTreeView: public QTreeView
{
  CheckItemDelegate* mCheckItemDelegate;
  bool               mCheckPressed;

protected:
  virtual void keyPressEvent(QKeyEvent* event) override;

  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;

protected:
  void CheckChange(const QModelIndex& index);

private:
  void SelectionChange(const QModelIndex& index);

public:
  CheckableTreeView(QWidget* parent = 0);
};
