#include <limits>

#include "FormEditReal.h"
#include "ui_FormEditReal.h"


void FormEditReal::SetValues(const QString& _MinValue, const QString& _MaxValue)
{
  mMinValue = _MinValue.toDouble();
  mMaxValue = _MaxValue.toDouble();

  if (mMaxValue <= 0 || mMaxValue <= mMinValue) {
    mMaxValue = std::numeric_limits<double>::max();
  }

  ui->doubleSpinBoxMain->setMinimum(mMinValue);
  ui->doubleSpinBoxMain->setMaximum(mMaxValue);
}

void FormEditReal::SetCurrent(const QVariant& data)
{
  bool ok;
  qreal value = data.toReal(&ok);
  if (ok) {
    ui->doubleSpinBoxMain->setValue(value);
  }
}

QVariant FormEditReal::GetCurrent()
{
  return QVariant(QLocale::c().toString(ui->doubleSpinBoxMain->value()));
}


FormEditReal::FormEditReal(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditReal)
{
  ui->setupUi(this);
  this->setLocale(QLocale::c());

  setFocusProxy(ui->doubleSpinBoxMain);
}

FormEditReal::~FormEditReal()
{
  delete ui;
}

void FormEditReal::on_doubleSpinBoxMain_editingFinished()
{
  EditDone();
}
