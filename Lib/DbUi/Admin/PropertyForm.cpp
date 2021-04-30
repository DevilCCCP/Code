#include <QItemDelegate>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectSettings.h>
#include <Lib/Db/ObjectSettingsType.h>

#include "PropertyForm.h"
#include "Def.h"
#include "PropertyItemModel.h"
#include "PropertyVariantDelegate.h"
#include "ui_PropertyForm.h"


PropertyForm::PropertyForm(const Db& _Db, QWidget *parent)
  : QWidget(parent), ui(new Ui::PropertyForm)
  , mDb(_Db), mObjectTable(new ObjectTable(_Db))
  , mObjectSettingsTable(new ObjectSettingsTable(_Db)), mObjectSettingsTypeTable(new ObjectSettingsTypeTable(_Db))
  , mUnlockEdit(false)
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->splitterMain->setStyleSheet(QLatin1String("QSplitter::handle {\n"
    "border-image: url(:/ObjTree/separator);\n"
    "}\n"
    "	"));
  ui->splitterMain->setStretchFactor(0, 1);
  ui->splitterMain->setStretchFactor(1, 0);

  QStandardItem testItem;
  mBrushNormal = testItem.background();
  QColor normColor = mBrushNormal.color();
  mBrushDisabled = ui->tableViewProperties->palette().brush(QPalette::Background);
  mBrushEdited = QBrush(QColor::fromRgb(normColor.red(), normColor.green(), 250));
  mBrushSaved = QBrush(QColor::fromRgb(normColor.red(), 150, normColor.blue()));
  mBrushFail = QBrush(QColor::fromRgb(200, normColor.green(), normColor.blue()));

  mPropertiesModel = new PropertyItemModel(this);
  ui->tableViewProperties->setModel(mPropertiesModel);
  QItemSelectionModel* smp = ui->tableViewProperties->selectionModel();
  mColumnsNames << "Property" << "Value";
  ui->tableViewProperties->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  ui->tableViewProperties->addAction(ui->actionUnlock);

  mObjectSettingsTypeTable->Reload();

  connect(mPropertiesModel, &PropertyItemModel::itemChanged, this, &PropertyForm::OnValueEditFinished);
  connect(smp, &QItemSelectionModel::currentChanged, this, &PropertyForm::OnTablecurrentChanged);
}

PropertyForm::~PropertyForm()
{
  delete ui;
}


void PropertyForm::showEvent(QShowEvent* event)
{
  ui->labelPropName->setStyleSheet("font: bold 9pt \"Segoe UI\", Verdana;");

  QWidget::showEvent(event);
}

bool PropertyForm::Reload()
{
  return mObjectSettingsTypeTable->Reload();
}

void PropertyForm::Clear()
{
  for (int i = 0; i < ui->tableViewProperties->model()->rowCount(); i++) {
    QAbstractItemDelegate* pd = ui->tableViewProperties->itemDelegateForRow(i);
    ui->tableViewProperties->setItemDelegateForRow(i, nullptr);
    delete pd;
  }
  mPropertiesModel->clear();
  mPropertyValues.clear();

  ui->labelPropName->setText("");
  ui->labelPropDescr->setText("");
  mCustomSettings.clear();
}

void PropertyForm::SetObject(const ObjectItemS& obj)
{
  Clear();

  mCurrentObject = obj;

  if (!mCurrentObject || !mObjectSettingsTable->GetObjectSettings(mCurrentObject->Id, mObjectSettings)) {
    return;
  }

  mStandartIdItem    = AppendSetting(0, "Id", QString::number(mCurrentObject->Id), false);
  mStandartNameItem  = AppendSetting(0, "Name", mCurrentObject->Name);
  mStandartDescrItem = AppendSetting(0, "Descr", mCurrentObject->Descr);
  mStandartUuidItem  = AppendSetting(0, "Uuid", mCurrentObject->Guid, false);

  for (auto itr = mObjectSettings.begin(); itr != mObjectSettings.end(); ) {
    const ObjectSettingsS& item = *itr;
    if (AppendSetting(obj->Type, item->Key, item->Value)) {
      itr++;
    } else {
      itr = mObjectSettings.erase(itr);
    }
  }
  ui->tableViewProperties->setColumnWidth(0, 100);
  ui->tableViewProperties->horizontalHeader()->setStretchLastSection(true);

  QModelIndex firstIndex = ui->tableViewProperties->model()->index(0, 0);
  ui->tableViewProperties->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
  ui->tableViewProperties->setCurrentIndex(firstIndex);

  mPropertiesModel->setHorizontalHeaderLabels(mColumnsNames);
}

