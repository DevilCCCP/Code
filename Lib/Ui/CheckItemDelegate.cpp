#include <QDebug>

#include "CheckItemDelegate.h"


void CheckItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_ASSERT(index.isValid());

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QStyle* style = mWidget->style();
  QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, mWidget);
  mCheckRectMap[index] = checkRect;
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, mWidget);
}

QRect CheckItemDelegate::CheckBoxFromIndex(const QModelIndex& index)
{
  return mCheckRectMap[index];
}


CheckItemDelegate::CheckItemDelegate(QWidget* parentWidget)
  : QStyledItemDelegate(parentWidget)
  , mWidget(parentWidget)
{
}
