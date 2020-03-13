#include <limits>

#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/DbSettings.h>

#include "StorageForm.h"
#include "ui_StorageForm.h"


StorageForm::StorageForm(const ObjectItemS& _Object, const Db& _Db, QWidget* parent)
  : QWidget(parent), ui(new Ui::StorageForm)
  , mStorageObject(_Object), mDbSettings(new DbSettings(_Db))
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  ui->pushButtonSave->setIcon(QIcon(":/Icons/Menu Ok.png"));
  mDbSettings->SetSilent(true);
  if (mStorageObject->Status < 0 && LoadSettings()) {
    mOldCapacity = mCapacity;
    UpdateBytesValues();

    ui->labelEditWarning->setVisible(false);
  } else {
    ui->widgetSettings->setEnabled(false);
    ui->labelEditWarning->setVisible(true);
  }
}

StorageForm::~StorageForm()
{
  delete ui;
}


bool StorageForm::LoadSettings()
{
  if (!mDbSettings->Open(mStorageObject->Id)) {
    return false;
  }

  mCellSize = mDbSettings->GetValue("CellSize", -1).toInt();
  mCapacity = mDbSettings->GetValue("Capacity", -1).toInt();

  if (mCellSize <= 0 || mCapacity <= 0) {
    return false;
  }

  return true;
}

void StorageForm::UpdateBytesValues()
{
  ui->widgetSave->setVisible(mCapacity != mOldCapacity);

  ui->spinBoxClaster->setMinimum(mCellSize);
  ui->spinBoxClaster->setMaximum(mCellSize);
  ui->spinBoxClaster->setValue(mCellSize);
  ui->spinBoxCount->setMinimum(0);
  ui->spinBoxCount->setMaximum(std::numeric_limits<int>::max());
  ui->spinBoxCount->setValue(mCapacity);

  qreal mass = 1.0;
  qreal maxValue = std::numeric_limits<int>::max();
  ui->doubleSpinBoxB->setMinimum(0);
  ui->doubleSpinBoxB->setMaximum(mass * maxValue * mCellSize);
  ui->doubleSpinBoxB->setValue(mass * mCapacity * mCellSize);
  mass /= 1024.0;
  ui->doubleSpinBoxKb->setMinimum(0);
  ui->doubleSpinBoxKb->setMaximum(mass * maxValue * mCellSize);
  ui->doubleSpinBoxKb->setValue(mass * mCapacity * mCellSize);
  mass /= 1024.0;
  ui->doubleSpinBoxMb->setMinimum(0);
  ui->doubleSpinBoxMb->setMaximum(mass * maxValue * mCellSize);
  ui->doubleSpinBoxMb->setValue(mass * mCapacity * mCellSize);
  mass /= 1024.0;
  ui->doubleSpinBoxGb->setMinimum(0);
  ui->doubleSpinBoxGb->setMaximum(mass * maxValue * mCellSize);
  ui->doubleSpinBoxGb->setValue(mass * mCapacity * mCellSize);
  mass /= 1024.0;
  ui->doubleSpinBoxTb->setMinimum(0);
  ui->doubleSpinBoxTb->setMaximum(mass * maxValue * mCellSize);
  ui->doubleSpinBoxTb->setValue(mass * mCapacity * mCellSize);
}

void StorageForm::on_doubleSpinBoxB_editingFinished()
{
  mCapacity = (int)(ui->doubleSpinBoxB->value() / mCellSize);
  UpdateBytesValues();
}

void StorageForm::on_doubleSpinBoxKb_editingFinished()
{
  mCapacity = (int)(ui->doubleSpinBoxKb->value() * 1024 / mCellSize);
  UpdateBytesValues();
}

void StorageForm::on_doubleSpinBoxMb_editingFinished()
{
  mCapacity = (int)(ui->doubleSpinBoxMb->value() * 1024 * 1024 / mCellSize);
  UpdateBytesValues();
}

void StorageForm::on_doubleSpinBoxGb_editingFinished()
{
  mCapacity = (int)(ui->doubleSpinBoxGb->value() * 1024 * 1024 * 1024 / mCellSize);
  UpdateBytesValues();
}

void StorageForm::on_doubleSpinBoxTb_editingFinished()
{
  mCapacity = (int)(ui->doubleSpinBoxTb->value() * 1024 * 1024 * 1024 * 1024 / mCellSize);
  UpdateBytesValues();
}

void StorageForm::on_spinBoxCount_editingFinished()
{
  mCapacity = ui->spinBoxCount->value();
  UpdateBytesValues();
}

void StorageForm::on_pushButtonSave_clicked()
{
  mDbSettings->SetValue("Capacity", mCapacity);
  mOldCapacity = mCapacity;
  UpdateBytesValues();
}
