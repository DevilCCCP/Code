#include <QKeyEvent>
#include <QMouseEvent>
#include <QItemSelectionModel>
#include <QStyledItemDelegate>

#include "CheckableTreeView.h"
#include "CheckItemDelegate.h"


void CheckableTreeView::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Space) {
    if (selectionModel()) {
      QModelIndex index = selectionModel()->currentIndex();
      if (event->modifiers().testFlag(Qt::ControlModifier)) {
        SelectionChange(index);
      }
      if (index.isValid()) {
        return CheckChange(index);
      }
    }
  }

  QTreeView::keyPressEvent(event);
}

void CheckableTreeView::mousePressEvent(QMouseEvent* event)
{
  QModelIndex index = indexAt(event->pos());
  if (index.isValid() && index.flags().testFlag(Qt::ItemIsUserCheckable)) {
    QRect checkRect = mCheckItemDelegate->CheckBoxFromIndex(index);
    if (checkRect.contains(event->pos())) {
      mCheckPressed = true;
      return CheckChange(index);
    }
  }

  mCheckPressed = false;
  QTreeView::mousePressEvent(event);
}

void CheckableTreeView::mouseMoveEvent(QMouseEvent* event)
{
  if (mCheckPressed) {
    return;
  }

  QTreeView::mouseMoveEvent(event);
}

void CheckableTreeView::mouseReleaseEvent(QMouseEvent* event)
{
  if (mCheckPressed) {
    mCheckPressed = false;
    return;
  }

  QTreeView::mouseReleaseEvent(event);
}

void CheckableTreeView::CheckChange(const QModelIndex& index)
{
  if (model()) {
    bool hasChecked = model()->data(index, Qt::CheckStateRole).toBool();
    Qt::CheckState checked = hasChecked? Qt::Unchecked: Qt::Checked;
    if (selectionModel() && selectionModel()->isSelected(index)) {
      QModelIndexList selectedIndexes = selectionModel()->selectedRows();
      foreach (const QModelIndex& selectedIndex, selectedIndexes) {
        if (model()->flags(selectedIndex).testFlag(Qt::ItemIsUserCheckable)) {
          model()->setData(selectedIndex, checked, Qt::CheckStateRole);
        }
      }
    } else {
      model()->setData(index, checked, Qt::CheckStateRole);
    }
  }
}

void CheckableTreeView::SelectionChange(const QModelIndex& index)
{
  if (!selectionModel()) {
    return;
  }

  bool selected = !selectionModel()->isSelected(index);
  selectionModel()->select(index, selected? QItemSelectionModel::Select: QItemSelectionModel::Deselect);
}


CheckableTreeView::CheckableTreeView(QWidget* parent)
  : QTreeView(parent)
  , mCheckItemDelegate(new CheckItemDelegate(this)), mCheckPressed(false)
{
  setItemDelegateForColumn(0, mCheckItemDelegate);
}

