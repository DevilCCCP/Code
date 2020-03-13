#include <QHBoxLayout>
#include <QRadioButton>
#include <QComboBox>

#include "PropertyTextDelegate.h"


QWidget* PropertyTextDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);

  mForm = new FormText(parent);

  connect(mForm, &FormText::Done, this, &PropertyTextDelegate::Done);
  return mForm;
}

void PropertyTextDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (FormText* form = static_cast<FormText*>(editor)) {
    QVariant variant = index.model()->data(index);
    form->SetCurrent(variant);
  }
}

void PropertyTextDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (FormText* form = static_cast<FormText*>(editor)) {
    QVariant variant = form->GetCurrent();
    model->setData(index, variant);
  }
}

void PropertyTextDelegate::Done()
{
  emit commitData(mForm);
  emit closeEditor(mForm);
}

PropertyTextDelegate::PropertyTextDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}
