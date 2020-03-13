#pragma once

#include <QWidget>

#include <Lib/Db/Db.h>


DefineClassS(DbSettings);

namespace Ui {
class StorageForm;
}

class StorageForm: public QWidget
{
  Ui::StorageForm* ui;

  ObjectItemS      mStorageObject;
  DbSettingsS      mDbSettings;

  int              mCellSize;
  int              mCapacity;
  int              mOldCapacity;

  Q_OBJECT

public:
  explicit StorageForm(const ObjectItemS& _Object, const Db& _Db, QWidget* parent = 0);
  ~StorageForm();

private:
  bool LoadSettings();
  void UpdateBytesValues();

private slots:
  void on_doubleSpinBoxB_editingFinished();
  void on_doubleSpinBoxKb_editingFinished();
  void on_doubleSpinBoxMb_editingFinished();
  void on_doubleSpinBoxGb_editingFinished();
  void on_doubleSpinBoxTb_editingFinished();
  void on_spinBoxCount_editingFinished();
  void on_pushButtonSave_clicked();
};
