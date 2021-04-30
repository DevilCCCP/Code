#pragma once

#include <QWidget>
#include <QStandardItemModel>

#include <Lib/Db/Db.h>


DefineClassS(PropertyForm);
DefineClassS(PropertyItemModel);

namespace Ui {
class PropertyForm;
}

class PropertyForm: public QWidget
{
  Ui::PropertyForm* ui;

  const Db&                  mDb;
  ObjectTableS               mObjectTable;
  ObjectSettingsTableS       mObjectSettingsTable;
  ObjectSettingsTypeTableS   mObjectSettingsTypeTable;

  ObjectItemS                mCurrentObject;
  QList<ObjectSettingsS>     mObjectSettings;
  QList<ObjectSettingsTypeS> mCustomSettingTypes;
  QList<ObjectSettingsS>     mCustomSettings;
  QList<QStandardItem*>      mPropertyValues;
  PropertyItemModel*         mPropertiesModel;
  bool                       mStandartIdItem;
  bool                       mStandartNameItem;
  bool                       mStandartDescrItem;
  bool                       mStandartUuidItem;
  bool                       mUnlockEdit;

  QBrush                     mBrushNormal;
  QBrush                     mBrushDisabled;
  QBrush                     mBrushEdited;
  QBrush                     mBrushSaved;
  QBrush                     mBrushFail;

  PROPERTY_GET_SET(QStringList, ColumnsNames)

  Q_OBJECT

public:
  const ObjectItemS&            CurrentObject() const  { return mCurrentObject; }
  const QList<ObjectSettingsS>& CurrentSettings() const  { return mObjectSettings; }
  ObjectSettingsTableS          GetObjectSettingsTable() const { return mObjectSettingsTable; }
  ObjectSettingsTypeTableS      GetObjectSettingsTypeTable() const { return mObjectSettingsTypeTable; }

public:
  explicit PropertyForm(const Db& _Db, QWidget *parent = 0);
  ~PropertyForm();

protected:
  /*override */void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

public:
  bool Reload();
  void Clear();
  void SetObject(const ObjectItemS& obj);
  void AddCustom(const ObjectSettingsTypeS& descr, const QVariant& value, bool editable = true);

private:
  bool AppendSetting(int type, const QString& key, const QString& value, bool editable = true);
  void AppendSetting(const ObjectSettingsType* descr, const QVariant& value, bool editable);

  void OnValueEditFinished(QStandardItem* item);

signals:
  void OnItemNameEdited();
  void OnItemPropertyEdited();
  void OnItemCustomEdited(int index, QString value);

private slots:
  void OnTablecurrentChanged(const QModelIndex& current, const QModelIndex& previous);
  void on_actionUnlock_triggered();
};
