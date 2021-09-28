#pragma once

#include "FormEditVariant.h"


namespace Ui {
class FormEditSize;
}

class FormEditSize: public FormEditVariant
{
  Ui::FormEditSize* ui;
  qint64            mMinValue;
  qint64            mMaxValue;
  QList<qint64>     mSuffixBase;

  qint64            mCurrentSize;
  bool              mManual;

  Q_OBJECT

public:
  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) override;
  /*override */virtual void SetCurrent(const QVariant& data) override;
  /*override */virtual QVariant GetCurrent() override;

public:
  explicit FormEditSize(QWidget* parent = 0);
  ~FormEditSize();

private:
  qint64 CalcSize(int suffix);
  void ApplySize(qint64 size);

private slots:
  void on_doubleSpinBoxMain_editingFinished();
  void on_comboBoxType_currentIndexChanged(int index);
};