void PropertyForm::AddCustom(const ObjectSettingsTypeS& descr, const QVariant& value, bool editable)
{
  ObjectSettingsS setting(new ObjectSettings());
  setting->Value = value.toString();
  mCustomSettings.append(setting);
  mCustomSettingTypes.append(descr);
  AppendSetting(descr.data(), value, editable);
}

bool PropertyForm::AppendSetting(int type, const QString& key, const QString& value, bool editable)
{
  const ObjectSettingsType* typeItem = mObjectSettingsTypeTable->GetObjectTypeSettingsType(type, key);
  if (!typeItem) {
    return false;
  }

  AppendSetting(typeItem, value, editable);
  return true;
}

void PropertyForm::AppendSetting(const ObjectSettingsType* descr, const QVariant& value, bool editable)
{
  QStandardItem* keyItem = new QStandardItem(descr->Name);
  keyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  keyItem->setBackground(mBrushDisabled);
  keyItem->setData(qVariantFromValue(descr));
  QStandardItem* editItem = new QStandardItem(value.toString());
  if (editable || mUnlockEdit) {
    editItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
  } else {
    editItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    editItem->setBackground(mBrushDisabled);
  }
  editItem->setData(qVariantFromValue(descr));
  editItem->setData(value, Qt::UserRole + 2);
  mPropertyValues.append(editItem);
  mPropertiesModel->appendRow(QList<QStandardItem*>() << keyItem << editItem);
  PropertyVariantDelegate* pd = new PropertyVariantDelegate(descr, this);
  ui->tableViewProperties->setItemDelegateForRow(mPropertiesModel->rowCount() - 1, pd);
}

void PropertyForm::OnValueEditFinished(QStandardItem* item)
{
  bool changed = false;
  bool result = false;
  QString value = item->data(Qt::UserRole + 2).toString();
  for (int i = 0; i < mPropertyValues.size(); i++) {
    if (item != mPropertyValues.at(i)) {
      continue;
    }

    int index = i;
    if (mCurrentObject) {
      if (mStandartIdItem) {
        if (!index--) {
          int newId = value.toInt();
          changed = (newId > 0 && mCurrentObject->Id != newId);
          if (changed) {
            result = mObjectTable->UpdateItemId(mCurrentObject->Id, newId);
            if (result) {
              mCurrentObject->Id = newId;
            }
          }
          break;
        }
      }
      if (mStandartNameItem) {
        if (!index--) {
          changed = (mCurrentObject->Name != value);
          if (changed) {
            mCurrentObject->Name = value;
            result = mObjectTable->UpdateItem(*mCurrentObject);
            if (result) {
              emit OnItemNameEdited();
            }
          }
          break;
        }
      }
      if (mStandartDescrItem) {
        if (!index--) {
          changed = (mCurrentObject->Descr != value);
          if (changed) {
            mCurrentObject->Descr = value;
            result = mObjectTable->UpdateItem(*mCurrentObject);
          }
          break;
        }
      }
      if (mStandartUuidItem) {
        if (!index--) {
          changed = (mCurrentObject->Guid != value);
          if (changed) {
            mCurrentObject->Guid = value;
            result = mObjectTable->UpdateItem(*mCurrentObject);
          }
          break;
        }
      }
    }

    if (mCurrentObject) {
      if (index < mObjectSettings.size()) {
        ObjectSettingsS setting = mObjectSettings[index];
        changed = (setting->Value != value);
        if (changed) {
          setting->Value = value;
          result = mObjectSettingsTable->UpdateItem(*setting);
          if (result) {
            emit OnItemPropertyEdited();
          }
        }
      }
    } else {
      if (index < mCustomSettings.size()) {
        ObjectSettingsS setting = mCustomSettings[index];
        changed = (setting->Value != value);
        if (changed) {
          setting->Value = value;
          result = true;
          emit OnItemCustomEdited(index, value);
        }
      }
    }
    break;
  }

  if (changed) {
    item->setForeground(result? mBrushSaved: mBrushFail);
  }
}

void PropertyForm::OnTablecurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  Q_UNUSED(previous);

  const ObjectSettingsType* typeItem = mPropertiesModel->data(current, Qt::UserRole + 1).value<const ObjectSettingsType*>();
  if (typeItem) {
    ui->labelPropName->setText(typeItem->Name);
    ui->labelPropDescr->setText(typeItem->Descr);
  } else {
    ui->labelPropName->setText("");
    ui->labelPropDescr->setText("");
  }
}

void PropertyForm::on_actionUnlock_triggered()
{
  mUnlockEdit = true;

  SetObject(mCurrentObject);
}
