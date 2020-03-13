#pragma once

#include <QStyledItemDelegate>

#include <Lib/Include/Common.h>


DefineClassS(FormInt);

class PropertyIntDelegate: public QStyledItemDelegate
{
  qint64   mMinValue;
  qint64   mMaxValue;

  mutable FormInt* mForm;

public:
  /*override */virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;
  /*override */virtual void setEditorData(QWidget* editor, const QModelIndex& index) const Q_DECL_OVERRIDE;
  /*override */virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE;
//  /*override */virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
  void Done();

public:
  explicit PropertyIntDelegate(const QString& _MinValue, const QString& _MaxValue, QObject *parent = 0);
};

