#include <limits>

#include <Lib/Common/Format.h>

#include "FormEditTimePeriod.h"
#include "ui_FormEditTimePeriod.h"


void FormEditTimePeriod::SetValues(const QString& _MinValue, const QString& _MaxValue)
{
  mMinValue = ParseValue(_MinValue);
  mMaxValue = ParseValue(_MaxValue);

  if (!mMaxValue || mMaxValue <= mMinValue) {
    mMaxValue = (qint64)30 * 24 * 60 * 60 * 1000;
  }

  ui->doubleSpinBoxMain->setMinimum(mMinValue);
  ui->doubleSpinBoxMain->setMaximum(mMaxValue);
}

void FormEditTimePeriod::SetCurrent(const QVariant& data)
{
  qint64 size = ParseValue(data.toString());
  if (size) {
    mManual = true;
    ApplySize(size);
    mManual = false;
    mCurrentSize = size;
  }
}

QVariant FormEditTimePeriod::GetCurrent()
{
  return QVariant(mCurrentSize);
}


FormEditTimePeriod::FormEditTimePeriod(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditTimePeriod)
  , mManual(false)
{
  ui->setupUi(this);

  setFocusProxy(ui->doubleSpinBoxMain);

  mSuffixBase << 1 << 1000 << 60 * 1000 << 60 * 60 * 1000 << 24 * 60 * 60 * 1000;
  foreach (const QString& suffix, kTimeSuffixes) {
    ui->comboBoxType->addItem(suffix);
  }
}

FormEditTimePeriod::~FormEditTimePeriod()
{
  delete ui;
}


qint64 FormEditTimePeriod::ParseValue(const QString& text)
{
  if (text.endsWith("ms") || text.endsWith("мс")) {
    return text.left(text.size() - 2).trimmed().toLongLong();
  } else if (text.endsWith("s") || text.endsWith("с")) {
    return text.left(text.size() - 1).trimmed().toLongLong() * 1000;
  } else if (text.endsWith("m") || text.endsWith("м")) {
    return text.left(text.size() - 1).trimmed().toLongLong() * 60 * 1000;
  } else if (text.endsWith("h") || text.endsWith("ч")) {
    return text.left(text.size() - 1).trimmed().toLongLong() * 60 * 60 * 1000;
  } else if (text.endsWith("d") || text.endsWith("д")) {
    return text.left(text.size() - 1).trimmed().toLongLong() * 24 * 60 * 60 * 1000;
  }
  return text.toLongLong();
}

qint64 FormEditTimePeriod::CalcSize(int suffix)
{
  qint64 size = (qint64)(mSuffixBase.at(suffix) * ui->doubleSpinBoxMain->value());
  return size;
}

void FormEditTimePeriod::ApplySize(qint64 size)
{
  qreal value;
  int suffix;
  FormatTime(size, value, suffix);

  ui->doubleSpinBoxMain->setMinimum((qreal)mMinValue / mSuffixBase.at(suffix));
  ui->doubleSpinBoxMain->setMaximum((qreal)mMaxValue / mSuffixBase.at(suffix));
  ui->doubleSpinBoxMain->setValue(value);
  ui->comboBoxType->setCurrentIndex(suffix);
}

void FormEditTimePeriod::on_doubleSpinBoxMain_editingFinished()
{
  if (!mManual) {
    mCurrentSize = CalcSize(ui->comboBoxType->currentIndex());
  }
}

void FormEditTimePeriod::on_comboBoxType_currentIndexChanged(int index)
{
  if (!mManual) {
    int suffix = index;
    qreal value = (qreal)mCurrentSize / mSuffixBase.at(suffix);

    ui->doubleSpinBoxMain->setMinimum((qreal)mMinValue / mSuffixBase.at(suffix));
    ui->doubleSpinBoxMain->setMaximum((qreal)mMaxValue / mSuffixBase.at(suffix));
    ui->doubleSpinBoxMain->setValue(value);
  }
}
