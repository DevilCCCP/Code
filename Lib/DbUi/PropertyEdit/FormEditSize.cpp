#include <limits>

#include <Lib/Common/Format.h>

#include "FormEditSize.h"
#include "ui_FormEditSize.h"


void FormEditSize::SetValues(const QString& _MinValue, const QString& _MaxValue)
{
  mMinValue = _MinValue.toLongLong();
  mMaxValue = _MaxValue.toLongLong();

  if (!mMaxValue || mMaxValue <= mMinValue) {
    mMaxValue = std::numeric_limits<qint64>::max();
  }

  ui->doubleSpinBoxMain->setMinimum(mMinValue);
  ui->doubleSpinBoxMain->setMaximum(mMaxValue);
}

void FormEditSize::SetCurrent(const QVariant& data)
{
  bool ok;
  qint64 size = data.toLongLong(&ok);
  if (ok) {
    mManual = true;
    ApplySize(size);
    mManual = false;
    mCurrentSize = size;
  }
}

QVariant FormEditSize::GetCurrent()
{
  return QVariant(mCurrentSize);
}


FormEditSize::FormEditSize(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditSize)
  , mManual(false)
{
  ui->setupUi(this);

  setFocusProxy(ui->doubleSpinBoxMain);

  qint64 base = 1;
  foreach (const QString& suffix, kBytesSuffixes) {
    mSuffixBase << base;
    base *= 1024;
    ui->comboBoxType->addItem(suffix);
  }
}

FormEditSize::~FormEditSize()
{
  delete ui;
}

qint64 FormEditSize::CalcSize(int suffix)
{
  qint64 size = (qint64)(mSuffixBase.at(suffix) * ui->doubleSpinBoxMain->value());
  return size;
}

void FormEditSize::ApplySize(qint64 size)
{
  qreal value;
  int suffix;
  FormatBytes(size, value, suffix);

  ui->doubleSpinBoxMain->setMinimum((qreal)mMinValue / mSuffixBase.at(suffix));
  ui->doubleSpinBoxMain->setMaximum((qreal)mMaxValue / mSuffixBase.at(suffix));
  ui->doubleSpinBoxMain->setValue(value);
  ui->comboBoxType->setCurrentIndex(suffix);
}

void FormEditSize::on_doubleSpinBoxMain_editingFinished()
{
  if (!mManual) {
    mCurrentSize = CalcSize(ui->comboBoxType->currentIndex());
  }
}

void FormEditSize::on_comboBoxType_currentIndexChanged(int index)
{
  if (!mManual) {
    int suffix = index;
    qreal value = (qreal)mCurrentSize / mSuffixBase.at(suffix);

    ui->doubleSpinBoxMain->setMinimum((qreal)mMinValue / mSuffixBase.at(suffix));
    ui->doubleSpinBoxMain->setMaximum((qreal)mMaxValue / mSuffixBase.at(suffix));
    ui->doubleSpinBoxMain->setValue(value);
  }
}
