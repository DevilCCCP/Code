#pragma once

#include <QStyledItemDelegate>
#include <QMap>


class CheckItemDelegate: public QStyledItemDelegate
{
  QWidget*                         mWidget;

  mutable QMap<QModelIndex, QRect> mCheckRectMap;

public:
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

public:
  QRect CheckBoxFromIndex(const QModelIndex& index);

public:
  explicit CheckItemDelegate(QWidget* parentWidget);
};

