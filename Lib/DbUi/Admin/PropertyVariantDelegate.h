#pragma once

#include <QStyledItemDelegate>


class ObjectSettingsType;
class FormEditVariant;

class PropertyVariantDelegate: public QStyledItemDelegate
{
  const ObjectSettingsType* mTypeItem;

  mutable FormEditVariant*  mForm;

public:
  /*override */virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  /*override */virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  /*override */virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
//  /*override */virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
  void Done();

public:
  explicit PropertyVariantDelegate(const ObjectSettingsType* _TypeItem, QObject *parent = 0);
};

