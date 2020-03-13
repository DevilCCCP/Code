#include <QHBoxLayout>
#include <QRadioButton>
#include <QComboBox>

#include "PropertyIntDelegate.h"
#include "FormInt.h"


QWidget* PropertyIntDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);

  mForm = new FormInt(parent);
  mForm->SetValues(mMinValue, mMaxValue);

  connect(mForm, &FormInt::Done, this, &PropertyIntDelegate::Done);
  return mForm;
}

void PropertyIntDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (FormInt* form = static_cast<FormInt*>(editor)) {
    QVariant variant = index.model()->data(index);
    form->SetCurrent(variant);
  }
}

void PropertyIntDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (FormInt* form = static_cast<FormInt*>(editor)) {
    QVariant variant = form->GetCurrent();
    model->setData(index, variant);
  }
}

void PropertyIntDelegate::Done()
{
  emit commitData(mForm);
  emit closeEditor(mForm);
}

PropertyIntDelegate::PropertyIntDelegate(const QString& _MinValue, const QString& _MaxValue, QObject* parent)
  : QStyledItemDelegate(parent)
  , mMinValue(_MinValue.toLongLong()), mMaxValue(_MaxValue.toLongLong())
{
}
