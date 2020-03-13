#include <QHBoxLayout>
#include <QRadioButton>
#include <QComboBox>

#include "PropertyBoolDelegate.h"


QWidget* PropertyBoolDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);

  mForm = new FormBool(parent);
  mForm->SetValues(mFalseText, mTrueText);

  connect(mForm, &FormBool::Done, this, &PropertyBoolDelegate::Done);
  return mForm;
}

void PropertyBoolDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (FormBool* form = static_cast<FormBool*>(editor)) {
    QVariant variant = index.model()->data(index);
    form->SetCurrent(variant);
  }
}

void PropertyBoolDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (FormBool* form = static_cast<FormBool*>(editor)) {
    QVariant variant = form->GetCurrent();
    model->setData(index, variant);
  }
}

void PropertyBoolDelegate::Done()
{
  emit commitData(mForm);
  emit closeEditor(mForm);
}

PropertyBoolDelegate::PropertyBoolDelegate(const QString& _FalseText, const QString& _TrueText, QObject* parent)
  : QStyledItemDelegate(parent)
  , mFalseText(_FalseText), mTrueText(_TrueText)
{
}
