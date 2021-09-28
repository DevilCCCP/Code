#pragma once

#include <QStyledItemDelegate>

#include <Lib/Include/Common.h>


DefineClassS(FormText);

class PropertyTextDelegate: public QStyledItemDelegate
{
  mutable FormText* mForm;

public:
  /*override */virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  /*override */virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  /*override */virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
//  /*override */virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
  void Done();

public:
  explicit PropertyTextDelegate(QObject *parent = 0);
};

