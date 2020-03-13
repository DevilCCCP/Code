#include <limits>

#include "FormEditInt.h"
#include "ui_FormEditInt.h"


void FormEditInt::SetValues(const QString& _MinValue, const QString& _MaxValue)
{
  mMinValue = _MinValue.toLongLong();
  mMaxValue = _MaxValue.toLongLong();

  if (mMaxValue <= 0 || mMaxValue <= mMinValue) {
    mMaxValue = std::numeric_limits<int>::max();
  }

  ui->spinBoxMain->setMinimum(mMinValue);
  ui->spinBoxMain->setMaximum(mMaxValue);
}

void FormEditInt::SetCurrent(const QVariant& data)
{
  bool ok;
  qint64 value = data.toLongLong(&ok);
  if (ok) {
    ui->spinBoxMain->setValue(value);
  }
}

QVariant FormEditInt::GetCurrent()
{
  return QVariant(ui->spinBoxMain->value());
}


FormEditInt::FormEditInt(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditInt)
{
  ui->setupUi(this);

  setFocusProxy(ui->spinBoxMain);
}

FormEditInt::~FormEditInt()
{
  delete ui;
}

void FormEditInt::on_spinBoxMain_editingFinished()
{
  EditDone();
}
