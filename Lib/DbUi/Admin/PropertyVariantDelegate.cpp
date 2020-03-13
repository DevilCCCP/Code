#include <Lib/Db/ObjectSettingsType.h>

#include "PropertyVariantDelegate.h"
#include "PropertyEdit/FormEditVariant.h"
#include "PropertyEdit/FormEditBool.h"
#include "PropertyEdit/FormEditInt.h"
#include "PropertyEdit/FormEditReal.h"
#include "PropertyEdit/FormEditText.h"
#include "PropertyEdit/FormEditSize.h"
#include "PropertyEdit/FormEditTimeRange.h"
#include "PropertyEdit/FormEditTimePeriod.h"


QWidget* PropertyVariantDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);

  if (mTypeItem->Type == "bool") {
    mForm = new FormEditBool(parent);
  } else if (mTypeItem->Type == "int") {
    mForm = new FormEditInt(parent);
  } else if (mTypeItem->Type == "real" || mTypeItem->Type == "float") {
    mForm = new FormEditReal(parent);
  } else if (mTypeItem->Type == "size") {
    mForm = new FormEditSize(parent);
  } else if (mTypeItem->Type == "time_range") {
    mForm = new FormEditTimeRange(parent);
  } else if (mTypeItem->Type == "time_period") {
    mForm = new FormEditTimePeriod(parent);
  } else {
    mForm = new FormEditText(parent);
  }
  mForm->SetValues(mTypeItem->MinValue, mTypeItem->MaxValue);

  connect(mForm, &FormEditVariant::Done, this, &PropertyVariantDelegate::Done);
  return mForm;
}

void PropertyVariantDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  if (FormEditVariant* form = static_cast<FormEditVariant*>(editor)) {
    QVariant variant = index.model()->data(index, Qt::UserRole + 2);
    form->SetCurrent(variant);
  }
}

void PropertyVariantDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  if (FormEditVariant* form = static_cast<FormEditVariant*>(editor)) {
    QVariant variant = form->GetCurrent();
    model->setData(index, variant, Qt::UserRole + 2);
  }
}

void PropertyVariantDelegate::Done()
{
  emit commitData(mForm);
  emit closeEditor(mForm);
}

PropertyVariantDelegate::PropertyVariantDelegate(const ObjectSettingsType* _TypeItem, QObject* parent)
  : QStyledItemDelegate(parent)
  , mTypeItem(_TypeItem)
{
}
