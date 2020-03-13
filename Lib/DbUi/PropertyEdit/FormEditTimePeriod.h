#pragma once

#include "FormEditVariant.h"


namespace Ui {
class FormEditTimePeriod;
}

class FormEditTimePeriod: public FormEditVariant
{
  Ui::FormEditTimePeriod* ui;
  qint64            mMinValue;
  qint64            mMaxValue;
  QList<qint64>     mSuffixBase;

  qint64            mCurrentSize;
  bool              mManual;

  Q_OBJECT

public:
  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) Q_DECL_OVERRIDE;
  /*override */virtual void SetCurrent(const QVariant& data) Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetCurrent() Q_DECL_OVERRIDE;

public:
  explicit FormEditTimePeriod(QWidget* parent = 0);
  ~FormEditTimePeriod();

private:
  qint64 ParseValue(const QString& text);
  qint64 CalcSize(int suffix);
  void ApplySize(qint64 size);

private slots:
  void on_doubleSpinBoxMain_editingFinished();
  void on_comboBoxType_currentIndexChanged(int index);
};
